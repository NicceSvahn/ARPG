// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "TestGame/AbilitySystem/Attributes/HealthAttributeSet.h"
#include "GenericCharacter.generated.h"

class UAbilitySystemComponent;

UCLASS()
class TESTGAME_API AGenericCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UHealthAttributeSet> HealthAttributeSet;

	UPROPERTY(EditAnywhere)
	float InitialHealth = 100.0f;

	UFUNCTION()
	virtual void HandleHealthChanged(float Magnitude, float NewHealth);

	// Sets default values for this character's properties
	AGenericCharacter();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


};
