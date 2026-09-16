#include "TestGamePlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "../UI/PlayerHudWidget.h"

#include "../AbilitySystem/AbilityInputContext.h"
#include "../AbilitySystem/TestGameAbilitySystemComponent.h"
#include "../Characters/GenericCharacter.h"
#include "../UI/PlayerHudWidget.h"
#include "../UI/CombatDebugWidget.h"
#include "../Camera/CameraOccludableComponent.h"

ATestGamePlayerController::ATestGamePlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATestGamePlayerController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PlayerController BeginPlay: %s"),
        *GetNameSafe(this));

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("IsLocalController: %s"),
        IsLocalController() ? TEXT("true") : TEXT("false"));

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PlayerHudWidgetClass: %s"),
        *GetNameSafe(PlayerHudWidgetClass));

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

    if (IsLocalController() && PlayerHudWidgetClass)
    {
        PlayerHudWidget =
                CreateWidget<UPlayerHudWidget>(
                this,
                PlayerHudWidgetClass);

        if (PlayerHudWidget)
        {
            PlayerHudWidget->AddToViewport();
            UE_LOG(LogTemp, Error, TEXT("Added to viewport?"));
        }
    }

    TryInitializeHud();
}

void ATestGamePlayerController::OnPossess(
    APawn* InPawn)
{
    Super::OnPossess(InPawn);

    TryInitializeHud();
}

void ATestGamePlayerController::OnRep_Pawn()
{
    Super::OnRep_Pawn();

    TryInitializeHud();
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

    if (ToggleCombatDebugAction)
    {
        EnhancedInput->BindAction(
            ToggleCombatDebugAction,
            ETriggerEvent::Started,
            this,
            &ATestGamePlayerController::ToggleCombatDebug
        );
    }
}

void ATestGamePlayerController::MoveIntoRange(
    AActor* Target,
    float DesiredRange,
    FOnMoveIntoRangeCompleted OnCompleted)
{
    if (!Target || !IsLocalController())
    {
        OnCompleted.ExecuteIfBound(false);
        return;
    }

    if (bIsMovingToTarget)
    {
        CancelMoveIntoRange();
    }

    StopClickMove();

    MovementTarget = Target;
    MovementAcceptanceRadius = DesiredRange;
    MoveCompletedDelegate = OnCompleted;
    bIsMovingToTarget = true;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[MOVE INTO RANGE] Start | Pawn=%s | Target=%s | Range=%.1f"
        ),
        *GetNameSafe(GetPawn()),
        *GetNameSafe(Target),
        DesiredRange
    );
}

void ATestGamePlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCameraOcclusion();

    UpdateClickMove();

    if (!bIsMovingToTarget ||
        !IsLocalController())
    {
        return;
    }

    ACharacter* ControlledCharacter =
        GetCharacter();

    AActor* Target =
        MovementTarget.Get();

    if (!ControlledCharacter ||
        !Target)
    {
        FinishMoveIntoRange(false);
        return;
    }

    FVector ToTarget =
        Target->GetActorLocation() -
        ControlledCharacter->GetActorLocation();

    ToTarget.Z = 0.0f;

    const float Distance =
        ToTarget.Size();

    if (Distance <= MovementAcceptanceRadius)
    {
        FinishMoveIntoRange(true);
        return;
    }

    const FVector MoveDirection =
        ToTarget.GetSafeNormal();

    ControlledCharacter->AddMovementInput(
        MoveDirection,
        1.0f
    );
}

void ATestGamePlayerController::FinishMoveIntoRange(
    bool bSuccess)
{
    if (!bIsMovingToTarget)
    {
        return;
    }

    bIsMovingToTarget = false;

    MovementTarget.Reset();
    MovementAcceptanceRadius = 0.0f;

    FOnMoveIntoRangeCompleted CompletedDelegate =
        MoveCompletedDelegate;

    MoveCompletedDelegate.Unbind();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[MOVE INTO RANGE] Finished | Pawn=%s | Success=%s"
        ),
        *GetNameSafe(GetPawn()),
        bSuccess ? TEXT("TRUE") : TEXT("FALSE")
    );

    CompletedDelegate.ExecuteIfBound(
        bSuccess
    );
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

    ASC->RequestAbility(InputTag, Context);
}

void ATestGamePlayerController::TryInitializeHud()
{
    if (!IsLocalController())
    {
        return;
    }

    AGenericCharacter* PlayerCharacter =
        Cast<AGenericCharacter>(GetPawn());

    if (!PlayerCharacter)
    {
        return;
    }

    UTestGameAbilitySystemComponent* ASC =
        PlayerCharacter->GetAbilitySystemComponent();

    if (!ASC)
    {
        return;
    }

    if (!PlayerHudWidget)
    {
        if (!PlayerHudWidgetClass)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT("PlayerHudClass is not set")
            );

            return;
        }

        PlayerHudWidget =
            CreateWidget<UPlayerHudWidget>(
                this,
                PlayerHudWidgetClass
            );

        if (!PlayerHudWidget)
        {
            return;
        }

        PlayerHudWidget->AddToViewport();
    }

    PlayerHudWidget->InitializeHud(PlayerCharacter);
}

void ATestGamePlayerController::ToggleCombatDebug()
{
    if (!IsLocalController())
    {
        return;
    }

    if (!CombatDebugWidget)
    {
        if (!CombatDebugWidgetClass)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("CombatDebugWidgetClass is not set")
            );

            return;
        }

        AGenericCharacter* PlayerCharacter =
            Cast<AGenericCharacter>(GetPawn());

        if (!PlayerCharacter)
        {
            return;
        }

        CombatDebugWidget =
            CreateWidget<UCombatDebugWidget>(
                this,
                CombatDebugWidgetClass
            );

        if (!CombatDebugWidget)
        {
            return;
        }

        CombatDebugWidget->InitializeDebugWidget(
            PlayerCharacter
        );

        CombatDebugWidget->AddToViewport();

        bCombatDebugVisible = true;
        return;
    }

    bCombatDebugVisible = !bCombatDebugVisible;

    CombatDebugWidget->SetVisibility(
        bCombatDebugVisible
        ? ESlateVisibility::HitTestInvisible
        : ESlateVisibility::Collapsed
    );
}

AGenericCharacter*
ATestGamePlayerController::GetCharacterUnderCursor() const
{
    FHitResult HitResult;

    if (!GetHitResultUnderCursor(
        ECC_Visibility,
        false,
        HitResult))
    {
        return nullptr;
    }

    return Cast<AGenericCharacter>(
        HitResult.GetActor()
    );
}

void ATestGamePlayerController::
UpdateCameraOcclusion()
{
    AGenericCharacter* ControlledCharacter =
        Cast<AGenericCharacter>(GetPawn());

    if (!ControlledCharacter)
    {
        return;
    }

    FVector CameraLocation;
    FRotator CameraRotation;

    GetPlayerViewPoint(
        CameraLocation,
        CameraRotation
    );

    const FVector PlayerLocation =
        ControlledCharacter->GetActorLocation();

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(
        ControlledCharacter
    );

    TArray<FHitResult> Hits;

    const FCollisionShape Sphere =
        FCollisionShape::MakeSphere(50.0f);

    GetWorld()->SweepMultiByChannel(
        Hits,
        CameraLocation,
        PlayerLocation,
        FQuat::Identity,
        ECC_Visibility,
        Sphere,
        QueryParams
    );

    TSet<
        TWeakObjectPtr<UCameraOccludableComponent>>
        CurrentlyOccluded;

    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();

        if (!HitActor)
        {
            continue;
        }

        UCameraOccludableComponent*
            OccludableComponent =
            HitActor->FindComponentByClass<
            UCameraOccludableComponent>();

        if (!OccludableComponent)
        {
            continue;
        }

        CurrentlyOccluded.Add(
            OccludableComponent
        );

        OccludableComponent->SetOccluded(
            true
        );
    }

    for (
        const TWeakObjectPtr<
        UCameraOccludableComponent>&
        PreviousComponent
        : OccludedComponents)
    {
        if (!PreviousComponent.IsValid())
        {
            continue;
        }

        if (!CurrentlyOccluded.Contains(
            PreviousComponent))
        {
            PreviousComponent
                ->SetOccluded(false);
        }
    }

    OccludedComponents =
        MoveTemp(CurrentlyOccluded);
}

void ATestGamePlayerController::OnClickMove()
{
    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[MOVE INPUT] Controller=%s Local=%s Authority=%s Pawn=%s"
        ),
        *GetName(),
        IsLocalController() ? TEXT("TRUE") : TEXT("FALSE"),
        HasAuthority() ? TEXT("TRUE") : TEXT("FALSE"),
        *GetNameSafe(GetPawn())
    );

    FHitResult HitResult;

    const bool bHit = GetHitResultUnderCursor(
        ECC_MovementGround,
        false,
        HitResult
    );

    if (!bHit || !HitResult.bBlockingHit)
    {
        return;
    }

    CancelMoveIntoRange();

    StartClickMove(
        HitResult.ImpactPoint
    );
}

void ATestGamePlayerController::StartClickMove(
    const FVector& Destination)
{
    if (!IsLocalController())
    {
        return;
    }

    ClickMoveDestination = Destination;
    bIsClickMoving = true;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[CLICK MOVE] Start | Pawn=%s | Destination=%s"
        ),
        *GetNameSafe(GetPawn()),
        *ClickMoveDestination.ToString()
    );
}

void ATestGamePlayerController::UpdateClickMove()
{
    if (!bIsClickMoving || !IsLocalController())
    {
        return;
    }

    ACharacter* ControlledCharacter = GetCharacter();

    if (!ControlledCharacter)
    {
        StopClickMove();
        return;
    }

    FVector ToDestination =
        ClickMoveDestination -
        ControlledCharacter->GetActorLocation();

    ToDestination.Z = 0.0f;

    const float Distance =
        ToDestination.Size();

    if (Distance <= ClickMoveAcceptanceRadius)
    {
        StopClickMove();
        return;
    }

    const FVector MoveDirection =
        ToDestination.GetSafeNormal();

    ControlledCharacter->AddMovementInput(
        MoveDirection,
        1.0f
    );
}

void ATestGamePlayerController::StopClickMove()
{
    bIsClickMoving = false;
    ClickMoveDestination = FVector::ZeroVector;
}