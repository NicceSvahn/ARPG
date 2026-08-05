// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
}


void UHealthComponent::TakeDamage(float DamageAmount)
{
	if (bDead)
	{
		return;
	}


	CurrentHealth -= DamageAmount;

	CurrentHealth = FMath::Clamp(
		CurrentHealth,
		0.f,
		MaxHealth
	);


	OnHealthChanged.Broadcast(CurrentHealth);


	if (CurrentHealth <= 0.f)
	{
		bDead = true;

		OnDeath.Broadcast();
	}
}



void UHealthComponent::Heal(float HealAmount)
{
	if (bDead)
	{
		return;
	}


	CurrentHealth += HealAmount;


	CurrentHealth = FMath::Clamp(
		CurrentHealth,
		0.f,
		MaxHealth
	);


	OnHealthChanged.Broadcast(CurrentHealth);
}



float UHealthComponent::GetHealthPercent() const
{
	return CurrentHealth / MaxHealth;
}