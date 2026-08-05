// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnHealthChanged,
	float,
	NewHealth
);


DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FOnDeath
);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TESTGAME_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.f;


	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth;


	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;


	UPROPERTY(BlueprintAssignable)
	FOnDeath OnDeath;


	UFUNCTION(BlueprintCallable)
	void TakeDamage(float DamageAmount);


	UFUNCTION(BlueprintCallable)
	void Heal(float HealAmount);


	UFUNCTION(BlueprintPure)
	float GetHealthPercent() const;

private:

	bool bDead = false;
		
};
