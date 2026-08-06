#include "DamageCalculationLibrary.h"

#include "AttributeComponent.h"

FDamageResult UDamageCalculationLibrary::CalculateDamage(
	const FDamageRequest& Request,
	const UAttributeComponent* SourceAttributes,
	const UAttributeComponent* TargetAttributes
)
{
	FDamageResult Result;

	if (Request.BaseDamage <= 0.0f)
	{
		return Result;
	}

	Result.RawDamage = Request.BaseDamage;

	float WorkingDamage = Request.BaseDamage;

	/*
	 * Temporary physical scaling:
	 * Every point of Strength adds 1% of base damage.
	 *
	 * Example:
	 * BaseDamage = 30
	 * Strength = 10
	 * WorkingDamage = 33
	 */
	if (SourceAttributes)
	{
		const float StrengthMultiplier =
			1.0f + SourceAttributes->GetStrength() * 0.01f;

		WorkingDamage *= StrengthMultiplier;
	}

	/*
	 * Critical hit roll.
	 *
	 * FMath::FRand() is acceptable for the prototype.
	 * Later, inject FRandomStream for deterministic tests.
	 */
	if (Request.bCanCrit && SourceAttributes)
	{
		const float CriticalChance = FMath::Clamp(
			SourceAttributes->GetCriticalChance(),
			0.0f,
			1.0f
		);

		if (FMath::FRand() <= CriticalChance)
		{
			WorkingDamage *=
				SourceAttributes->GetCriticalDamageMultiplier();

			Result.bWasCritical = true;
		}
	}

	Result.DamageBeforeMitigation = WorkingDamage;

	float MitigationMultiplier = 1.0f;

	if (TargetAttributes)
	{
		switch (Request.Element)
		{
		case EDamageElement::Physical:
			MitigationMultiplier =
				CalculatePhysicalMultiplier(
					TargetAttributes->GetArmor(),
					Request.ArmorPenetration
				);
			break;

		case EDamageElement::Fire:
			MitigationMultiplier =
				CalculateResistanceMultiplier(
					TargetAttributes->GetFireResistance(),
					Request.ResistancePenetration
				);
			break;

		case EDamageElement::Cold:
			MitigationMultiplier =
				CalculateResistanceMultiplier(
					TargetAttributes->GetColdResistance(),
					Request.ResistancePenetration
				);
			break;

			/*
			 * Lightning and Poison currently have no dedicated
			 * resistance attributes, so they deal unmitigated damage.
			 */
		case EDamageElement::Lightning:
		case EDamageElement::Poison:
		default:
			MitigationMultiplier = 1.0f;
			break;
		}
	}

	Result.FinalDamage = FMath::Max(
		0.0f,
		WorkingDamage * MitigationMultiplier
	);

	Result.MitigatedDamage = FMath::Max(
		0.0f,
		Result.DamageBeforeMitigation - Result.FinalDamage
	);

	return Result;
}

float UDamageCalculationLibrary::CalculatePhysicalMultiplier(
	float Armor,
	float ArmorPenetration
)
{
	const float EffectiveArmor = FMath::Max(
		0.0f,
		Armor - ArmorPenetration
	);

	/*
	 * Armor 0   -> multiplier 1.00
	 * Armor 25  -> multiplier 0.80
	 * Armor 100 -> multiplier 0.50
	 */
	return 100.0f / (100.0f + EffectiveArmor);
}

float UDamageCalculationLibrary::CalculateResistanceMultiplier(
	float Resistance,
	float ResistancePenetration
)
{
	const float EffectiveResistance = FMath::Clamp(
		Resistance - ResistancePenetration,
		0.0f,
		0.95f
	);

	return 1.0f - EffectiveResistance;
}