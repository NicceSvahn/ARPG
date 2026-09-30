#include "InventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"
#include "Abilities/GameplayAbility.h"
#include "Templates/UnrealTemplate.h"

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


    bool ValidateGenerationSettings(
        const UItemDefinition* Definition,
        const UItemGenerationProfile* Profile)
    {
        if (!IsValid(Definition))
        {
            return false;
        }

        // No selected profile means guaranteed stats only.
        if (!Profile)
        {
            return true;
        }

        TSet<FName> GroupNames;

        for (const FItemStatGroup& Group : Definition->StatGroups)
        {
            if (Group.GroupName.IsNone() ||
                GroupNames.Contains(Group.GroupName))
            {
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT("[INVENTORY] Empty or duplicate group name on %s"),
                    *GetNameSafe(Definition)
                );

                return false;
            }

            GroupNames.Add(Group.GroupName);
        }

        TSet<FName> RuleNames;

        for (const FItemAffixCountRule& Rule : Profile->AffixRules)
        {
            if (Rule.GroupName.IsNone() ||
                RuleNames.Contains(Rule.GroupName) ||
                Rule.MinCount < 0 ||
                Rule.MaxCount < Rule.MinCount)
            {
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT("[INVENTORY] Invalid affix rule in %s"),
                    *GetNameSafe(Profile)
                );

                return false;
            }

            RuleNames.Add(Rule.GroupName);

            const FItemStatGroup* MatchingGroup = nullptr;

            for (const FItemStatGroup& Group : Definition->StatGroups)
            {
                if (Group.GroupName == Rule.GroupName)
                {
                    MatchingGroup = &Group;
                    break;
                }
            }

            if (!MatchingGroup)
            {
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT("[INVENTORY] Profile group '%s' is missing on %s"),
                    *Rule.GroupName.ToString(),
                    *GetNameSafe(Definition)
                );

                return false;
            }

            if (MatchingGroup->bGuaranteed)
            {
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT(
                        "[INVENTORY] Profile must not assign affix counts "
                        "to guaranteed group '%s' on %s"
                    ),
                    *Rule.GroupName.ToString(),
                    *GetNameSafe(Definition)
                );

                return false;
            }

            // Reject the configuration up front instead of randomly
            // failing only when a count larger than the pool is selected.
            if (Rule.MaxCount > MatchingGroup->StatPool.Num())
            {
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT(
                        "[INVENTORY] Group '%s' on %s needs at least "
                        "%d pool entries"
                    ),
                    *Rule.GroupName.ToString(),
                    *GetNameSafe(Definition),
                    Rule.MaxCount
                );

                return false;
            }
        }

        return true;
    }


    bool SelectGenerationProfile(
        const UItemDefinition* Definition,
        const UItemGenerationProfile*& OutProfile)
    {
        OutProfile = nullptr;

        if (!IsValid(Definition))
        {
            return false;
        }

        if (Definition->GenerationChoices.Num() > 3)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("[INVENTORY] %s has more than 3 generation choices"),
                *GetNameSafe(Definition)
            );

            return false;
        }

        // Validate the definition even if it has no choices.
        if (!ValidateGenerationSettings(Definition, nullptr))
        {
            return false;
        }

        int32 TotalChance = 0;

        // Validate all choices before rolling so bad configuration
        // does not cause intermittent item-creation failures.
        for (const FItemGenerationChoice& Choice :
            Definition->GenerationChoices)
        {
            if (!IsValid(Choice.Profile.Get()) ||
                Choice.ChancePercent < 0 ||
                Choice.ChancePercent > 100)
            {
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT("[INVENTORY] Invalid generation choice on %s"),
                    *GetNameSafe(Definition)
                );

                return false;
            }

            TotalChance += Choice.ChancePercent;

            if (!ValidateGenerationSettings(
                Definition,
                Choice.Profile.Get()))
            {
                return false;
            }
        }

        if (TotalChance > 100)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("[INVENTORY] Generation chances exceed 100%% on %s"),
                *GetNameSafe(Definition)
            );

            return false;
        }

        if (TotalChance == 0)
        {
            return true;
        }

        const int32 Roll = FMath::RandRange(1, 100);

        int32 CumulativeChance = 0;

        for (const FItemGenerationChoice& Choice :
            Definition->GenerationChoices)
        {
            CumulativeChance += Choice.ChancePercent;

            if (Roll <= CumulativeChance)
            {
                OutProfile = Choice.Profile.Get();
                return true;
            }
        }

        // The roll landed in the unassigned percentage.
        // OutProfile stays null: guaranteed stats only.
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
    EquipmentAbilityHandles.SetNum(EquipmentCount);

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


bool UInventoryComponent::AddItem(
    UItemDefinition* Definition)
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

    const UItemGenerationProfile* SelectedProfile = nullptr;

    if (!SelectGenerationProfile(Definition, SelectedProfile))
    {
        return false;
    }

    FInventoryItem NewItem;
    NewItem.InstanceId = FGuid::NewGuid();
    NewItem.Definition = Definition;

    NewItem.Rarity = SelectedProfile
        ? SelectedProfile->Rarity
        : Definition->Rarity;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[INVENTORY] Generated %s | Profile=%s | Rarity=%s"),
        *GetNameSafe(Definition),
        *GetNameSafe(SelectedProfile),
        *ItemRarity::GetDisplayName(NewItem.Rarity).ToString()
    );

    TSet<FGameplayTag> ConfiguredTags;

    for (const FItemStatGroup& Group : Definition->StatGroups)
    {
        int32 CountToRoll = 0;

        if (Group.bGuaranteed)
        {
            CountToRoll = Group.StatPool.Num();
        }
        else if (SelectedProfile)
        {
            for (const FItemAffixCountRule& Rule :
                SelectedProfile->AffixRules)
            {
                if (Rule.GroupName == Group.GroupName)
                {
                    CountToRoll = FMath::RandRange(
                        Rule.MinCount,
                        Rule.MaxCount
                    );

                    break;
                }
            }
        }

        const int32 FirstNewStatIndex =
            NewItem.RolledStats.Num();

        if (!RollStatsFromPool(
            Group.StatPool,
            CountToRoll,
            ConfiguredTags,
            NewItem.RolledStats))
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("[INVENTORY] Invalid stat pool '%s' on %s"),
                *Group.GroupName.ToString(),
                *GetNameSafe(Definition)
            );

            return false;
        }

        for (int32 StatIndex = FirstNewStatIndex;
            StatIndex < NewItem.RolledStats.Num();
            ++StatIndex)
        {
            NewItem.RolledStats[StatIndex].bGuaranteed =
                Group.bGuaranteed;
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
    if (!GetOwner() ||
        !GetOwner()->HasAuthority() ||
        bChangingEquipment)
    {
        return;
    }

    const AGenericCharacter* Character =
        Cast<AGenericCharacter>(GetOwner());

    if (!IsValid(Character) ||
        Character->bIsDead ||
        !Items.IsValidIndex(Index) ||
        !Items[Index].IsValid() ||
        Items[Index].InstanceId != ExpectedItemId)
    {
        return;
    }

    TGuardValue<bool> ChangeGuard(bChangingEquipment, true);

    if (Index < InventoryCapacity)
    {
        // Equip an item from the backpack.
        UItemDefinition* Definition =
            Items[Index].Definition.Get();

        if (!IsValid(Definition))
        {
            return;
        }

        int32 EquipmentSlotIndex = INDEX_NONE;

        const int32 EquipmentCount =
            static_cast<int32>(EEquipmentSlot::Count);

        for (const EEquipmentSlot AllowedSlot :
        Definition->AllowedSlots)
        {
            const int32 Candidate =
                static_cast<int32>(AllowedSlot);

            if (Candidate < 0 || Candidate >= EquipmentCount)
            {
                continue;
            }

            const int32 CandidateIndex =
                InventoryCapacity + Candidate;

            if (!Items.IsValidIndex(CandidateIndex))
            {
                continue;
            }

            // Replace the first allowed position if all are occupied.
            if (EquipmentSlotIndex == INDEX_NONE)
            {
                EquipmentSlotIndex = Candidate;
            }

            // Prefer an empty allowed position.
            if (!Items[CandidateIndex].IsValid())
            {
                EquipmentSlotIndex = Candidate;
                break;
            }
        }

        if (EquipmentSlotIndex == INDEX_NONE ||
            !EquipmentEffectHandles.IsValidIndex(EquipmentSlotIndex) ||
            !EquipmentAbilityHandles.IsValidIndex(EquipmentSlotIndex))
        {
            return;
        }

        UAbilitySystemComponent* ASC = GetASC();

        const FActiveGameplayEffectHandle OldEffectHandle =
            EquipmentEffectHandles[EquipmentSlotIndex];

        const bool bHasOldAbility =
            EquipmentAbilityHandles[EquipmentSlotIndex].IsValid();

        // Existing grants need the ASC for removal.
        if ((OldEffectHandle.IsValid() || bHasOldAbility) &&
            !IsValid(ASC))
        {
            return;
        }

        // A new item ability requires initialized actor information.
        if (Definition->EquippedAbility &&
            (!IsValid(ASC) || ASC->GetAvatarActor() != GetOwner()))
        {
            return;
        }

        FActiveGameplayEffectHandle NewEffectHandle;

        // Preserve the previous equipment if applying the new
        // item's stat effect fails.
        if (!ApplyEquipmentEffect(Items[Index], NewEffectHandle))
        {
            return;
        }

        // Stop the previous item's ability before starting the new one.
        RemoveEquipmentAbility(EquipmentSlotIndex);

        if (OldEffectHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(OldEffectHandle);
        }

        const int32 EquippedIndex =
            InventoryCapacity + EquipmentSlotIndex;

        // Return replaced equipment to the clicked backpack slot.
        Swap(Items[Index], Items[EquippedIndex]);

        EquipmentEffectHandles[EquipmentSlotIndex] =
            NewEffectHandle;

        // The new item is now in its equipment slot, so its ability
        // can inspect the equipped inventory during activation.
        GrantAndActivateEquipmentAbility(
            Items[EquippedIndex],
            EquipmentSlotIndex
        );
    }
    else
    {
        // Unequip an item into the backpack.
        const int32 EmptyIndex = FindEmptyInventorySlot();

        if (EmptyIndex == INDEX_NONE)
        {
            // Backpack full: preserve the item and its ability.
            return;
        }

        const int32 EquipmentSlotIndex =
            Index - InventoryCapacity;

        if (!EquipmentEffectHandles.IsValidIndex(EquipmentSlotIndex) ||
            !EquipmentAbilityHandles.IsValidIndex(EquipmentSlotIndex))
        {
            return;
        }

        UAbilitySystemComponent* ASC = GetASC();

        const FActiveGameplayEffectHandle EffectHandle =
            EquipmentEffectHandles[EquipmentSlotIndex];

        const bool bHasAbility =
            EquipmentAbilityHandles[EquipmentSlotIndex].IsValid();

        if ((EffectHandle.IsValid() || bHasAbility) &&
            !IsValid(ASC))
        {
            return;
        }

        RemoveEquipmentAbility(EquipmentSlotIndex);

        if (EffectHandle.IsValid())
        {
            ASC->RemoveActiveGameplayEffect(EffectHandle);
        }

        EquipmentEffectHandles[EquipmentSlotIndex] =
            FActiveGameplayEffectHandle();

        Swap(Items[Index], Items[EmptyIndex]);
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
    // Block equipment changes triggered by teardown callbacks.
    bChangingEquipment = true;

    if (GetOwner() && GetOwner()->HasAuthority())
    {
        for (int32 EquipmentSlotIndex = 0;
            EquipmentSlotIndex < EquipmentAbilityHandles.Num();
            ++EquipmentSlotIndex)
        {
            RemoveEquipmentAbility(EquipmentSlotIndex);
        }

        if (UAbilitySystemComponent* ASC = GetASC())
        {
            for (const FActiveGameplayEffectHandle EffectHandle :
            EquipmentEffectHandles)
            {
                if (EffectHandle.IsValid())
                {
                    ASC->RemoveActiveGameplayEffect(EffectHandle);
                }
            }
        }
    }

    EquipmentAbilityHandles.Reset();
    EquipmentEffectHandles.Reset();

    Super::EndPlay(EndPlayReason);
}


void UInventoryComponent::GrantAndActivateEquipmentAbility(
    const FInventoryItem& Item,
    int32 EquipmentSlotIndex)
{
    if (!GetOwner() ||
        !GetOwner()->HasAuthority() ||
        !Item.IsValid() ||
        !EquipmentAbilityHandles.IsValidIndex(EquipmentSlotIndex))
    {
        return;
    }

    UItemDefinition* Definition = Item.Definition.Get();

    if (!IsValid(Definition) || !Definition->EquippedAbility)
    {
        return;
    }

    // Never overwrite a tracked ability handle.
    if (EquipmentAbilityHandles[EquipmentSlotIndex].IsValid())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[INVENTORY] Equipment slot already has an ability.")
        );

        return;
    }

    UAbilitySystemComponent* ASC = GetASC();

    if (!IsValid(ASC) || ASC->GetAvatarActor() != GetOwner())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[INVENTORY] ASC is not ready for item ability: %s"),
            *GetNameSafe(Definition)
        );

        return;
    }

    FGameplayAbilitySpec AbilitySpec(
        Definition->EquippedAbility,
        1,
        INDEX_NONE,
        Definition
    );

    const FGameplayAbilitySpecHandle AbilityHandle =
        ASC->GiveAbility(AbilitySpec);

    if (!AbilityHandle.IsValid())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[INVENTORY] Could not grant ability for item: %s"),
            *GetNameSafe(Definition)
        );

        return;
    }

    // Store the handle before activation, which may invoke callbacks.
    EquipmentAbilityHandles[EquipmentSlotIndex] = AbilityHandle;

    // Activate here on the server. Do not forward activation
    // to a client for a locally executed ability.
    const bool bActivated =
        ASC->TryActivateAbility(AbilityHandle, false);

    if (!bActivated)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[INVENTORY] Item %s equipped, but ability %s "
                "could not activate. Check network policy, "
                "costs, cooldowns, and activation requirements."
            ),
            *GetNameSafe(Definition),
            *GetNameSafe(Definition->EquippedAbility.Get())
        );

        // The item remains equipped. Remove the failed grant.
        RemoveEquipmentAbility(EquipmentSlotIndex);
    }
}


void UInventoryComponent::RemoveEquipmentAbility(
    int32 EquipmentSlotIndex)
{
    if (!GetOwner() ||
        !GetOwner()->HasAuthority() ||
        !EquipmentAbilityHandles.IsValidIndex(EquipmentSlotIndex))
    {
        return;
    }

    const FGameplayAbilitySpecHandle AbilityHandle =
        EquipmentAbilityHandles[EquipmentSlotIndex];

    if (!AbilityHandle.IsValid())
    {
        return;
    }

    UAbilitySystemComponent* ASC = GetASC();

    if (!IsValid(ASC))
    {
        return;
    }

    // Clear our tracking before cancellation invokes callbacks.
    EquipmentAbilityHandles[EquipmentSlotIndex] =
        FGameplayAbilitySpecHandle();

    ASC->CancelAbilityHandle(AbilityHandle);
    ASC->ClearAbility(AbilityHandle);
}