#include "BTTask_ActivateCombatAbility.h"

#include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "TestGame/Characters/EnemyCharacter.h"
#include "TestGame/AbilitySystem/AbilityInputContext.h"
#include "TestGame/AbilitySystem/TestGameAbilitySystemComponent.h"

UBTTask_ActivateCombatAbility::UBTTask_ActivateCombatAbility()
{
	NodeName = TEXT("Activate Combat Ability");

	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(
			UBTTask_ActivateCombatAbility,
			TargetActorKey
		),
		AActor::StaticClass()
	);
}

EBTNodeResult::Type
UBTTask_ActivateCombatAbility::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AIController =
		OwnerComp.GetAIOwner();

	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn =
		AIController->GetPawn();

	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* Blackboard =
		OwnerComp.GetBlackboardComponent();

	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor =
		Cast<AActor>(
			Blackboard->GetValueAsObject(
				TargetActorKey.SelectedKeyName)
		);

	if (!IsValid(TargetActor))
	{
		return EBTNodeResult::Failed;
	}

	AEnemyCharacter* EnemyCharacter =
		Cast<AEnemyCharacter>(ControlledPawn);

	if (!EnemyCharacter)
	{
		return EBTNodeResult::Failed;
	}

	const FGameplayTag AbilityInputTag =
		EnemyCharacter->GetPrimaryAttackInputTag();

	if (!AbilityInputTag.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	UTestGameAbilitySystemComponent* ASC =
		Cast<UTestGameAbilitySystemComponent>(
			UAbilitySystemBlueprintLibrary::
			GetAbilitySystemComponent(
				ControlledPawn
			)
		);

	if (!ASC)
	{

		return EBTNodeResult::Failed;
	}

	FAbilityInputContext Context;
	Context.TargetActor = TargetActor;
	Context.HitLocation =
		TargetActor->GetActorLocation();

	const bool bRequested =
		ASC->RequestAbility(
			AbilityInputTag,
			Context
		);

	return bRequested
		? EBTNodeResult::Succeeded
		: EBTNodeResult::Failed;
}

