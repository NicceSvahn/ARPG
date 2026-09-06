#include "TestGamePlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemBlueprintLibrary.h"

#include "../Characters/GenericCharacter.h"

ATestGamePlayerController::ATestGamePlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATestGamePlayerController::BeginPlay()
{
    Super::BeginPlay();

    int32 priority = 0; // Low numerical priority means high priority

    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, priority);
            }
        }
    }
}

void ATestGamePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);

    if (!EnhancedInput)
    {
        UE_LOG(LogTemp, Error, TEXT("EnhancedInputComponent not found"));
        return;
    }

    if (ClickMoveAction)
    {
        EnhancedInput->BindAction(
            ClickMoveAction,
            ETriggerEvent::Triggered,
            this,
            &ATestGamePlayerController::OnClickMove
        );
    }

    if (BashAction)
    {
        EnhancedInput->BindAction(
            BashAction,
            ETriggerEvent::Started,
            this,
            &ATestGamePlayerController::OnBashPressed
        );
    }

    if (FireballAction)
    {
        UE_LOG(LogTemp, Warning, TEXT("Binding FireballAction"));

        EnhancedInput->BindAction(
            FireballAction,
            ETriggerEvent::Started,
            this,
            &ATestGamePlayerController::OnFireballPressed
        );
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FireballAction is not assigned"));
    }
}

void ATestGamePlayerController::OnClickMove()
{
    FHitResult HitResult;

    const bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

    if (!bHit)
    {
        return;
    }

    // Clicking somewhere else cancels any ability movement.
    CancelMoveIntoRange();

    UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitResult.ImpactPoint);
}

void ATestGamePlayerController::MoveIntoRange(AActor* Target, float DesiredRange, FOnMoveIntoRangeCompleted OnCompleted)
{
    if (!Target)
    {
        OnCompleted.ExecuteIfBound(false);
        return;
    }

    // Cancel an existing special movement request first.
    if (bIsMovingToTarget)
    {
        CancelMoveIntoRange();
    }

    MovementTarget = Target;
    MovementAcceptanceRadius = DesiredRange;
    MoveCompletedDelegate = OnCompleted;
    bIsMovingToTarget = true;

    UAIBlueprintHelperLibrary::SimpleMoveToActor(this, Target);
}

void ATestGamePlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsMovingToTarget)
    {
        return;
    }

    ACharacter* ControlledCharacter = GetCharacter();
    AActor* Target = MovementTarget.Get();

    if (!ControlledCharacter || !Target)
    {
        FinishMoveIntoRange(false);
        return;
    }

    const float Distance = FVector::Dist2D(ControlledCharacter->GetActorLocation(), Target->GetActorLocation());

    if (Distance <= MovementAcceptanceRadius)
    {
        FinishMoveIntoRange(true);
    }
}

void ATestGamePlayerController::FinishMoveIntoRange(bool bSuccess)
{
    if (!bIsMovingToTarget)
    {
        return;
    }

    bIsMovingToTarget = false;

    StopMovement();

    MovementTarget.Reset();
    MovementAcceptanceRadius = 0.0f; // Set zero when reached "into range of target"

    FOnMoveIntoRangeCompleted CompletedDelegate = MoveCompletedDelegate;

    MoveCompletedDelegate.Unbind();

    CompletedDelegate.ExecuteIfBound(bSuccess);
}

void ATestGamePlayerController::CancelMoveIntoRange()
{
    if (!bIsMovingToTarget)
    {
        return;
    }

    FinishMoveIntoRange(false);
}

void ATestGamePlayerController::OnBashPressed()
{
    FHitResult HitResult;

    if (!GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
    {
        return;
    }

    AActor* TargetActor = HitResult.GetActor();

    if (!TargetActor)
    {
        return;
    }

    AGenericCharacter* ControlledCharacter = Cast<AGenericCharacter>(GetPawn());

    if (!ControlledCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("BASH: No GenericCharacter pawn"));
        return;
    }

    UAbilitySystemComponent* ASC = ControlledCharacter->GetAbilitySystemComponent();

    if (!ASC)
    {
        UE_LOG(LogTemp, Error, TEXT("BASH: No AbilitySystemComponent"));
        return;
    }

    FGameplayEventData EventData;
    EventData.Instigator = ControlledCharacter;
    EventData.Target = TargetActor;

    const FGameplayTag BashEventTag = FGameplayTag::RequestGameplayTag(FName("Event.Ability.Bash"));

    const int32 ActivatedAbilities = ASC->HandleGameplayEvent(BashEventTag, &EventData);

    UE_LOG(LogTemp, Warning, TEXT("BASH: Gameplay event sent. Activated abilities: %d"), ActivatedAbilities);
}

void ATestGamePlayerController::OnFireballPressed()
{
    UE_LOG(LogTemp, Warning, TEXT("FIREBALL INPUT PRESSED"));

    FHitResult HitResult;

    if (!GetHitResultUnderCursor(
        ECC_Visibility,
        false,
        HitResult))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("FIREBALL: Nothing found under cursor"));

        return;
    }

    AActor* TargetActor = HitResult.GetActor();

    if (!IsValid(TargetActor))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("FIREBALL: No target actor"));

        return;
    }

    AGenericCharacter* ControlledCharacter =
        Cast<AGenericCharacter>(GetPawn());

    if (!ControlledCharacter)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("FIREBALL: No GenericCharacter pawn"));

        return;
    }

    UAbilitySystemComponent* ASC =
        ControlledCharacter->GetAbilitySystemComponent();

    if (!ASC)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("FIREBALL: No AbilitySystemComponent"));

        return;
    }

    const FGameplayTag FireballEventTag =
        FGameplayTag::RequestGameplayTag(
            TEXT("Event.Ability.Fireball"));

    FGameplayEventData EventData;
    EventData.EventTag = FireballEventTag;
    EventData.Instigator = ControlledCharacter;
    EventData.Target = TargetActor;

    // Store the mouse cursor's hit position in the event.
    EventData.TargetData =
        UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(
            HitResult);

    const int32 ActivatedAbilities =
        ASC->HandleGameplayEvent(
            FireballEventTag,
            &EventData);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("FIREBALL: Event sent. Activated abilities: %d"),
        ActivatedAbilities);
}