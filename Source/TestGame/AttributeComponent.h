#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

UCLASS(ClassGroup = (ARPG), meta = (BlueprintSpawnableComponent))
class TESTGAME_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();

	UFUNCTION(BlueprintPure, Category = "Attributes|Offense")
	float GetStrength() const { return Strength; }

	UFUNCTION(BlueprintPure, Category = "Attributes|Offense")
	float GetCriticalChance() const { return CriticalChance; }

	UFUNCTION(BlueprintPure, Category = "Attributes|Offense")
	float GetCriticalDamageMultiplier() const
	{
		return CriticalDamageMultiplier;
	}

	UFUNCTION(BlueprintPure, Category = "Attributes|Defense")
	float GetArmor() const { return Armor; }

	UFUNCTION(BlueprintPure, Category = "Attributes|Defense")
	float GetFireResistance() const { return FireResistance; }

	UFUNCTION(BlueprintPure, Category = "Attributes|Defense")
	float GetColdResistance() const { return ColdResistance; }

protected:
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Attributes|Primary",
		meta = (ClampMin = "0.0")
	)
	float Strength = 0.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Attributes|Offense",
		meta = (ClampMin = "0.0", ClampMax = "1.0")
	)
	float CriticalChance = 0.05f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Attributes|Offense",
		meta = (ClampMin = "1.0")
	)
	float CriticalDamageMultiplier = 2.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Attributes|Defense",
		meta = (ClampMin = "0.0")
	)
	float Armor = 0.0f;

	/*
	 * Resistances are stored as fractions:
	 * 0.25 means 25% resistance.
	 */
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Attributes|Defense",
		meta = (ClampMin = "0.0", ClampMax = "0.95")
	)
	float FireResistance = 0.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Attributes|Defense",
		meta = (ClampMin = "0.0", ClampMax = "0.95")
	)
	float ColdResistance = 0.0f;
};