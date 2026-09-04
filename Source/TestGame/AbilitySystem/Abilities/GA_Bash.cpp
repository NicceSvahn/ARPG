#include "GA_Bash.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "../../Characters/GenericCharacter.h"
#include "../../Player/TestGamePlayerController.h"

UGA_Bash::UGA_Bash()
{
    InstancingPolicy =
        EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Bash::ActivateAbility(
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

    if (!TriggerEventData)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Bash activated without TriggerEventData")
        );

        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    const AActor* EventTarget =
        TriggerEventData->Target.Get();

    if (!EventTarget)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Bash activated without a target")
        );

        EndAbility(
            Handle,
            ActorInfo,
            ActivationInfo,
            true,
            true
        );

        return;
    }

    Target = const_cast<AActor*>(EventTarget);

    if (IsTargetInRange())
    {
        PerformBash();
        return;
    }

    RequestMoveIntoRange();
}

bool UGA_Bash::IsTargetInRange() const
{
    const AGenericCharacter* Character =
        GetGenericCharacter();

    const AActor* TargetActor =
        Target.Get();

    if (!Character || !TargetActor)
    {
        return false;
    }

    const float Distance =
        FVector::Dist2D(
            Character->GetActorLocation(),
            TargetActor->GetActorLocation()
        );

    return Distance <= BashRange;
}

void UGA_Bash::RequestMoveIntoRange()
{
    AGenericCharacter* Character =
        GetGenericCharacter();

    if (!Character || !Target.IsValid())
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

    ATestGamePlayerController* Controller =
        Cast<ATestGamePlayerController>(
            Character->GetController()
        );

    if (!Controller)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("Bash: Avatar is not controlled by TestGamePlayerController")
        );

        EndAbility(
            CurrentSpecHandle,
            CurrentActorInfo,
            CurrentActivationInfo,
            true,
            true
        );

        return;
    }

    Controller->MoveIntoRange(
        Target.Get(),
        BashRange,
        FOnMoveIntoRangeCompleted::CreateUObject(
            this,
            &UGA_Bash::OnMovementCompleted
        )
    );
}

void UGA_Bash::OnMovementCompleted(bool bSuccess)
{
    if (!bSuccess)
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

    // Target may have moved/died/become invalid while walking.
    if (!Target.IsValid() || !IsTargetInRange())
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

    PerformBash();
}

void UGA_Bash::PerformBash()
{
    AGenericCharacter* Character =
        GetGenericCharacter();

    AActor* TargetActor =
        Target.Get();

    if (!Character || !TargetActor)
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

    if (!CommitAbility(
        CurrentSpecHandle,
        CurrentActorInfo,
        CurrentActivationInfo))
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

    if (BashMontage)
    {
        Character->PlayAnimMontage(
            BashMontage
        );
    }

    ApplyBashDamage();

    EndAbility(
        CurrentSpecHandle,
        CurrentActorInfo,
        CurrentActivationInfo,
        true,
        false
    );
}

void UGA_Bash::ApplyBashDamage()
{
    AActor* TargetActor =
        Target.Get();

    if (!TargetActor || !DamageEffect)
    {
        return;
    }

    UAbilitySystemComponent* SourceASC =
        GetAbilitySystemComponentFromActorInfo();

    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::
        GetAbilitySystemComponent(
            TargetActor
        );

    if (!SourceASC || !TargetASC)
    {
        return;
    }

    FGameplayEffectContextHandle Context =
        SourceASC->MakeEffectContext();

    Context.AddSourceObject(this);

    FGameplayEffectSpecHandle Spec =
        SourceASC->MakeOutgoingSpec(
            DamageEffect,
            1.0f,
            Context
        );

    if (!Spec.IsValid())
    {
        return;
    }

    SourceASC->ApplyGameplayEffectSpecToTarget(
        *Spec.Data.Get(),
        TargetASC
    );
}