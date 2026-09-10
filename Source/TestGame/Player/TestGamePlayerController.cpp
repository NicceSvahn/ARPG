#include "TestGamePlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

#include "../AbilitySystem/AbilityInputContext.h"
#include "../AbilitySystem/TestGameAbilitySystemComponent.h"
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

    // Ability activation
    for (const FAbilityInputBinding& Binding : AbilityInputBindings)
    {
        if (!Binding.InputAction || !Binding.InputTag.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("Setup 1"));
            continue;
        }
        UE_LOG(LogTemp, Warning, TEXT("INPUT: Binding %s -> %s"), *Binding.InputAction->GetName(), *Binding.InputTag.ToString());

        EnhancedInput->BindAction(
            Binding.InputAction,
            ETriggerEvent::Started,
            this,
            &ATestGamePlayerController::OnAbilityInputPressed,
            Binding.InputTag
        );
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

void ATestGamePlayerController::OnAbilityInputPressed(FGameplayTag InputTag)
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT("1 CONTROLLER: %s"),
        *InputTag.ToString()
    );

    AGenericCharacter* ControlledCharacter = Cast<AGenericCharacter>(GetPawn());

    if (!ControlledCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("NO CHARACTER"));
        return;
    }

    UTestGameAbilitySystemComponent* ASC = ControlledCharacter->GetAbilitySystemComponent();

    if (!ASC)
    {
        UE_LOG(LogTemp, Error, TEXT("NO ASC"));
        return;
    }

    FAbilityInputContext Context;
    FHitResult HitResult;

    if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
    {
        Context.HitResult = HitResult;
        Context.TargetActor = HitResult.GetActor();
        Context.HitLocation = HitResult.ImpactPoint;
    }

    ASC->AbilityInputTagPressed(InputTag, Context);
}