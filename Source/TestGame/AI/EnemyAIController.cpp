#include "EnemyAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

const FName AEnemyAIController::TargetActorKey =
	TEXT("TargetActor");

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!BehaviorTreeAsset)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("%s has no Behavior Tree assigned"),
			*GetName()
		);

		return;
	}

	if (!RunBehaviorTree(BehaviorTreeAsset))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("%s failed to start its Behavior Tree"),
			*GetName()
		);
	}
}

void AEnemyAIController::SetAggroTarget(AActor* NewTarget)
{
	if (!IsValid(NewTarget))
	{
		return;
	}

	if (UBlackboardComponent* BlackboardComponent =
		GetBlackboardComponent())
	{
		BlackboardComponent->SetValueAsObject(
			TargetActorKey,
			NewTarget
		);
	}
}

void AEnemyAIController::ClearAggroTarget(
	AActor* TargetToClear)
{
	UBlackboardComponent* BlackboardComponent =
		GetBlackboardComponent();

	if (!BlackboardComponent)
	{
		return;
	}

	if (Blackboard->GetValueAsObject(TargetActorKey) !=
		TargetToClear)
	{
		return;
	}

	Blackboard->ClearValue(TargetActorKey);
	StopMovement();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("%s cleared aggro target %s"),
		*GetName(),
		*TargetToClear->GetName()
	);
}