#include "GA_FrostNova.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameplayEffect.h"
#include "../../Abilities/AoEAbilities/GA_GenericAoE.h"
#include "TestGame/Characters/EnemyCharacter.h"
#include "TestGame/Characters/GenericCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UGA_FrostNova::UGA_FrostNova()
{
    InstancingPolicy =
        EGameplayAbilityInstancingPolicy::InstancedPerActor;

    AffectedCharacterClass = AEnemyCharacter::StaticClass();

    DamageData.BaseDamage = 15.0f;
    DamageData.IntellectCoefficient = 0.25f;
    DamageData.SpellPowerCoefficient = 0.50f;
    DamageData.DamageType = EAbilityDamageType::Magic;
}

void UGA_FrostNova::ActivateAbility(
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

    AGenericCharacter* SourceCharacter =
        GetGenericCharacter();

    UAbilitySystemComponent* SourceASC =
        GetAbilitySystemComponentFromActorInfo();

    if (!SourceCharacter ||
        !SourceASC ||
        !DamageEffect ||
        !FreezeEffect)
    {
        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    if (!CommitAbility(
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

        return;
    }

    PlayFrostNovaVisuals(
        SourceCharacter
    );

    if (ActorInfo->IsNetAuthority())
    {
        ApplyFrostNovaGameplay(
            SourceCharacter,
            SourceASC
        );
    }

    EndAbility(
        Handle,
        ActorInfo,
        ActivationInfo,
        true,
        false
    );
}

void UGA_FrostNova::ApplyFrostNovaGameplay(
    AGenericCharacter* SourceCharacter,
    UAbilitySystemComponent* SourceASC)
{
    if (!SourceCharacter ||
        !SourceASC)
    {
        return;
    }

    if (!SourceCharacter->HasAuthority())
    {
        return;
    }

    const FVector NovaOrigin =
        SourceCharacter->GetActorLocation();

    const TArray<AGenericCharacter*> Targets =
        UGA_GenericAoE::FindCharactersInRadius(
            this,
            NovaOrigin,
            NovaRadius,
            SourceCharacter,
            AffectedCharacterClass
        );

    for (AGenericCharacter* Target : Targets)
    {
        if (!IsValid(Target))
        {
            continue;
        }

        UAbilitySystemComponent* TargetASC =
            Target->GetAbilitySystemComponent();

        if (!TargetASC)
        {
            continue;
        }

        ApplyDamageToTarget(
            Target,
            DamageEffect,
            DamageData
        );

        ApplyEffectToTarget(
            SourceASC,
            TargetASC,
            FreezeEffect,
            FreezeDuration
        );
    }
}

bool UGA_FrostNova::ApplyEffectToTarget(
    UAbilitySystemComponent* SourceASC,
    UAbilitySystemComponent* TargetASC,
    TSubclassOf<UGameplayEffect> EffectClass,
    const float DurationOverride)
{
    if (!SourceASC || !TargetASC || !EffectClass)
    {
        return false;
    }

    FGameplayEffectContextHandle EffectContext =
        SourceASC->MakeEffectContext();

    EffectContext.AddSourceObject(this);

    FGameplayEffectSpecHandle EffectSpec =
        SourceASC->MakeOutgoingSpec(
            EffectClass,
            GetAbilityLevel(),
            EffectContext
        );

    if (!EffectSpec.IsValid())
    {
        return false;
    }

    if (DurationOverride > 0.0f)
    {
        EffectSpec.Data->SetDuration(
            DurationOverride,
            true
        );
    }

    SourceASC->ApplyGameplayEffectSpecToTarget(
        *EffectSpec.Data.Get(),
        TargetASC
    );

    return true;
}

void UGA_FrostNova::PlayFrostNovaVisuals(
    AGenericCharacter* SourceCharacter)
{
    if (!SourceCharacter)
    {
        return;
    }

    if (FrostNovaMontage)
    {
        SourceCharacter->PlayAnimMontage(
            FrostNovaMontage
        );
    }

    if (FrostNovaEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this,
            FrostNovaEffect,
            SourceCharacter->GetActorLocation(),
            FRotator::ZeroRotator,
            FVector::OneVector,
            true,
            true,
            ENCPoolMethod::AutoRelease,
            true
        );
    }
}