#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TestGameProceduralGameMode.generated.h"

class ALevelGenerator;
class AController;

UCLASS()
class TESTGAME_API ATestGameProceduralGameMode : public AGameMode
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason
    ) override;

    virtual void RestartPlayer(
        AController* NewPlayer
    ) override;

private:
    void HandleGeneratedMapReady();
    bool HasGroundAtPlayerStart(AController* Player);

    UPROPERTY()
    TObjectPtr<ALevelGenerator> LevelGenerator = nullptr;

    bool bMapReadyForPlayers = false;

    TArray<TWeakObjectPtr<AController>> WaitingPlayers;
};
