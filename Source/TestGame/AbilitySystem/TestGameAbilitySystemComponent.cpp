#include "TestGameAbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Pawn.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "AbilityRequestPolicy.h"
#include "../Player/TestGamePlayerController.h"

bool UTestGameAbilitySystemComponent::RequestAbility(
    const FGameplayTag& AbilityTag,
    const FAbilityInputContext& Context)
{
    if (IsOwnerDead() || !AbilityTag.IsValid())
    {
        return false;
    }

    FGameplayAbilitySpec* AbilitySpec =
        FindAbilitySpecForTag(AbilityTag);

    if (!AbilitySpec || !AbilitySpec->Ability)
    {
        return false;
    }

    const IAbilityRequestPolicy* RequestPolicy =
        Cast<IAbilityRequestPolicy>(AbilitySpec->Ability);

    const bool bSupportsForceDirectionalAttack =
        RequestPolicy &&
        RequestPolicy->SupportsForceDirectionalAttack();

    // Only abilities that explicitly opt into this behaviour may consume
    // the Shift modifier. Holding Shift must not silently change targeting
    // data for unrelated abilities such as Fireball.
    FAbilityInputContext EffectiveContext = Context;
    EffectiveContext.bForceDirectionalAttack =
        bSupportsForceDirectionalAttack &&
        Context.bForceDirectionalAttack;

    const bool bRequiresTarget =
        RequestPolicy &&
        RequestPolicy->RequiresTarget() &&
        !EffectiveContext.bForceDirectionalAttack;

    const float MaximumRange =
        EffectiveContext.bForceDirectionalAttack
        ? 0.0f
        : (RequestPolicy
            ? RequestPolicy->GetMaximumRange()
            : 0.0f);

    // Directional attacks need a real cursor/world hit to define the aim
    // point. If the cursor trace found nothing, do not activate.
    if (EffectiveContext.bForceDirectionalAttack &&
        !EffectiveContext.HitResult.bBlockingHit)
    {
        return false;
    }

    if (!CheckTarget(
        bRequiresTarget,
        EffectiveContext))
    {
        return false;
    }

    if (!CheckRange(
        MaximumRange,
        EffectiveContext))
    {
        return RequestMovement(
            AbilityTag,
            EffectiveContext,
            MaximumRange
        );
    }

    return TryActivateRequestedAbility(
        AbilitySpec->Handle,
        EffectiveContext
    );
}

void UTestGameAbilitySystemComponent::AbilityInputTagPressed(
    const FGameplayTag& InputTag)
{
    if (IsOwnerDead() || !InputTag.IsValid())
    {
        return;
    }

    FGameplayAbilitySpec* AbilitySpec =
        FindAbilitySpecForTag(InputTag);

    if (!AbilitySpec || !AbilitySpec->Ability)
    {
        return;
    }

    AbilitySpec->InputPressed = true;

    if (AbilitySpec->IsActive())
    {
        AbilitySpecInputPressed(
            *AbilitySpec
        );
    }
}

void UTestGameAbilitySystemComponent::AbilityInputTagReleased(
    const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid())
    {
        return;
    }

    FGameplayAbilitySpec* AbilitySpec =
        FindAbilitySpecForTag(InputTag);

    if (!AbilitySpec || !AbilitySpec->Ability)
    {
        return;
    }

    AbilitySpec->InputPressed = false;

    if (AbilitySpec->IsActive())
    {
        AbilitySpecInputReleased(
            *AbilitySpec
        );
    }
}

void UTestGameAbilitySystemComponent::AbilitySpecInputPressed(
    FGameplayAbilitySpec& Spec)
{
    Super::AbilitySpecInputPressed(Spec);

    if (!Spec.IsActive())
    {
        return;
    }

    const UGameplayAbility* Ability =
        Spec.GetPrimaryInstance();

    const FPredictionKey PredictionKey =
        Ability
        ? Ability
        ->GetCurrentActivationInfo()
        .GetActivationPredictionKey()
        : Spec
        .ActivationInfo
        .GetActivationPredictionKey();

    InvokeReplicatedEvent(
        EAbilityGenericReplicatedEvent::InputPressed,
        Spec.Handle,
        PredictionKey
    );
}


void UTestGameAbilitySystemComponent::AbilitySpecInputReleased(
    FGameplayAbilitySpec& Spec)
{
    Super::AbilitySpecInputReleased(Spec);

    if (!Spec.IsActive())
    {
        return;
    }

    const UGameplayAbility* Ability =
        Spec.GetPrimaryInstance();

    const FPredictionKey PredictionKey =
        Ability
        ? Ability
        ->GetCurrentActivationInfo()
        .GetActivationPredictionKey()
        : Spec
        .ActivationInfo
        .GetActivationPredictionKey();

    InvokeReplicatedEvent(
        EAbilityGenericReplicatedEvent::InputReleased,
        Spec.Handle,
        PredictionKey
    );
}

FGameplayAbilitySpec*
UTestGameAbilitySystemComponent::FindAbilitySpecForTag(
    const FGameplayTag& AbilityTag)
{
    return GetActivatableAbilities().FindByPredicate(
        [&AbilityTag](const FGameplayAbilitySpec& AbilitySpec)
        {
            return AbilitySpec
                .GetDynamicSpecSourceTags()
                .HasTagExact(AbilityTag);
        }
    );
}

bool UTestGameAbilitySystemComponent::CheckTarget(
    const bool bRequiresTarget,
    const FAbilityInputContext& Context) const
{
    if (!bRequiresTarget)
    {
        return true;
    }

    AActor* AvatarActorInstance = GetAvatarActor();

    return IsValid(Context.TargetActor) &&
        IsValid(AvatarActorInstance) &&
        Context.TargetActor != AvatarActorInstance &&
        UAbilitySystemBlueprintLibrary::
        GetAbilitySystemComponent(Context.TargetActor) != nullptr;
}

bool UTestGameAbilitySystemComponent::TryActivateRequestedAbility(
    const FGameplayAbilitySpecHandle& AbilityHandle,
    const FAbilityInputContext& Context)
{
    AbilityInputContext = Context;

    FGameplayAbilitySpec* Spec =
        FindAbilitySpecFromHandle(AbilityHandle);

    if (!Spec || !Spec->Ability)
    {
        return false;
    }

    FGameplayTagContainer FailureTags;

    if (!Spec->Ability->CanActivateAbility(
        AbilityHandle,
        AbilityActorInfo.Get(),
        nullptr,
        nullptr,
        &FailureTags
    ))
    {
        return false;
    }

    // A forced directional melee attack is intentionally stationary.
    // Stop both click movement and any pending chase only after we know the
    // ability is actually allowed to activate.
    if (Context.bForceDirectionalAttack)
    {
        APawn* AvatarPawn = Cast<APawn>(GetAvatarActor());

        ATestGamePlayerController* PlayerController =
            AvatarPawn
            ? Cast<ATestGamePlayerController>(
                AvatarPawn->GetController())
            : nullptr;

        if (PlayerController)
        {
            PlayerController->StopMovementForAbility();
        }
    }

    return TryActivateAbility(AbilityHandle);
}

bool UTestGameAbilitySystemComponent::RequestMovement(
    const FGameplayTag& AbilityTag,
    const FAbilityInputContext& Context,
    const float MaximumRange)
{
    APawn* AvatarPawn = Cast<APawn>(GetAvatarActor());
    ATestGamePlayerController* PlayerController =
        AvatarPawn
        ? Cast<ATestGamePlayerController>(AvatarPawn->GetController())
        : nullptr;

    if (!PlayerController || !IsValid(Context.TargetActor))
    {
        // AI-specific positioning remains the responsibility of its controller.
        return false;
    }

    // Cancelling first lets any previous callback clear its own request before
    // this request becomes the new pending one.
    PlayerController->CancelMoveIntoRange();

    PendingAbilityTag = AbilityTag;
    PendingAbilityContext = Context;

    PlayerController->MoveIntoRange(
        Context.TargetActor,
        MaximumRange,
        FOnMoveIntoRangeCompleted::CreateUObject(
            this,
            &UTestGameAbilitySystemComponent::OnRequestMovementCompleted
        )
    );

    return true;
}

void UTestGameAbilitySystemComponent::OnRequestMovementCompleted(
    const bool bSuccess)
{
    const FGameplayTag AbilityTag = PendingAbilityTag;
    const FAbilityInputContext Context = PendingAbilityContext;

    PendingAbilityTag = FGameplayTag();
    PendingAbilityContext = FAbilityInputContext();

    if (bSuccess)
    {
        RequestAbility(AbilityTag, Context);
    }
}

bool UTestGameAbilitySystemComponent::CheckRange(
    const float MaximumRange,
    const FAbilityInputContext& Context) const
{
    if (MaximumRange <= 0.0f)
    {
        return true;
    }

    const AActor* AvatarActorInstance = GetAvatarActor();

    if (!IsValid(AvatarActorInstance) || !IsValid(Context.TargetActor))
    {
        return false;
    }

    return FVector::Dist2D(
        AvatarActorInstance->GetActorLocation(),
        Context.TargetActor->GetActorLocation()
    ) <= MaximumRange;
}

UGameplayAbility*
UTestGameAbilitySystemComponent::GetAbilityForInputTag(
    const FGameplayTag& InputTag) const
{
    if (!InputTag.IsValid())
    {
        return nullptr;
    }

    for (const FGameplayAbilitySpec& AbilitySpec :
        GetActivatableAbilities())
    {
        if (AbilitySpec
            .GetDynamicSpecSourceTags()
            .HasTagExact(InputTag))
        {
            return AbilitySpec.Ability;
        }
    }

    return nullptr;
}

void UTestGameAbilitySystemComponent::NotifyAbilityBarChanged()
{
    OnAbilityBarChanged.Broadcast();
}

void UTestGameAbilitySystemComponent::SendAbilityEvent(
    const FGameplayTag& EventTag,
    const FAbilityInputContext& Context)
{
    if (IsOwnerDead() || !EventTag.IsValid())
    {
        return;
    }

    AActor* CurrentAvatarActor = GetAvatarActor();

    if (!IsValid(CurrentAvatarActor))
    {
        return;
    }

    FGameplayEventData EventData;
    EventData.Instigator = GetAvatarActor();
    EventData.Target = Context.TargetActor;

    HandleGameplayEvent(
        EventTag,
        &EventData
    );
}

float UTestGameAbilitySystemComponent::GetRemainingCooldown(
    const FGameplayTag& CooldownTag) const
{
    if (!CooldownTag.IsValid())
    {
        return 0.0f;
    }

    FGameplayTagContainer Tags;
    Tags.AddTag(CooldownTag);

    const FGameplayEffectQuery Query =
        FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
            Tags
        );

    const TArray<float> Times =
        GetActiveEffectsTimeRemaining(Query);

    float LongestRemainingTime = 0.0f;

    for (const float TimeRemaining : Times)
    {
        LongestRemainingTime =
            FMath::Max(
                LongestRemainingTime,
                TimeRemaining
            );
    }

    return LongestRemainingTime;
}

bool UTestGameAbilitySystemComponent::
IsOwnerDead() const
{
    const FGameplayTag DeadTag =
        FGameplayTag::RequestGameplayTag(
            FName(TEXT("State.Dead"))
        );

    return HasMatchingGameplayTag(DeadTag);
}

void UTestGameAbilitySystemComponent::
HandleOwnerDeath()
{
    CancelAllAbilities();

    PendingAbilityTag = FGameplayTag();
    PendingAbilityContext = FAbilityInputContext();

    APawn* AvatarPawn = Cast<APawn>(GetAvatarActor());

    ATestGamePlayerController* PlayerController = nullptr;

    if (AvatarPawn)
    {
        AController* Controller =
            AvatarPawn->GetController();

        PlayerController =
            Cast<ATestGamePlayerController>(Controller);
    }

    if (PlayerController)
    {
        PlayerController->CancelMoveIntoRange();
    }
}

void UTestGameAbilitySystemComponent::BroadcastDamageResult(
    const FDamageResult& DamageResult)
{
    DamageResultDelegate.Broadcast(DamageResult);
}