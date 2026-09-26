#include "GA_GenericAbility.h"

#include "AbilitySystemBlueprintLibrary.h"

#include "../Attributes/ResourceAttributeSet.h"
#include "../TestGameAbilitySystemComponent.h"
#include "../AbilityInputContext.h"
#include "../../Characters/GenericCharacter.h"

UGA_GenericAbility::UGA_GenericAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

AGenericCharacter* UGA_GenericAbility::GetGenericCharacter() const
{
    return Cast<AGenericCharacter>(GetAvatarActorFromActorInfo());
}

UTestGameAbilitySystemComponent*
UGA_GenericAbility::GetTestGameASC() const
{
    return Cast<UTestGameAbilitySystemComponent>(
        GetAbilitySystemComponentFromActorInfo()
    );
}

void UGA_GenericAbility::SendTargetDataToServer(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    UTestGameAbilitySystemComponent* ASC =
        GetTestGameASC();

    if (!ASC)
    {
        return;
    }

    const FAbilityInputContext& InputContext =
        ASC->GetAbilityInputContext();

    const FGameplayAbilityTargetDataHandle TargetData =
        InputContext.MakeTargetData();

    if (TargetData.Num() <= 0)
    {
        return;
    }

    FScopedPredictionWindow ScopedPrediction(ASC);

    ASC->ServerSetReplicatedTargetData(
        Handle,
        ActivationInfo.GetActivationPredictionKey(),
        TargetData,
        FGameplayTag(),
        ASC->ScopedPredictionKey
    );
}

void UGA_GenericAbility::WaitForTargetData()
{
    UAbilitySystemComponent* ASC =
        GetAbilitySystemComponentFromActorInfo();

    if (!ASC)
    {
        return;
    }

    ASC->AbilityTargetDataSetDelegate(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActivationInfo().GetActivationPredictionKey()
    ).AddUObject(
        this,
        &UGA_GenericAbility::HandleTargetDataReceived
    );

    ASC->CallReplicatedTargetDataDelegatesIfSet(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActivationInfo().GetActivationPredictionKey()
    );
}

void UGA_GenericAbility::HandleTargetDataReceived(
    const FGameplayAbilityTargetDataHandle& Data,
    FGameplayTag ActivationTag)
{
    UAbilitySystemComponent* ASC =
        GetAbilitySystemComponentFromActorInfo();

    if (!ASC)
    {
        return;
    }

    ASC->ConsumeClientReplicatedTargetData(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActivationInfo().GetActivationPredictionKey()
    );

    if (Data.Num() <= 0)
    {
        EndAbility(
            GetCurrentAbilitySpecHandle(),
            GetCurrentActorInfo(),
            GetCurrentActivationInfo(),
            true,
            true
        );

        return;
    }

    OnTargetDataReady(Data);
}

void UGA_GenericAbility::OnTargetDataReady(
    const FGameplayAbilityTargetDataHandle& Data)
{
}

const FGameplayTagContainer*
UGA_GenericAbility::GetCooldownTags() const
{
    CooldownTagContainer.Reset();

    if (CooldownTag.IsValid())
    {
        CooldownTagContainer.AddTag(CooldownTag);
    }

    return &CooldownTagContainer;
}

void UGA_GenericAbility::ApplyCooldown(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo) const
{
    if (!CooldownEffectTemplate ||
        CooldownDuration <= 0.0f ||
        !CooldownTag.IsValid())
    {
        return;
    }

    FGameplayEffectSpecHandle SpecHandle =
        MakeOutgoingGameplayEffectSpec(
            Handle,
            ActorInfo,
            ActivationInfo,
            CooldownEffectTemplate,
            GetAbilityLevel(Handle, ActorInfo)
        );

    if (!SpecHandle.IsValid())
    {
        return;
    }

    SpecHandle.Data->SetDuration(
        CooldownDuration,
        true
    );

    SpecHandle.Data->DynamicGrantedTags.AddTag(
        CooldownTag
    );

    ApplyGameplayEffectSpecToOwner(
        Handle,
        ActorInfo,
        ActivationInfo,
        SpecHandle
    );
}

bool UGA_GenericAbility::CheckCost(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    FGameplayTagContainer* OptionalRelevantTags
) const
{
    if (ResourceCost <= 0.0f)
    {
        return true;
    }

    const UAbilitySystemComponent* ASC =
        ActorInfo
        ? ActorInfo->AbilitySystemComponent.Get()
        : nullptr;

    if (!ASC)
    {
        return false;
    }

    const float CurrentResource =
        ASC->GetNumericAttribute(
            UResourceAttributeSet::GetResourceAttribute()
        );

    return CurrentResource >= ResourceCost;
}

void UGA_GenericAbility::ApplyCost(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo
) const
{
    if (ResourceCost <= 0.0f)
    {
        return;
    }

    UAbilitySystemComponent* ASC =
        ActorInfo
        ? ActorInfo->AbilitySystemComponent.Get()
        : nullptr;

    if (!ASC || !ResourceCostEffect)
    {
        return;
    }

    FGameplayEffectContextHandle EffectContext =
        ASC->MakeEffectContext();

    EffectContext.AddSourceObject(
        ActorInfo->AvatarActor.Get()
    );

    FGameplayEffectSpecHandle CostSpec =
        ASC->MakeOutgoingSpec(
            ResourceCostEffect,
            GetAbilityLevel(),
            EffectContext
        );

    if (!CostSpec.IsValid())
    {
        return;
    }

    const FGameplayTag ResourceCostTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("Data.ResourceCost")
        );

    CostSpec.Data->SetSetByCallerMagnitude(
        ResourceCostTag,
        -ResourceCost
    );

    ASC->ApplyGameplayEffectSpecToSelf(
        *CostSpec.Data.Get()
    );
}

bool UGA_GenericAbility::StartTargetedAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo)
{
    if (!ActorInfo)
    {
        return false;
    }

    UTestGameAbilitySystemComponent* ASC =
        GetTestGameASC();

    if (!ASC)
    {
        return false;
    }

    const APawn* AvatarPawn =
        Cast<APawn>(
            ActorInfo->AvatarActor.Get()
        );

    const bool bIsPlayerControlled =
        AvatarPawn &&
        AvatarPawn->IsPlayerControlled();

    // Server copy of a remote PLAYER.
    //
    // This pawn's target came from the owning client's cursor,
    // so the server must wait for replicated TargetData.
    if (ActorInfo->IsNetAuthority() &&
        !ActorInfo->IsLocallyControlled() &&
        bIsPlayerControlled)
    {
        WaitForTargetData();
        return true;
    }

    // Owning client sends its locally collected target
    // to the authoritative server.
    if (!ActorInfo->IsNetAuthority() &&
        ActorInfo->IsLocallyControlled())
    {
        SendTargetDataToServer(
            Handle,
            ActivationInfo
        );
    }

    // Listen-server players and server-controlled AI already
    // have their target context locally.
    const FAbilityInputContext& InputContext =
        ASC->GetAbilityInputContext();

    const FGameplayAbilityTargetDataHandle TargetData =
        InputContext.MakeTargetData();

    if (TargetData.Num() <= 0)
    {
        return false;
    }

    OnTargetDataReady(TargetData);

    return true;
}

void UGA_GenericAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(
        Handle,
        ActorInfo,
        ActivationInfo,
        TriggerEventData
    );

    if (!bRequiresTargetData)
    {
        return;
    }

    if (!StartTargetedAbility(
        Handle,
        ActorInfo,
        ActivationInfo))
    {
        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );
    }
}

FGameplayEffectSpecHandle UGA_GenericAbility::CreateDamageSpec(
    UAbilitySystemComponent* SourceASC,
    TSubclassOf<UGameplayEffect> DamageEffect,
    const FAbilityDamageData& InDamageData) const
{
    if (!SourceASC || !DamageEffect)
    {
        return FGameplayEffectSpecHandle();
    }

    // Gameplay Tags are registered in Project Settings -> Gameplay Tags.
    static const FGameplayTag DataDamageBaseTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.Base"));

    static const FGameplayTag DataDamageWeaponCoefficientTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.WeaponCoefficient"));

    static const FGameplayTag DataDamageStrengthCoefficientTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.StrengthCoefficient"));

    static const FGameplayTag DataDamageDexterityCoefficientTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.DexterityCoefficient"));

    static const FGameplayTag DataDamageIntellectCoefficientTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.IntellectCoefficient"));

    static const FGameplayTag DataDamageAttackPowerCoefficientTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.AttackPowerCoefficient"));

    static const FGameplayTag DataDamageSpellPowerCoefficientTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.SpellPowerCoefficient"));

    static const FGameplayTag DataDamageIsMagicTag =
        FGameplayTag::RequestGameplayTag(FName("Data.Damage.IsMagic"));

    static const FGameplayTag DamageTypePhysicalTag =
        FGameplayTag::RequestGameplayTag(FName("Damage.Type.Physical"));

    static const FGameplayTag DamageTypeMagicTag =
        FGameplayTag::RequestGameplayTag(FName("Damage.Type.Magic"));


    FGameplayEffectContextHandle EffectContext =
        SourceASC->MakeEffectContext();

    EffectContext.AddSourceObject(
        GetAvatarActorFromActorInfo()
    );

    FGameplayEffectSpecHandle DamageSpec =
        SourceASC->MakeOutgoingSpec(
            DamageEffect,
            GetAbilityLevel(),
            EffectContext
        );

    if (!DamageSpec.IsValid())
    {
        return FGameplayEffectSpecHandle();
    }


    // ------------------------------------------------------------
    // Damage scaling data
    // ------------------------------------------------------------

    DamageSpec.Data->SetSetByCallerMagnitude(
        DataDamageBaseTag,
        InDamageData.BaseDamage
    );

    DamageSpec.Data->SetSetByCallerMagnitude(
        DataDamageWeaponCoefficientTag,
        InDamageData.WeaponDamageCoefficient
    );

    DamageSpec.Data->SetSetByCallerMagnitude(
        DataDamageStrengthCoefficientTag,
        InDamageData.StrengthCoefficient
    );

    DamageSpec.Data->SetSetByCallerMagnitude(
        DataDamageDexterityCoefficientTag,
        InDamageData.DexterityCoefficient
    );

    DamageSpec.Data->SetSetByCallerMagnitude(
        DataDamageIntellectCoefficientTag,
        InDamageData.IntellectCoefficient
    );

    DamageSpec.Data->SetSetByCallerMagnitude(
        DataDamageAttackPowerCoefficientTag,
        InDamageData.AttackPowerCoefficient
    );

    DamageSpec.Data->SetSetByCallerMagnitude(
        DataDamageSpellPowerCoefficientTag,
        InDamageData.SpellPowerCoefficient
    );


    // Damage types
    const bool bIsMagic =
        InDamageData.DamageType == EAbilityDamageType::Magic;

    DamageSpec.Data->SetSetByCallerMagnitude(
        DataDamageIsMagicTag,
        bIsMagic ? 1.0f : 0.0f
    );

    DamageSpec.Data->AddDynamicAssetTag(
        bIsMagic
        ? DamageTypeMagicTag
        : DamageTypePhysicalTag
    );


    return DamageSpec;
}

bool UGA_GenericAbility::ApplyDamageToTarget(
    AActor* TargetActor,
    TSubclassOf<UGameplayEffect> DamageEffect,
    const FAbilityDamageData& InDamageData) const
{
    if (!IsValid(TargetActor))
    {
        return false;
    }

    UAbilitySystemComponent* SourceASC =
        GetAbilitySystemComponentFromActorInfo();

    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::
        GetAbilitySystemComponent(TargetActor);

    if (!SourceASC || !TargetASC)
    {
        return false;
    }

    const FGameplayEffectSpecHandle DamageSpec =
        CreateDamageSpec(
            SourceASC,
            DamageEffect,
            InDamageData
        );

    if (!DamageSpec.IsValid())
    {
        return false;
    }

    SourceASC->ApplyGameplayEffectSpecToTarget(
        *DamageSpec.Data.Get(),
        TargetASC
    );

    return true;
}
