#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_GenericAbility.generated.h"

class AGenericCharacter;

UCLASS()
class TESTGAME_API UGA_GenericAbility : public UGameplayAbility
{

	GENERATED_BODY()

public:
	
	UGA_GenericAbility();
	
protected:

	AGenericCharacter* GetGenericCharacter() const;

	void SetAbilityTarget(AActor* Target);

	AActor* GetAbilityTarget() const;

	void MoveToTarget(AActor* Target);

	virtual void OnTargetReached(AActor* Target);

	void UpdateMoveToTarget();

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Movement")
	float MoveStopDistance = 120.0f;

	FTimerHandle MoveTimerHandle;

	TWeakObjectPtr<AActor> CurrentTarget;

};
