#include "GA_Whirlwind.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Components/SceneComponent.h"
#include "GameplayEffect.h"
#include "TimerManager.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

#include "TestGame/AbilitySystem/Abilities/AoEAbilities/GA_GenericAoE.h"
#include "TestGame/AbilitySystem/Attributes/ResourceAttributeSet.h"
#include "TestGame/AbilitySystem/Damage/GE_Damage.h"

#include "TestGame/Characters/EnemyCharacter.h"
#include "TestGame/Characters/GenericCharacter.h"


UGA_Whirlwind::UGA_Whirlwind()
{
    DamageEffect =
        UGE_Damage::StaticClass();

    AffectedCharacterClass =
        AEnemyCharacter::StaticClass();

    DamageData.DamageType =
        EAbilityDamageType::Physical;
}


// ACTIVATE
void UGA_Whirlwind::ActivateAbility(
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

    if (
        !ActorInfo ||
        !ActorInfo->AvatarActor.IsValid() ||
        !ActorInfo->AbilitySystemComponent.IsValid()
        )
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

    if (!WhirlwindMontage)
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

    if (!DamageEffect)
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

    if (
        ResourceCostPerTick > 0.0f &&
        !ResourceCostEffect
        )
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

    if (!CanPayTickCost())
    {
        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            false
        );

        return;
    }

    StartWhirlwind();
}


// START
void UGA_Whirlwind::StartWhirlwind()
{
    if (bWhirlwindActive)
    {
        return;
    }

    bWhirlwindActive = true;

    // MONTAGE
    MontageTask =
        UAbilityTask_PlayMontageAndWait::
        CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            WhirlwindMontage,
            1.0f,
            NAME_None,
            false
        );

    if (!MontageTask)
    {
        StopWhirlwind(true);
        return;
    }

    MontageTask->OnCompleted.AddDynamic(
        this,
        &UGA_Whirlwind::HandleMontageCompleted
    );

    MontageTask->OnInterrupted.AddDynamic(
        this,
        &UGA_Whirlwind::HandleMontageInterrupted
    );

    MontageTask->OnCancelled.AddDynamic(
        this,
        &UGA_Whirlwind::HandleMontageCancelled
    );

    MontageTask->ReadyForActivation();

    // INPUT RELEASE
    InputReleaseTask =
        UAbilityTask_WaitInputRelease::
        WaitInputRelease(
            this,
            false
        );

    if (!InputReleaseTask)
    {
        StopWhirlwind(true);
        return;
    }

    InputReleaseTask->OnRelease.AddDynamic(
        this,
        &UGA_Whirlwind::HandleInputReleased
    );

    InputReleaseTask->ReadyForActivation();

    // COSMETIC VFX
    StartWhirlwindEffect();

    // SERVER GAMEPLAY
    if (
        CurrentActorInfo &&
        CurrentActorInfo->IsNetAuthority()
        )
    {
        StartGameplayTimer();
    }
}


// VFX
void UGA_Whirlwind::StartWhirlwindEffect()
{
    if (
        !WhirlwindEffect ||
        WhirlwindEffectComponent
        )
    {
        return;
    }

    AActor* AvatarActor =
        GetAvatarActorFromActorInfo();

    if (!AvatarActor)
    {
        return;
    }

    USceneComponent* AttachComponent =
        AvatarActor->GetRootComponent();

    if (!AttachComponent)
    {
        return;
    }

    WhirlwindEffectComponent =
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            WhirlwindEffect,
            AttachComponent,
            NAME_None,
            WhirlwindEffectOffset,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false,
            true,
            ENCPoolMethod::None,
            true
        );
}


void UGA_Whirlwind::StopWhirlwindEffect()
{
    if (!WhirlwindEffectComponent)
    {
        return;
    }

    WhirlwindEffectComponent->Deactivate();

    WhirlwindEffectComponent = nullptr;
}


// GAMEPLAY TIMER
void UGA_Whirlwind::StartGameplayTimer()
{
    UWorld* World =
        GetWorld();

    if (!World)
    {
        StopWhirlwind(true);
        return;
    }

    World->GetTimerManager().SetTimer(
        GameplayTickTimer,
        this,
        &UGA_Whirlwind::PerformGameplayTick,
        TickInterval,
        true,
        0.0f
    );
}


void UGA_Whirlwind::StopGameplayTimer()
{
    UWorld* World =
        GetWorld();

    if (!World)
    {
        return;
    }

    World->GetTimerManager().ClearTimer(
        GameplayTickTimer
    );
}


// GAMEPLAY TICK
void UGA_Whirlwind::PerformGameplayTick()
{
    if (
        !bWhirlwindActive ||
        !CurrentActorInfo ||
        !CurrentActorInfo->IsNetAuthority()
        )
    {
        return;
    }

    if (!CanPayTickCost())
    {
        StopWhirlwind(false);
        return;
    }

    if (!ApplyTickCost())
    {
        StopWhirlwind(true);
        return;
    }

    ApplyWhirlwindDamage();
}


// RESOURCE
bool UGA_Whirlwind::CanPayTickCost() const
{
    if (ResourceCostPerTick <= 0.0f)
    {
        return true;
    }

    const UAbilitySystemComponent* ASC =
        GetAbilitySystemComponentFromActorInfo();

    if (!ASC)
    {
        return false;
    }

    const float CurrentResource =
        ASC->GetNumericAttribute(
            UResourceAttributeSet::
            GetResourceAttribute()
        );

    return CurrentResource >=
        ResourceCostPerTick;
}


bool UGA_Whirlwind::ApplyTickCost()
{
    if (ResourceCostPerTick <= 0.0f)
    {
        return true;
    }

    UAbilitySystemComponent* ASC =
        GetAbilitySystemComponentFromActorInfo();

    if (
        !ASC ||
        !ResourceCostEffect
        )
    {
        return false;
    }

    FGameplayEffectContextHandle EffectContext =
        ASC->MakeEffectContext();

    EffectContext.AddSourceObject(
        GetAvatarActorFromActorInfo()
    );

    FGameplayEffectSpecHandle CostSpec =
        ASC->MakeOutgoingSpec(
            ResourceCostEffect,
            GetAbilityLevel(),
            EffectContext
        );

    if (!CostSpec.IsValid())
    {
        return false;
    }

    const FGameplayTag ResourceCostTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("Data.ResourceCost")
        );

    CostSpec.Data->SetSetByCallerMagnitude(
        ResourceCostTag,
        -ResourceCostPerTick
    );

    ASC->ApplyGameplayEffectSpecToSelf(
        *CostSpec.Data.Get()
    );

    return true;
}


// DAMAGE
void UGA_Whirlwind::ApplyWhirlwindDamage()
{
    AGenericCharacter* SourceCharacter =
        GetGenericCharacter();

    if (!SourceCharacter)
    {
        return;
    }

    if (!SourceCharacter->HasAuthority())
    {
        return;
    }

    const TArray<AGenericCharacter*> Targets =
        UGA_GenericAoE::FindCharactersInRadius(
            this,
            SourceCharacter->GetActorLocation(),
            Radius,
            SourceCharacter,
            AffectedCharacterClass
        );

    for (AGenericCharacter* Target : Targets)
    {
        if (!IsValid(Target))
        {
            continue;
        }

        ApplyDamageToTarget(
            Target,
            DamageEffect,
            DamageData
        );
    }
}


// INPUT RELEASE
void UGA_Whirlwind::HandleInputReleased(
    float TimeHeld)
{
    StopWhirlwind(false);
}


// MONTAGE CALLBACKS
void UGA_Whirlwind::HandleMontageCompleted()
{
    StopWhirlwind(false);
}


void UGA_Whirlwind::HandleMontageInterrupted()
{
    StopWhirlwind(true);
}


void UGA_Whirlwind::HandleMontageCancelled()
{
    StopWhirlwind(true);
}


// STOP
void UGA_Whirlwind::StopWhirlwind(
    bool bWasCancelled)
{
    if (!bWhirlwindActive)
    {
        return;
    }

    bWhirlwindActive = false;

    StopGameplayTimer();

    EndAbility(
        CurrentSpecHandle,
        CurrentActorInfo,
        CurrentActivationInfo,
        true,
        bWasCancelled
    );
}


// END ABILITY
void UGA_Whirlwind::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    bWhirlwindActive = false;

    StopGameplayTimer();

    // STOP VFX
    StopWhirlwindEffect();

    // STOP MONTAGE
    if (
        ActorInfo &&
        ActorInfo->AbilitySystemComponent.IsValid()
        )
    {
        ActorInfo
            ->AbilitySystemComponent
            ->CurrentMontageStop();
    }

    // CLEANUP TASKS
    if (MontageTask)
    {
        MontageTask->EndTask();
        MontageTask = nullptr;
    }

    if (InputReleaseTask)
    {
        InputReleaseTask->EndTask();
        InputReleaseTask = nullptr;
    }

    Super::EndAbility(
        Handle,
        ActorInfo,
        ActivationInfo,
        bReplicateEndAbility,
        bWasCancelled
    );
}