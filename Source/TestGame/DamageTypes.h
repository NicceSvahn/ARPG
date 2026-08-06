#pragma once

#include "CoreMinimal.h"
#include "DamageTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EDamageElement : uint8
{
	Physical	UMETA(DisplayName = "Physical"),
	Fire		UMETA(DisplayName = "Fire"),
	Cold		UMETA(DisplayName = "Cold"),
	Lightning	UMETA(DisplayName = "Lightning"),
	Poison		UMETA(DisplayName = "Poison")
};

USTRUCT(BlueprintType)
struct TESTGAME_API FDamageRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	EDamageElement Element = EDamageElement::Physical;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Damage")
	TObjectPtr<AActor> DamageCauser = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bCanCrit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float ArmorPenetration = 0.0f;

	/*
	 * Stored as a fraction:
	 * 0.10 means 10 percentage points of resistance penetration.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float ResistancePenetration = 0.0f;
};

USTRUCT(BlueprintType)
struct TESTGAME_API FDamageResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float RawDamage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float DamageBeforeMitigation = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float MitigatedDamage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float FinalDamage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	bool bWasCritical = false;
};