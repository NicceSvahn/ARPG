#include "GA_Bash.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include "../../Characters/GenericCharacter.h"
#include "../../Characters/EnemyCharacter.h"
#include "../../Player/TestGamePlayerController.h"
#include "../../AbilitySystem/TestGameAbilitySystemComponent.h"
#include "../../AbilitySystem/AbilityInputContext.h"


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

    UTestGameAbilitySystemComponent* ASC =
        Cast<UTestGameAbilitySystemComponent>(
            GetAbilitySystemComponentFromActorInfo()
        );

    if (!ASC)
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

    const FAbilityInputContext& Context =
        ASC->GetAbilityInputContext();

    CurrentTargetActor = Context.TargetActor;

    if (!IsValid(CurrentTargetActor))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("BASH: No valid target")
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

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("BASH: Target = %s"),
        *GetNameSafe(CurrentTargetActor)
    );

    if (IsTargetInRange())
    {
        PerformBash();
        return;
    }

    if (Cast<AEnemyCharacter>(GetGenericCharacter()))
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

    RequestMoveIntoRange();
}


bool UGA_Bash::IsTargetInRange() const
{
    const AActor* OwnerActor =
        GetAvatarActorFromActorInfo();

    if (!IsValid(OwnerActor) ||
        !IsValid(CurrentTargetActor))
    {
        return false;
    }

    const float Distance = FVector::Dist2D(
        OwnerActor->GetActorLocation(),
        CurrentTargetActor->GetActorLocation()
    );

    return Distance <= BashRange;
}


void UGA_Bash::RequestMoveIntoRange()
{
    AGenericCharacter* Character =
        GetGenericCharacter();

    if (!Character ||
        !IsValid(CurrentTargetActor))
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
            TEXT("BASH: Avatar is not controlled by TestGamePlayerController")
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
        CurrentTargetActor,
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

    if (!IsValid(CurrentTargetActor) ||
        !IsTargetInRange())
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

    if (!Character ||
        !IsValid(CurrentTargetActor))
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
        CurrentTargetActor->GetActorLocation() -
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
    if (!IsValid(CurrentTargetActor) ||
        !DamageEffect)
    {
        return;
    }

    UAbilitySystemComponent* SourceASC =
        GetAbilitySystemComponentFromActorInfo();

    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::
        GetAbilitySystemComponent(
            CurrentTargetActor
        );

    if (!SourceASC ||
        !TargetASC)
    {
        return;
    }

    FGameplayEffectContextHandle EffectContext =
        SourceASC->MakeEffectContext();

    EffectContext.AddSourceObject(this);

    FGameplayEffectSpecHandle Spec =
        SourceASC->MakeOutgoingSpec(
            DamageEffect,
            1.0f,
            EffectContext
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