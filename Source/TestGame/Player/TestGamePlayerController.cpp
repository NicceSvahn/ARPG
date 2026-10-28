#include "TestGamePlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"

#include "../UI/PlayerHudWidget.h"
#include "../AbilitySystem/TestGameAbilitySystemComponent.h"
#include "../Characters/GenericCharacter.h"
#include "../UI/CombatDebugWidget.h"
#include "../UI/CharacterPanelWidget.h"
#include "../Camera/CameraOccludableComponent.h"
#include "../Game/TestGamePlayerState.h"


ATestGamePlayerController::ATestGamePlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}


void ATestGamePlayerController::BeginPlay()
{
    Super::BeginPlay();

    constexpr int32 MappingPriority = 0;

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
                Subsystem->AddMappingContext(
                    DefaultMappingContext,
                    MappingPriority
                );
            }
        }
    }

    TryInitializeHud();
    TryInitializeCharacterPanel();
}


void ATestGamePlayerController::OnPossess(
    APawn* InPawn)
{
    Super::OnPossess(InPawn);

    TryInitializeHud();
    TryInitializeCharacterPanel();
}


void ATestGamePlayerController::OnRep_Pawn()
{
    Super::OnRep_Pawn();

    TryInitializeHud();
    TryInitializeCharacterPanel();
}


void ATestGamePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInput =
        Cast<UEnhancedInputComponent>(
            InputComponent
        );

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

    for (const FAbilityInputBinding& Binding :
        AbilityInputBindings)
    {
        if (
            !Binding.InputAction ||
            !Binding.InputTag.IsValid()
            )
        {
            continue;
        }

        EnhancedInput->BindAction(
            Binding.InputAction,
            ETriggerEvent::Started,
            this,
            &ATestGamePlayerController::
            OnAbilityInputPressed,
            Binding.InputTag
        );

        EnhancedInput->BindAction(
            Binding.InputAction,
            ETriggerEvent::Completed,
            this,
            &ATestGamePlayerController::
            OnAbilityInputReleased,
            Binding.InputTag
        );

        EnhancedInput->BindAction(
            Binding.InputAction,
            ETriggerEvent::Canceled,
            this,
            &ATestGamePlayerController::
            OnAbilityInputReleased,
            Binding.InputTag
        );
    }

    if (ToggleCombatDebugAction)
    {
        EnhancedInput->BindAction(
            ToggleCombatDebugAction,
            ETriggerEvent::Started,
            this,
            &ATestGamePlayerController::
            ToggleCombatDebug
        );
    }

    if (IA_ToggleCharacterPanel)
    {
        EnhancedInput->BindAction(
            IA_ToggleCharacterPanel,
            ETriggerEvent::Started,
            this,
            &ATestGamePlayerController::
            ToggleCharacterPanel
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
}


void ATestGamePlayerController::Tick(
    float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateCameraOcclusion();

    UpdateClickMove();

    if (
        !bIsMovingToTarget ||
        !IsLocalController()
        )
    {
        return;
    }

    ACharacter* ControlledCharacter =
        GetCharacter();

    AActor* Target =
        MovementTarget.Get();

    if (
        !ControlledCharacter ||
        !Target
        )
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

void ATestGamePlayerController::StopMovementForAbility()
{
    CancelMoveIntoRange();
    StopClickMove();

    ACharacter* ControlledCharacter =
        GetCharacter();

    if (!ControlledCharacter)
    {
        return;
    }

    if (UCharacterMovementComponent* MovementComponent =
        ControlledCharacter->GetCharacterMovement())
    {
        MovementComponent->StopMovementImmediately();
    }
}

void ATestGamePlayerController::OnAbilityInputPressed(
    FGameplayTag InputTag)
{
    AGenericCharacter* ControlledCharacter =
        Cast<AGenericCharacter>(
            GetPawn()
        );

    if (!ControlledCharacter)
    {
        return;
    }

    UTestGameAbilitySystemComponent* ASC =
        ControlledCharacter->
        GetAbilitySystemComponent();

    if (!ASC)
    {
        return;
    }

    FAbilityInputContext Context;

    // Capture the modifier at the instant the ability is pressed.
    // Both Shift keys behave identically. The ASC will only honour this
    // flag for abilities that explicitly support directional attacks.
    Context.bForceDirectionalAttack =
        IsInputKeyDown(EKeys::LeftShift) ||
        IsInputKeyDown(EKeys::RightShift);

    FHitResult HitResult;

    if (GetHitResultUnderCursor(
        ECC_Visibility,
        false,
        HitResult))
    {
        Context.HitResult =
            HitResult;

        Context.TargetActor =
            HitResult.GetActor();

        Context.HitLocation =
            HitResult.ImpactPoint;
    }

    ASC->AbilityInputTagPressed(
        InputTag
    );

    ASC->RequestAbility(
        InputTag,
        Context
    );
}

void ATestGamePlayerController::OnAbilityInputReleased(
    FGameplayTag InputTag)
{
    AGenericCharacter* ControlledCharacter =
        Cast<AGenericCharacter>(GetPawn());

    if (!ControlledCharacter)
    {
        return;
    }

    UTestGameAbilitySystemComponent* ASC =
        ControlledCharacter->GetAbilitySystemComponent();

    if (!ASC)
    {
        return;
    }

    ASC->AbilityInputTagReleased(
        InputTag
    );
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

    if (!PlayerHudWidget)
    {
        if (!PlayerHudWidgetClass)
        {
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

    PlayerHudWidget->InitializeFromActor(PlayerCharacter);
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
            return;
        }

        AGenericCharacter* PlayerCharacter =
            Cast<AGenericCharacter>(
                GetPawn()
            );

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

        CombatDebugWidget->
            InitializeDebugWidget(
                PlayerCharacter
            );

        CombatDebugWidget->AddToViewport();

        bCombatDebugVisible = true;
        return;
    }

    bCombatDebugVisible =
        !bCombatDebugVisible;

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

    return Cast<AGenericCharacter>(HitResult.GetActor());
}

void ATestGamePlayerController::
UpdateCameraOcclusion()
{
    AGenericCharacter* ControlledCharacter =
        Cast<AGenericCharacter>(
            GetPawn()
        );

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
        ControlledCharacter->
        GetActorLocation();

    FCollisionQueryParams QueryParams;

    QueryParams.AddIgnoredActor(
        ControlledCharacter
    );

    TArray<FHitResult> Hits;

    const FCollisionShape Sphere =
        FCollisionShape::MakeSphere(
            50.0f
        );

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
        TWeakObjectPtr<
        UCameraOccludableComponent
        >
    > CurrentlyOccluded;

    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor =
            Hit.GetActor();

        if (!HitActor)
        {
            continue;
        }

        UCameraOccludableComponent*
            OccludableComponent =
            HitActor->
            FindComponentByClass<
            UCameraOccludableComponent
            >();

        if (!OccludableComponent)
        {
            continue;
        }

        CurrentlyOccluded.Add(
            OccludableComponent
        );

        OccludableComponent->
            SetOccluded(true);
    }

    for (
        const TWeakObjectPtr<
        UCameraOccludableComponent
        >& PreviousComponent
        : OccludedComponents)
    {
        if (!PreviousComponent.IsValid())
        {
            continue;
        }

        if (!CurrentlyOccluded.Contains(
            PreviousComponent))
        {
            PreviousComponent->
                SetOccluded(false);
        }
    }

    OccludedComponents =
        MoveTemp(CurrentlyOccluded);
}

void ATestGamePlayerController::OnClickMove()
{
    FHitResult HitResult;

    const bool bHit =
        GetHitResultUnderCursor(
            ECC_MovementGround,
            false,
            HitResult
        );

    if (
        !bHit ||
        !HitResult.bBlockingHit
        )
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

    ClickMoveDestination =
        Destination;

    bIsClickMoving = true;
}


void ATestGamePlayerController::UpdateClickMove()
{
    if (
        !bIsClickMoving ||
        !IsLocalController()
        )
    {
        return;
    }

    ACharacter* ControlledCharacter =
        GetCharacter();

    if (!ControlledCharacter)
    {
        StopClickMove();
        return;
    }

    FVector ToDestination =
        ClickMoveDestination -
        ControlledCharacter->
        GetActorLocation();

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

    ClickMoveDestination =
        FVector::ZeroVector;
}

void ATestGamePlayerController::RequestPlayerClass(
    EPlayerClass NewClass)
{
    if (HasAuthority())
    {
        ATestGamePlayerState* TestPlayerState =
            GetPlayerState<
            ATestGamePlayerState
            >();

        if (!IsValid(TestPlayerState))
        {
            return;
        }

        TestPlayerState->
            SetPlayerClass(
                NewClass
            );

        return;
    }

    ServerRequestPlayerClass(
        NewClass
    );
}


void ATestGamePlayerController::
ServerRequestPlayerClass_Implementation(
    EPlayerClass NewClass)
{
    ATestGamePlayerState* TestPlayerState =
        GetPlayerState<
        ATestGamePlayerState
        >();

    if (!IsValid(TestPlayerState))
    {
        return;
    }

    TestPlayerState->
        SetPlayerClass(
            NewClass
        );
}

void ATestGamePlayerController::
TryInitializeCharacterPanel()
{
    if (!IsLocalController() || !CharacterPanelClass)
    {
        return;
    }

    APawn* PlayerPawn = GetPawn();

    if (!PlayerPawn)
    {
        return;
    }

    if (!CharacterPanelWidget)
    {
        CharacterPanelWidget =
            CreateWidget<UCharacterPanelWidget>(
                this,
                CharacterPanelClass
            );

        if (!CharacterPanelWidget)
        {
            return;
        }

        CharacterPanelWidget->AddToViewport();
        CharacterPanelWidget->SetVisibility(
            ESlateVisibility::Collapsed
        );
    }

    CharacterPanelWidget->InitializeFromActor(PlayerPawn);
}


void ATestGamePlayerController::
ToggleCharacterPanel()
{
    if (!CharacterPanelWidget)
    {
        return;
    }

    const bool bIsVisible =
        CharacterPanelWidget->
        GetVisibility()
        != ESlateVisibility::Collapsed;

    CharacterPanelWidget->
        SetVisibility(
            bIsVisible
            ? ESlateVisibility::Collapsed
            : ESlateVisibility::Visible
        );
}