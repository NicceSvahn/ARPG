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
    if (!AbilityTag.IsValid())
    {
        return false;
    }

    FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecForTag(AbilityTag);

    if (!AbilitySpec || !AbilitySpec->Ability)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("RequestAbility: no granted ability has tag %s"),
            *AbilityTag.ToString()
        );
        return false;
    }

    const IAbilityRequestPolicy* RequestPolicy =
        Cast<IAbilityRequestPolicy>(AbilitySpec->Ability);

    const bool bRequiresTarget =
        RequestPolicy && RequestPolicy->RequiresTarget();

    const float MaximumRange =
        RequestPolicy ? RequestPolicy->GetMaximumRange() : 0.0f;

    if (!CheckTarget(bRequiresTarget, Context))
    {
        return false;
    }

    if (!CheckRange(MaximumRange, Context))
    {
        return RequestMovement(
            AbilityTag,
            Context,
            MaximumRange
        );
    }

    return TryActivateRequestedAbility(AbilitySpec->Handle, Context);
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
    if (!EventTag.IsValid())
    {
        return;
    }

    AActor* CurrentAvatarActor = GetAvatarActor();

    if (!IsValid(CurrentAvatarActor))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("SendAbilityEvent: ASC has no valid avatar actor")
        );

        return;
    }

    FGameplayEventData EventData;
    EventData.Instigator = GetAvatarActor();
    EventData.Target = Context.TargetActor;
    //EventData.ContextHandle = MakeEffectContext();

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