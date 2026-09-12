#include "BTTask_ClearAggroTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_ClearAggroTarget::UBTTask_ClearAggroTarget()
{
	NodeName = TEXT("Clear Aggro Target");

	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(
			UBTTask_ClearAggroTarget,
			TargetActorKey
		),
		AActor::StaticClass()
	);
}

EBTNodeResult::Type
UBTTask_ClearAggroTarget::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    UBlackboardComponent* BlackboardComponent =
        OwnerComp.GetBlackboardComponent();

    if (!BlackboardComponent)
    {
        return EBTNodeResult::Failed;
    }

    BlackboardComponent->ClearValue(
        TargetActorKey.SelectedKeyName
    );

    if (AAIController* AIController =
        OwnerComp.GetAIOwner())
    {
        AIController->StopMovement();
    }

    return EBTNodeResult::Succeeded;
}


