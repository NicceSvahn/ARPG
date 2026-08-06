#pragma once

#include "CoreMinimal.h"
#include "DamageTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DamageCalculationLibrary.generated.h"

class UAttributeComponent;

UCLASS()
class TESTGAME_API UDamageCalculationLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "Combat|Damage"
	)
	static FDamageResult CalculateDamage(
		const FDamageRequest& Request,
		const UAttributeComponent* SourceAttributes,
		const UAttributeComponent* TargetAttributes
	);

private:
	static float CalculatePhysicalMultiplier(
		float Armor,
		float ArmorPenetration
	);

	static float CalculateResistanceMultiplier(
		float Resistance,
		float ResistancePenetration
	);
};