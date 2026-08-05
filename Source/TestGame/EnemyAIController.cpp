// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"


void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("EnemyAIController BeginPlay"));

	APawn* PlayerPawn =
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