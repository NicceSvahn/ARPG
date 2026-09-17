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
class UResourceAttributeSet;
class UGameplayEffect;
class UMovementSpeedAttributeSet;

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FOnHealthChanged,
	float,
	float
);

DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnDamageReceived,
	float
);

DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnHealingReceived,
	float
);

UCLASS()
class TESTGAME_API AGenericCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	UTestGameAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
	TObjectPtr<UTestGameAbilitySystemComponent> AbilitySystemComponent;

	FOnHealthChanged OnHealthChanged;

	float GetCurrentHealth() const;
	float GetMaxHealth() const;

	UFUNCTION()
	void OnRep_IsDead();

	UPROPERTY(
		ReplicatedUsing = OnRep_IsDead
	)
	bool bIsDead = false;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;

	UPROPERTY()
	TObjectPtr<UHealthAttributeSet> HealthAttributeSet;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Health"
	)
	float InitialHealth = 100.0f;

	UFUNCTION()
	virtual void HandleAttributeChanged(FGameplayAttribute Attribute, float Magnitude, float NewValue);

	void HandleMovementSpeedChanged(
		const FOnAttributeChangeData& Data
	);

	FDelegateHandle MovementSpeedChangedHandle;

	FOnDamageReceived OnDamageReceived;
	FOnHealingReceived OnHealingReceived;

	AGenericCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Death",
		meta = (ClampMin = "0.0", Units = "s")
	)
	float DeathCleanupDelay = 3.0f;

	UFUNCTION(
		BlueprintImplementableEvent,
		Category = "Death",
		meta = (DisplayName = "On Death")
	)
	void ReceiveDeath();

	virtual void OnDeathStarted();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<FGrantedAbility> StartupAbilities;

	void GrantStartupAbilities();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<UResourceAttributeSet> ResourceAttributeSet;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Resource"
	)
	TSubclassOf<UGameplayEffect> ResourceRegenerationEffect;

	UPROPERTY()
	TObjectPtr<UMovementSpeedAttributeSet> MovementSpeedAttributeSet;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Movement"
	)
	float InitialMovementSpeed = 600.0f;

private:	
	float PreviousHealth = 0.0f;

	void EnterDeathState();
	void ApplyDeathState();
	void ApplyResourceRegeneration();
};
