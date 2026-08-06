#pragma once

#include "CoreMinimal.h"
#include "DamageTypes.h"
#include "AbilityTypes.generated.h"

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	BasicAttack	UMETA(DisplayName = "Basic Attack"),
	Bash		UMETA(DisplayName = "Bash")
};

USTRUCT(BlueprintType)
struct TESTGAME_API FAbilitySpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	EAbilityType AbilityType = EAbilityType::BasicAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float BaseDamage = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float Range = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	float Cooldown = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	EDamageElement DamageElement = EDamageElement::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	bool bCanCrit = true;
};