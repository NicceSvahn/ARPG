// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GenericCharacter.generated.h"

class UAbilitySystemComponent;
class UAttributeComponent;
class UHealthComponent;

UCLASS()
class TESTGAME_API AGenericCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// Sets default values for this character's properties
	AGenericCharacter();


	UFUNCTION(BlueprintPure, Category = "Components")
	UHealthComponent* GetHealthComponent() const
	{
		return HealthComponent;
	}

	UFUNCTION(BlueprintPure, Category = "Components")
	UAttributeComponent* GetAttributeComponent() const
	{
		return AttributeComponent;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Components",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Components",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UAttributeComponent> AttributeComponent;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


};
