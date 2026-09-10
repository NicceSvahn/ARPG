// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"

#include "../AbilitySystem/Attributes/HealthAttributeSet.h"
#include "../AbilitySystem/TestGameAbilitySystemComponent.h"
#include "../AbilitySystem/GrantedAbility.h"

#include "GenericCharacter.generated.h"

class UTestGameAbilitySystemComponent;
class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilitiesGranted);

UCLASS()
class TESTGAME_API AGenericCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	UTestGameAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
	TObjectPtr<UTestGameAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UHealthAttributeSet> HealthAttributeSet;

	UPROPERTY(EditAnywhere)
	float InitialHealth = 100.0f;

	UFUNCTION()
	virtual void HandleAttributeChanged(FGameplayAttribute Attribute, float Magnitude, float NewValue);

	UPROPERTY(BlueprintAssignable, Category = "Abilities")
	FOnAbilitiesGranted OnAbilitiesGranted;

	AGenericCharacter();


protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<FGrantedAbility> StartupAbilities;

	void GrantStartupAbilities();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


};
