// Fill out your copyright notice in the Description page of Project Settings.


#include "TestGamePlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "PlayerCharacter.h"

ATestGamePlayerController::ATestGamePlayerController()
{
    UE_LOG(LogTemp, Warning, TEXT("Constructor on PlayerController"));
}

void ATestGamePlayerController::BeginPlay()
{
    Super::BeginPlay();

    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    UE_LOG(LogTemp, Warning, TEXT("BeginPlay on PlayerController"));

    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);

            UE_LOG(LogTemp, Warning, TEXT("Added Player Mapping Context"));
        }
    }
}

void ATestGamePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput =
        Cast<UEnhancedInputComponent>(InputComponent))
    {
        EnhancedInput->BindAction(
            ClickMoveAction,
            ETriggerEvent::Triggered,
            this,
            &ATestGamePlayerController::OnClickMove
        );

        UE_LOG(LogTemp, Warning, TEXT("Click Move Bound"));
    }
}

void ATestGamePlayerController::OnClickMove(const FInputActionValue& Value)
{
    FHitResult Hit;

    if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
        {
            UAIBlueprintHelperLibrary::SimpleMoveToLocation(
                this,
                Hit.Location
            );
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ECC Visibility not found"));
        return;
    }
}