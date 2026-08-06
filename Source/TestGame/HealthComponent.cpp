#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	bDead = false;
}

float UHealthComponent::TakeDamage(float FinalDamage)
{
	if (bDead || FinalDamage <= 0.0f)
	{
		return 0.0f;
	}

	const float PreviousHealth = CurrentHealth;

	CurrentHealth = FMath::Clamp(
		CurrentHealth - FinalDamage,
		0.0f,
		MaxHealth
	);

	const float AppliedDamage =
		PreviousHealth - CurrentHealth;

	OnHealthChanged.Broadcast(
		CurrentHealth,
		MaxHealth,
		-AppliedDamage
	);

	if (CurrentHealth <= 0.0f && !bDead)
	{
		bDead = true;
		OnDeath.Broadcast(GetOwner());
	}

	return AppliedDamage;
}

void UHealthComponent::Heal(float HealAmount)
{
	if (bDead || HealAmount <= 0.0f)
	{
		return;
	}

	const float PreviousHealth = CurrentHealth;

	CurrentHealth = FMath::Clamp(
		CurrentHealth + HealAmount,
		0.0f,
		MaxHealth
	);

	const float AppliedHealing =
		CurrentHealth - PreviousHealth;

	OnHealthChanged.Broadcast(
		CurrentHealth,
		MaxHealth,
		AppliedHealing
	);
}

float UHealthComponent::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return CurrentHealth / MaxHealth;
}