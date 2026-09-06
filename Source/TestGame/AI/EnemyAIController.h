#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class TESTGAME_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaSeconds) override;

private:
	
	UPROPERTY()
	TObjectPtr<APawn> PlayerPawn;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float BashRange = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float BashAttemptInterval = 2.0f;

	float BashAttemptTimer = 0.0f;

	void UpdateCombat(float DeltaSeconds);

	void TryBash();

};
