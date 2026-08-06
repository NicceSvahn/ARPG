#pragma once

#include "CoreMinimal.h"
#include "GenericCharacter.h"
#include "AbilityTypes.h"
#include "DamageTypes.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class TESTGAME_API APlayerCharacter : public AGenericCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	/*
	 * Stores an ability request and its target.
	 *
	 * Returns false if the target is invalid.
	 */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	bool RequestTargetedAbility(
		EAbilityType AbilityType,
		AGenericCharacter* Target
	);

	/*
	 * Checks whether the stored target is currently within
	 * the requested ability's range.
	 */
	UFUNCTION(BlueprintPure, Category = "Abilities")
	bool IsPendingAbilityInRange() const;

	/*
	 * Executes the currently requested ability.
	 *
	 * Returns an empty FDamageResult if execution fails.
	 */
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	FDamageResult TryExecutePendingAbility();

	UFUNCTION(BlueprintPure, Category = "Abilities")
	AGenericCharacter* GetPendingAbilityTarget() const
	{
		return PendingAbilityTarget;
	}

	UFUNCTION(BlueprintPure, Category = "Abilities")
	EAbilityType GetPendingAbilityType() const
	{
		return PendingAbilityType;
	}

	UFUNCTION(BlueprintPure, Category = "Abilities")
	bool HasPendingAbility() const
	{
		return IsValid(PendingAbilityTarget);
	}

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void ClearPendingAbility();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FaceTarget(const AActor* Target);

protected:
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Abilities|Basic Attack"
	)
	FAbilitySpec BasicAttackSpec;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Abilities|Bash"
	)
	FAbilitySpec BashSpec;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

private:
	const FAbilitySpec* GetAbilitySpec(
		EAbilityType AbilityType
	) const;

	bool IsAbilityOnCooldown(
		const FAbilitySpec& AbilitySpec
	) const;

	void StartAbilityCooldown(
		EAbilityType AbilityType
	);

	FDamageResult ExecuteDamageAbility(
		AGenericCharacter* Target,
		const FAbilitySpec& AbilitySpec
	);

	UPROPERTY()
	TObjectPtr<AGenericCharacter> PendingAbilityTarget = nullptr;

	EAbilityType PendingAbilityType =
		EAbilityType::BasicAttack;

	TMap<EAbilityType, float> LastAbilityUseTimes;
	
	FVector MovementTarget = FVector::ZeroVector;
	bool bHasMovementTarget = false;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float AcceptanceRadius = 50.f;

	float LastBasicAttackTime = -1000.0f;
};