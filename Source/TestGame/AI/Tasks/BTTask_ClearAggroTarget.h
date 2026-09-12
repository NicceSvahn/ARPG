#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BTTask_ClearAggroTarget.generated.h"

UCLASS()
class TESTGAME_API UBTTask_ClearAggroTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ClearAggroTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
