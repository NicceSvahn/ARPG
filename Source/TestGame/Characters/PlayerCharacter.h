// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericCharacter.h"
#include "TestGame/AbilitySystem/Attributes/HealthAttributeSet.h"
#include "PlayerCharacter.generated.h"

class UAbilitySystemComponent;
class UHealthAttributeSet;

UCLASS()
class TESTGAME_API APlayerCharacter : public AGenericCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

	UPROPERTY()
	TObjectPtr<UHealthAttributeSet> HealthAttributeSet;

	UPROPERTY(EditAnywhere)
	float InitialHealth = 100.0f;

	UFUNCTION()
	void HandleHealthChanged(float Magnitude, float NewHealth);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveToLocation(FVector& NewTarget);

private:

	FVector MovementTarget = FVector::ZeroVector;
	bool bHasMovementTarget = false;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float AcceptanceRadius = 50.f;
};
