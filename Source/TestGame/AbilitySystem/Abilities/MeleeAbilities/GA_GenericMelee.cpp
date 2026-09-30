#include "GA_GenericMelee.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

#include "../../../Characters/GenericCharacter.h"
#include "../../Damage/GE_Damage.h"

UGA_GenericMelee::UGA_GenericMelee()
{
    bRequiresTargetData = true;
    DamageEffect = UGE_Damage::StaticClass();

    MeleeHitEventTag =
        FGameplayTag::RequestGameplayTag(
            FName("Event.Combat.MeleeHit")
        );
}

AActor* UGA_GenericMelee::ExtractTargetActor(
    const FGameplayAbilityTargetDataHandle& Data) const
{
    if (Data.Num() <= 0)
    {
        return nullptr;
    }

    const FGameplayAbilityTargetData* TargetData =
        Data.Get(0);

    if (!TargetData)
    {
        return nullptr;
    }

    if (const FHitResult* HitResult =
        TargetData->GetHitResult())
    {
        return HitResult->GetActor();
    }

    const TArray<TWeakObjectPtr<AActor>> TargetActors =
        TargetData->GetActors();

    if (!TargetActors.IsEmpty() &&
        TargetActors[0].IsValid())
    {
        return TargetActors[0].Get();
    }

    return nullptr;
}

void UGA_GenericMelee::OnTargetDataReady(
    const FGameplayAbilityTargetDataHandle& Data)
{
    AActor* TargetActor =
        ExtractTargetActor(Data);

    if (!IsValid(TargetActor))
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

    ExecuteMeleeAbility(TargetActor);
}

void UGA_GenericMelee::ExecuteMeleeAbility(
    AActor* TargetActor)
{

    AGenericCharacter* Character =
        GetGenericCharacter();

    if (!Character ||
        (RequiresTarget() && !IsValid(TargetActor)))
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


    if (!CommitAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo()))
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

    UAnimMontage* MontageToPlay = GetAttackMontageForActivation();

    PendingTargetActor = TargetActor;

    bMeleeHitTriggered = false;

    // Targeted abilities such as Bash face their target.
    // Cleave has nullptr here and therefore keeps the
    // direction the player is already facing.
    if (IsValid(TargetActor))
    {
        FVector Direction =
            TargetActor->GetActorLocation() -
            Character->GetActorLocation();

        Direction.Z = 0.0f;

        if (!Direction.IsNearlyZero())
        {
            Character->SetActorRotation(
                Direction.Rotation()
            );
        }
    }

    // Allows a melee ability without a montage to still work.
    if (!MontageToPlay)
    {
        const FGameplayAbilityActorInfo* ActorInfo =
            GetCurrentActorInfo();

        if (ActorInfo &&
            ActorInfo->IsNetAuthority())
        {
            OnMeleeHit(
                PendingTargetActor.Get()
            );
        }

        FinishMeleeAbility(false);
        return;
    }

    // Start listening before the montage begins so that
    // the hit-frame event cannot be missed.
    HitEventTask =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
            this,
            MeleeHitEventTag,
            nullptr,
            true,
            true
        );

    if (!HitEventTask)
    {
        FinishMeleeAbility(true);
        return;
    }

    HitEventTask->EventReceived.AddDynamic(
        this,
        &UGA_GenericMelee::HandleMeleeHitEvent
    );

    HitEventTask->ReadyForActivation();

    MontageTask =
        UAbilityTask_PlayMontageAndWait::
        CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            MontageToPlay,
            1.0f,
            NAME_None
        );

    if (!MontageTask)
    {
        FinishMeleeAbility(true);
        return;
    }

    MontageTask->OnCompleted.AddDynamic(
        this,
        &UGA_GenericMelee::HandleMontageCompleted
    );

    MontageTask->OnInterrupted.AddDynamic(
        this,
        &UGA_GenericMelee::HandleMontageInterrupted
    );

    MontageTask->OnCancelled.AddDynamic(
        this,
        &UGA_GenericMelee::HandleMontageCancelled
    );

    MontageTask->ReadyForActivation();
}

void UGA_GenericMelee::HandleMeleeHitEvent(
    FGameplayEventData Payload)
{
    const FGameplayAbilityActorInfo* ActorInfo =
        GetCurrentActorInfo();

    if (!ActorInfo ||
        !ActorInfo->IsNetAuthority())
    {
        return;
    }

    // Prevent duplicate damage if a montage accidentally
    // contains multiple identical hit notifies.
    if (bMeleeHitTriggered)
    {
        return;
    }

    bMeleeHitTriggered = true;

    OnMeleeHit(
        PendingTargetActor.Get()
    );
}

void UGA_GenericMelee::HandleMontageCompleted()
{
    const FGameplayAbilityActorInfo* ActorInfo =
        GetCurrentActorInfo();

    // Fallback:
    // If no animation hit event was received, still perform
    // the attack at montage completion.
    if (!bMeleeHitTriggered &&
        ActorInfo &&
        ActorInfo->IsNetAuthority())
    {
        bMeleeHitTriggered = true;

        OnMeleeHit(
            PendingTargetActor.Get()
        );
    }

    FinishMeleeAbility(false);
}

void UGA_GenericMelee::HandleMontageInterrupted()
{
    FinishMeleeAbility(true);
}

void UGA_GenericMelee::HandleMontageCancelled()
{
    FinishMeleeAbility(true);
}

void UGA_GenericMelee::FinishMeleeAbility(
    const bool bWasCancelled)
{
    PendingTargetActor.Reset();

    bMeleeHitTriggered = false;

    MontageTask = nullptr;
    HitEventTask = nullptr;

    EndAbility(
        GetCurrentAbilitySpecHandle(),
        GetCurrentActorInfo(),
        GetCurrentActivationInfo(),
        true,
        bWasCancelled
    );
}

void UGA_GenericMelee::OnMeleeHit(
    AActor* TargetActor)
{
    ApplyDamageToTarget(
        TargetActor,
        DamageEffect,
        DamageData
    );
}

UAnimMontage* UGA_GenericMelee::GetAttackMontageForActivation()
{
    return AttackMontage;
}