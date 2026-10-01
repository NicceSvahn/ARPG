#include "InventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"

#include "../Characters/GenericCharacter.h"


namespace
{
    bool RollStatsFromPool(
        const TArray<FItemStatRange>& Pool,
        int32 Count,
        TSet<FGameplayTag>& ConfiguredTags,
        TArray<FItemRolledStat>& OutStats)
    {
        if (Count < 0 || Count > Pool.Num())
        {
            return false;
        }

        // Validate every entry, including duplicate tags across pools.
        for (const FItemStatRange& Range : Pool)
        {
            if (!Range.StatTag.IsValid() ||
                Range.MinValue > Range.MaxValue ||
                ConfiguredTags.Contains(Range.StatTag))
            {
                return false;
            }

            ConfiguredTags.Add(Range.StatTag);
        }

        // Work on a copy so the data asset is never modified.
        TArray<FItemStatRange> RemainingStats = Pool;

        for (int32 Pick = 0; Pick < Count; ++Pick)
        {
            const int32 RandomIndex = FMath::RandRange(
                0,
                RemainingStats.Num() - 1
            );

            const FItemStatRange& Selected =
                RemainingStats[RandomIndex];

            FItemRolledStat& Rolled =
                OutStats.AddDefaulted_GetRef();

            Rolled.StatTag = Selected.StatTag;
            Rolled.DisplayName = Selected.DisplayName;

            Rolled.Value = static_cast<float>(
                FMath::RandRange(
                    Selected.MinValue,
                    Selected.MaxValue
                )
                );

            // Prevent this stat from being selected again.
            RemainingStats.RemoveAtSwap(RandomIndex);
        }

        return true;
    }
}


UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}


void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    InventoryCapacity = FMath::Max(1, InventoryCapacity);

    const int32 EquipmentCount =
        static_cast<int32>(EEquipmentSlot::Count);

    Items.SetNum(InventoryCapacity + EquipmentCount);
    EquipmentEffectHandles.SetNum(EquipmentCount);

    for (UItemDefinition* Definition : StartingItems)
    {
        if (!AddItem(Definition))
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("[INVENTORY] Could not add starting item %s"),
                *GetNameSafe(Definition)
            );
        }
    }

    NotifyChanged();
}


void UInventoryComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(
        UInventoryComponent,
        Items,
        COND_OwnerOnly
    );
}


FInventoryItem UInventoryComponent::GetItem(
    int32 Index) const
{
    return Items.IsValidIndex(Index)
        ? Items[Index]
        : FInventoryItem{};
}


UAbilitySystemComponent* UInventoryComponent::GetASC() const
{
    IAbilitySystemInterface* Interface =
        Cast<IAbilitySystemInterface>(GetOwner());

    return Interface
        ? Interface->GetAbilitySystemComponent()
        : nullptr;
}


int32 UInventoryComponent::FindEmptyInventorySlot() const
{
    for (int32 Index = 0; Index < InventoryCapacity; ++Index)
    {
        if (
            Items.IsValidIndex(Index) &&
            !Items[Index].IsValid()
            )
        {
            return Index;
        }
    }

    return INDEX_NONE;
}


bool UInventoryComponent::AddItem(UItemDefinition* Definition)
{
    if (!GetOwner() ||
        !GetOwner()->HasAuthority() ||
        !IsValid(Definition))
    {
        return false;
    }

    const int32 Index = FindEmptyInventorySlot();

    if (Index == INDEX_NONE)
    {
        return false;
    }

    FInventoryItem NewItem;
    NewItem.InstanceId = FGuid::NewGuid();
    NewItem.Definition = Definition;

    TSet<FGameplayTag> ConfiguredTags;

    for (const FItemStatGroup& Group : Definition->StatGroups)
    {
        const bool bSuccess = RollStatsFromPool(
            Group.StatPool,
            Group.StatCount,
            ConfiguredTags,
            NewItem.RolledStats
        );

        if (!bSuccess)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("Invalid stat group '%s' on item '%s'"),
                *Group.GroupName.ToString(),
                *GetNameSafe(Definition)
            );

            return false;
        }
    }

    Items[Index] = MoveTemp(NewItem);

    NotifyChanged();
    return true;
}


void UInventoryComponent::RequestUseSlot(
    int32 Index,
    FGuid ExpectedItemId)
{
    ServerUseSlot(Index, ExpectedItemId);
}


bool UInventoryComponent::ApplyEquipmentEffect(
    const FInventoryItem& Item,
    FActiveGameplayEffectHandle& OutHandle)
{
    OutHandle = FActiveGameplayEffectHandle();

    if (!Item.IsValid())
    {
        return false;
    }

    UItemDefinition* Definition = Item.Definition.Get();

    if (!IsValid(Definition))
    {
        return false;
    }

    // Items without a bonus effect can still be equipped.
    if (!Definition->EquippedEffect)
    {
        return true;
    }

    UAbilitySystemComponent* ASC = GetASC();

    if (!ASC || ASC->GetAvatarActor() != GetOwner())
    {
        return false;
    }

    const UGameplayEffect* Effect =
        Definition->EquippedEffect
        ->GetDefaultObject<UGameplayEffect>();

    if (
        Effect->DurationPolicy !=
        EGameplayEffectDurationType::Infinite ||
        Effect->GetStackingType() !=
        EGameplayEffectStackingType::None
        )
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[INVENTORY] Equipment effects must use "
                "Infinite duration and Stacking Type None."
            )
        );

        return false;
    }

    FGameplayEffectContextHandle Context =
        ASC->MakeEffectContext();

    Context.AddSourceObject(Definition);

    FGameplayEffectSpecHandle Spec =
        ASC->MakeOutgoingSpec(
            Definition->EquippedEffect,
            1.0f,
            Context
        );

    if (!Spec.IsValid())
    {
        return false;
    }

    /// Initialize every possible additive stat bonus to zero.
    for (const FItemStatGroup& Group : Definition->StatGroups)
    {
        for (const FItemStatRange& Range : Group.StatPool)
        {
            Spec.Data->SetSetByCallerMagnitude(
                Range.StatTag,
                0.0f
            );
        }
    }

    // Replace zero with the actual values selected for this item.
    for (const FItemRolledStat& Stat : Item.RolledStats)
    {
        Spec.Data->SetSetByCallerMagnitude(
            Stat.StatTag,
            Stat.Value
        );
    }

    OutHandle =
        ASC->ApplyGameplayEffectSpecToSelf(
            *Spec.Data.Get()
        );

    return OutHandle.IsValid();
}


void UInventoryComponent::ServerUseSlot_Implementation(
    int32 Index,
    FGuid ExpectedItemId)
{
    const AGenericCharacter* Character =
        Cast<AGenericCharacter>(GetOwner());

    if (
        !Character ||
        Character->bIsDead ||
        !Items.IsValidIndex(Index) ||
        !Items[Index].IsValid() ||
        Items[Index].InstanceId != ExpectedItemId
        )
    {
        return;
    }

    if (Index < InventoryCapacity)
    {
        // Equip an item from the backpack.
        UItemDefinition* Definition =
            Items[Index].Definition.Get();

        int32 EquipmentSlotIndex = INDEX_NONE;

        const int32 EquipmentCount =
            static_cast<int32>(EEquipmentSlot::Count);

        for (
            EEquipmentSlot AllowedSlot :
        Definition->AllowedSlots
            )
        {
            const int32 Candidate =
                static_cast<int32>(AllowedSlot);

            if (Candidate < 0 || Candidate >= EquipmentCount)
            {
                continue;
            }

            // If all allowed positions are occupied,
            // replace the first allowed position.
            if (EquipmentSlotIndex == INDEX_NONE)
            {
                EquipmentSlotIndex = Candidate;
            }

            // Prefer an empty allowed position.
            if (!Items[InventoryCapacity + Candidate].IsValid())
            {
                EquipmentSlotIndex = Candidate;
                break;
            }
        }

        if (
            EquipmentSlotIndex == INDEX_NONE ||
            !EquipmentEffectHandles.IsValidIndex(
                EquipmentSlotIndex
            )
            )
        {
            return;
        }

        UAbilitySystemComponent* ASC = GetASC();

        const FActiveGameplayEffectHandle OldHandle =
            EquipmentEffectHandles[EquipmentSlotIndex];

        if (OldHandle.IsValid() && !ASC)
        {
            return;
        }

        FActiveGameplayEffectHandle NewHandle;

        // Keep existing equipment if the new effect fails.
        if (!ApplyEquipmentEffect(Items[Index], NewHandle))
        {
            return;
        }

        if (OldHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(OldHandle);
        }

        const int32 EquippedIndex =
            InventoryCapacity + EquipmentSlotIndex;

        // The previous equipment returns to the clicked
        // backpack slot, so swapping needs no extra space.
        Swap(
            Items[Index],
            Items[EquippedIndex]
        );

        EquipmentEffectHandles[EquipmentSlotIndex] =
            NewHandle;
    }
    else
    {
        // Unequip an item into the backpack.
        const int32 EmptyIndex =
            FindEmptyInventorySlot();

        if (EmptyIndex == INDEX_NONE)
        {
            // Backpack full: leave the item equipped.
            return;
        }

        const int32 EquipmentSlotIndex =
            Index - InventoryCapacity;

        if (!EquipmentEffectHandles.IsValidIndex(
            EquipmentSlotIndex))
        {
            return;
        }

        UAbilitySystemComponent* ASC = GetASC();

        const FActiveGameplayEffectHandle Handle =
            EquipmentEffectHandles[EquipmentSlotIndex];

        if (Handle.IsValid())
        {
            if (!ASC)
            {
                return;
            }

            ASC->RemoveActiveGameplayEffect(Handle);
        }

        EquipmentEffectHandles[EquipmentSlotIndex] =
            FActiveGameplayEffectHandle{};

        Swap(
            Items[Index],
            Items[EmptyIndex]
        );
    }

    NotifyChanged();
}


void UInventoryComponent::NotifyChanged()
{
    OnInventoryChanged.Broadcast();

    if (GetOwner())
    {
        GetOwner()->ForceNetUpdate();
    }
}


void UInventoryComponent::OnRep_Items()
{
    OnInventoryChanged.Broadcast();
}


void UInventoryComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        if (UAbilitySystemComponent* ASC = GetASC())
        {
            for (
                FActiveGameplayEffectHandle Handle :
            EquipmentEffectHandles
                )
            {
                if (Handle.IsValid())
                {
                    ASC->RemoveActiveGameplayEffect(Handle);
                }
            }
        }
    }

    Super::EndPlay(EndPlayReason);
}

