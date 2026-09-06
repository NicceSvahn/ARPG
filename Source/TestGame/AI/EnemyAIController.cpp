#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagContainer.h"


AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// Run AI logic 5 times per second instead of every frame.
	PrimaryActorTick.TickInterval = 0.2f;
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("AIController %s possessed %s"),
		*GetName(),
		InPawn ? *InPawn->GetName() : TEXT("NONE")
	);

	PlayerPawn =
		UGameplayStatics::GetPlayerPawn(
			GetWorld(),
			0
		);

	if (PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player found: %s"), *PlayerPawn->GetName());

		EPathFollowingRequestResult::Type Result = MoveToActor(PlayerPawn, 200.f);

		UE_LOG(LogTemp, Warning, TEXT("Move result: %d"), (int32)Result);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Player NOT found"));
	}
}

void AEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCombat(DeltaSeconds);
}

void AEnemyAIController::UpdateCombat(float DeltaSeconds)
{
	APawn* ControlledPawn = GetPawn();

	if (!ControlledPawn || !PlayerPawn)
	{
		return;
	}

	BashAttemptTimer += DeltaSeconds;

	const float Distance = FVector::Dist2D(ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation());

	// If close enough, stop chasing and try Bash.
	if (Distance <= BashRange)
	{
		StopMovement();

		if (BashAttemptTimer >= BashAttemptInterval)
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT("=== SKELETON WANTS TO BASH === Attacker=%s Target=%s Distance=%.1f"),
				*ControlledPawn->GetName(),
				*PlayerPawn->GetName(),
				Distance
			);

			TryBash();

			BashAttemptTimer = 0.0f;
		}

		return;
	}

	// Player is outside Bash range, keep following them.
	MoveToActor(
		PlayerPawn,
		BashRange * 0.8f
	);
}

void AEnemyAIController::TryBash()
{
	APawn* ControlledPawn = GetPawn();

	if (!ControlledPawn || !PlayerPawn)
	{
		return;
	}

	UE_LOG(
		LogTemp,
		Error,
		TEXT("=== SKELETON TRYING BASH === Attacker=%s Target=%s"),
		*ControlledPawn->GetName(),
		*PlayerPawn->GetName()
	);

	FGameplayEventData EventData;

	EventData.Instigator = ControlledPawn;
	EventData.Target = PlayerPawn;

	const FGameplayTag BashTag =
		FGameplayTag::RequestGameplayTag(
			FName("Event.Ability.Bash")
		);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		ControlledPawn,
		BashTag,
		EventData
	);
}





