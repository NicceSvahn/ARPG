#include "GA_GenericAbility.h"
#include "../../Characters/GenericCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UGA_GenericAbility::UGA_GenericAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

AGenericCharacter* UGA_GenericAbility::GetGenericCharacter() const
{
    return Cast<AGenericCharacter>(GetAvatarActorFromActorInfo());
}

void UGA_GenericAbility::SetAbilityTarget(AActor* Target)
{
    CurrentTarget = Target;
}

AActor* UGA_GenericAbility::GetAbilityTarget() const
{
    return CurrentTarget.Get();
}

void UGA_GenericAbility::MoveToTarget(AActor* Target)
{
    AGenericCharacter* GenericCharacter =
        GetGenericCharacter();

    if (!GenericCharacter || !Target)
    {
        EndAbility(
            CurrentSpecHandle,
            CurrentActorInfo,
            CurrentActivationInfo,
            true,
            true
        );

        return;
    }

    CurrentTarget = Target;

    GenericCharacter->GetWorldTimerManager().SetTimer(
        MoveTimerHandle,
        this,
        &UGA_GenericAbility::UpdateMoveToTarget,
        0.05f,
        true
    );
}

void UGA_GenericAbility::UpdateMoveToTarget()
{
    AGenericCharacter* GenericCharacter =
        GetGenericCharacter();

    AActor* Target = CurrentTarget.Get();

    if (!GenericCharacter || !Target)
    {
        EndAbility(
            CurrentSpecHandle,
            CurrentActorInfo,
            CurrentActivationInfo,
            true,
            true
        );

        return;
    }

    const FVector CharacterLocation =
        GenericCharacter->GetActorLocation();

    const FVector TargetLocation =
        Target->GetActorLocation();

    FVector Direction =
        TargetLocation - CharacterLocation;

    Direction.Z = 0.0f;

    const float Distance = Direction.Size();

    if (Distance <= MoveStopDistance)
    {
        GenericCharacter->GetWorldTimerManager()
            .ClearTimer(MoveTimerHandle);

        OnTargetReached(Target);

        return;
    }

    Direction.Normalize();

    GenericCharacter->SetActorRotation(
        Direction.Rotation()
    );

    GenericCharacter->AddMovementInput(
        Direction,
        1.0f
    );
}

void UGA_GenericAbility::OnTargetReached(AActor* Target)
{
    // Generic abilities don't do anything here.
}





