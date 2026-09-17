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
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[ENEMY AI] OnPossess rejected | "
                "Pawn=%s | Authority=FALSE"
            ),
            *GetNameSafe(InPawn)
        );

        return;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "[ENEMY AI] OnPossess | "
            "Controller=%s | Pawn=%s | Authority=TRUE"
        ),
        *GetNameSafe(this),
        *GetNameSafe(InPawn)
    );

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

	if (BlackboardComponent->GetValueAsObject(TargetActorKey) !=
		TargetToClear)
	{
		return;
	}

	BlackboardComponent->ClearValue(TargetActorKey);
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