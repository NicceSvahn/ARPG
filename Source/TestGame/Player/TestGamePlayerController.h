// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "TestGamePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class TESTGAME_API ATestGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATestGamePlayerController();

protected:

    virtual void BeginPlay() override;

    virtual void SetupInputComponent() override;

private:

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* ClickMoveAction;

    void OnClickMove(const FInputActionValue& Value);
};