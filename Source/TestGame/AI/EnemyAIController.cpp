#include "EnemyAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BrainComponent.h"

const FName AEnemyAIController::TargetActorKey =
	TEXT("TargetActor");

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!InPawn ||
        !InPawn->HasAuthority())
    {
        return;
    }

    if (!BehaviorTreeAsset)
    {
        return;
    }

    const bool bBehaviorTreeStarted =
        RunBehaviorTree(
            BehaviorTreeAsset
        );
}

void AEnemyAIController::SetAggroTarget(AActor* NewTarget)
{
	if (!IsValid(NewTarget))
	{
		return;
	}

    UBlackboardComponent* BlackboardComponent =
        GetBlackboardComponent();


    if (!BlackboardComponent)
    {
        return;
    }

    BlackboardComponent->SetValueAsObject(
        TargetActorKey,
        NewTarget
    );
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

	if (BlackboardComponent->GetValueAsObject(TargetActorKey) !=
		TargetToClear)
	{
		return;
	}

	BlackboardComponent->ClearValue(TargetActorKey);
	StopMovement();
}

void AEnemyAIController::ClearAggroTarget()
{
    UBlackboardComponent* BlackboardComponent =
        GetBlackboardComponent();

    if (!BlackboardComponent)
    {
        return;
    }

    BlackboardComponent->ClearValue(
        TargetActorKey
    );

    StopMovement();
}

void AEnemyAIController::
HandleControlledPawnDeath()
{
	if (UBlackboardComponent*
		BlackboardComponent =
		GetBlackboardComponent())
	{
		BlackboardComponent->ClearValue(TargetActorKey);
	}

	StopMovement();

	if (BrainComponent)
	{
		BrainComponent->StopLogic(TEXT("Controlled pawn died"));
	}
}