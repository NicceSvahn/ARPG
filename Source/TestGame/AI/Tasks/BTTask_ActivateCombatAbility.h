#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "GameplayTagContainer.h"
#include "BTTask_ActivateCombatAbility.generated.h"

UCLASS()
class TESTGAME_API UBTTask_ActivateCombatAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ActivateCombatAbility();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;

private:
	UPROPERTY(
		EditAnywhere,
		Category = "Blackboard"
	)
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(
		EditAnywhere,
		Category = "Ability",
		meta = (ExposeFunctionCategories = "Input.Ability")
	)
	FGameplayTag AbilityInputTag;

};
