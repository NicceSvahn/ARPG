#include "GA_GenericMelee.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../AoEAbilities/GA_GenericAoE.h"
#include "../../../Characters/GenericCharacter.h"
#include "../../Damage/GE_Damage.h"

UGA_GenericMelee::UGA_GenericMelee()
{
    bRequiresTargetData = true;
    DamageEffect = UGE_Damage::StaticClass();
    DirectionalTargetClass = AGenericCharacter::StaticClass();

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

bool UGA_GenericMelee::ExtractDirectionalAimLocation(
    const FGameplayAbilityTargetDataHandle& Data,
    FVector& OutAimLocation) const
{
    OutAimLocation = FVector::ZeroVector;

    if (Data.Num() <= 0)
    {
        return false;
    }

    const FGameplayAbilityTargetData* TargetData =
        Data.Get(0);

    if (!TargetData ||
        TargetData->GetScriptStruct() !=
        FGameplayAbilityTargetData_LocationInfo::StaticStruct() ||
        !TargetData->HasEndPoint())
    {
        return false;
    }

    OutAimLocation = TargetData->GetEndPoint();

    return true;
}

void UGA_GenericMelee::OnTargetDataReady(
    const FGameplayAbilityTargetDataHandle& Data)
{
    FVector DirectionalAimLocation;

    // Location-only target data is the explicit network-safe marker for a
    // Shift + melee attack. No actor target is used in this mode.
    if (ExtractDirectionalAimLocation(
        Data,
        DirectionalAimLocation))
    {
        bForceDirectionalAttack = true;
        PendingDirectionalAimLocation = DirectionalAimLocation;

        ExecuteMeleeAbility(nullptr);
        return;
    }

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

    bForceDirectionalAttack = false;
    PendingDirectionalAimLocation = FVector::ZeroVector;

    ExecuteMeleeAbility(TargetActor);
}

void UGA_GenericMelee::FaceWorldLocation(
    AGenericCharacter* Character,
    const FVector& WorldLocation) const
{
    if (!Character)
    {
        return;
    }

    FVector Direction =
        WorldLocation - Character->GetActorLocation();

    Direction.Z = 0.0f;

    if (Direction.IsNearlyZero())
    {
        return;
    }

    Character->SetActorRotation(
        Direction.Rotation()
    );
}

void UGA_GenericMelee::LockMovementForMontage(
    AGenericCharacter* Character)
{
    if (!Character || bMovementLockedForMontage)
    {
        return;
    }

    UCharacterMovementComponent* MovementComponent =
        Character->GetCharacterMovement();

    if (!MovementComponent ||
        MovementComponent->MovementMode == MOVE_None)
    {
        return;
    }

    CachedMovementMode =
        static_cast<uint8>(MovementComponent->MovementMode);

    CachedCustomMovementMode =
        MovementComponent->CustomMovementMode;

    // Remove all current velocity and input, then prevent CharacterMovement
    // from applying any new movement while the melee montage is playing.
    MovementComponent->StopMovementImmediately();
    Character->ConsumeMovementInputVector();
    MovementComponent->DisableMovement();

    bMovementLockedForMontage = true;
}

void UGA_GenericMelee::UnlockMovementAfterMontage()
{
    if (!bMovementLockedForMontage)
    {
        return;
    }

    AGenericCharacter* Character =
        GetGenericCharacter();

    UCharacterMovementComponent* MovementComponent =
        Character
        ? Character->GetCharacterMovement()
        : nullptr;

    // Do not revive locomotion if another state (most importantly death)
    // replaced our temporary MOVE_None while the ability was active.
    if (MovementComponent &&
        !Character->bIsDead &&
        MovementComponent->MovementMode == MOVE_None)
    {
        MovementComponent->SetMovementMode(
            static_cast<EMovementMode>(CachedMovementMode),
            CachedCustomMovementMode
        );
    }

    bMovementLockedForMontage = false;
    CachedMovementMode = 0;
    CachedCustomMovementMode = 0;
}

void UGA_GenericMelee::ExecuteMeleeAbility(
    AActor* TargetActor)
{
    AGenericCharacter* Character =
        GetGenericCharacter();

    const bool bNeedsActorTarget =
        RequiresTarget() &&
        !bForceDirectionalAttack;

    if (!Character ||
        (bNeedsActorTarget && !IsValid(TargetActor)))
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

    UAnimMontage* MontageToPlay =
        GetAttackMontageForActivation();

    PendingTargetActor = TargetActor;
    bMeleeHitTriggered = false;

    if (bForceDirectionalAttack)
    {
        // Shift + melee attacks face the cursor snapshot captured when the
        // input was pressed. This happens before montage/VFX/hit queries.
        FaceWorldLocation(
            Character,
            PendingDirectionalAimLocation
        );
    }
    else if (IsValid(TargetActor))
    {
        // Normal melee attacks face the chased/selected target.
        FaceWorldLocation(
            Character,
            TargetActor->GetActorLocation()
        );
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

    // The character may have been running or chasing immediately before the
    // attack. Freeze locomotion for the complete montage, then restore it in
    // EndAbility. This applies to every ability inheriting GA_GenericMelee.
    LockMovementForMontage(Character);

    MontageTask->ReadyForActivation();
}

AActor* UGA_GenericMelee::FindDirectionalMeleeTarget()
{
    AGenericCharacter* SourceCharacter =
        GetGenericCharacter();

    if (!SourceCharacter ||
        MeleeRange <= 0.0f)
    {
        return nullptr;
    }

    const TArray<AGenericCharacter*> Targets =
        UGA_GenericAoE::FindCharactersInArc(
            this,
            SourceCharacter->GetActorLocation(),
            SourceCharacter->GetActorForwardVector(),
            MeleeRange,
            DirectionalHitArcDegrees,
            SourceCharacter,
            DirectionalTargetClass
        );

    AGenericCharacter* ClosestTarget = nullptr;
    float ClosestDistanceSquared = TNumericLimits<float>::Max();

    for (AGenericCharacter* Target : Targets)
    {
        if (!IsValid(Target))
        {
            continue;
        }

        const float DistanceSquared =
            FVector::DistSquared2D(
                SourceCharacter->GetActorLocation(),
                Target->GetActorLocation()
            );

        if (DistanceSquared < ClosestDistanceSquared)
        {
            ClosestDistanceSquared = DistanceSquared;
            ClosestTarget = Target;
        }
    }

    return ClosestTarget;
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

void UGA_GenericMelee::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const bool bReplicateEndAbility,
    const bool bWasCancelled)
{
    UnlockMovementAfterMontage();

    Super::EndAbility(
        Handle,
        ActorInfo,
        ActivationInfo,
        bReplicateEndAbility,
        bWasCancelled
    );
}

void UGA_GenericMelee::FinishMeleeAbility(
    const bool bWasCancelled)
{
    PendingTargetActor.Reset();

    bForceDirectionalAttack = false;
    PendingDirectionalAimLocation = FVector::ZeroVector;
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
    AActor* ResolvedTarget = TargetActor;

    // Normal targeted attacks already have their actor. A Shift-forced attack
    // has no actor by design, so resolve the closest character actually inside
    // the generic forward melee arc at the hit frame.
    if (!IsValid(ResolvedTarget) &&
        bForceDirectionalAttack)
    {
        ResolvedTarget =
            FindDirectionalMeleeTarget();
    }

    ApplyDamageToTarget(
        ResolvedTarget,
        DamageEffect,
        DamageData
    );
}

UAnimMontage* UGA_GenericMelee::GetAttackMontageForActivation()
{
    return AttackMontage;
}
