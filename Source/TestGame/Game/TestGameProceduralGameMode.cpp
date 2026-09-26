#include "TestGameProceduralGameMode.h"


#include "../LevelGeneration/LevelGenerator.h"

#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"

void ATestGameProceduralGameMode::BeginPlay()
{
    Super::BeginPlay();

    for (TActorIterator<ALevelGenerator> It(GetWorld());
        It;
        ++It)
    {
        LevelGenerator = *It;
        break;
    }

    if (!IsValid(LevelGenerator))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[SPAWN] No LevelGenerator found. "
                "Player spawning is waiting."
            )
        );
        return;
    }

    LevelGenerator->OnGeneratedMapReady.AddUObject(
        this,
        &ATestGameProceduralGameMode::HandleGeneratedMapReady
    );

    // Covers a map that became ready before this binding.
    if (LevelGenerator->IsGeneratedMapReady())
    {
        HandleGeneratedMapReady();
    }
}

void ATestGameProceduralGameMode::RestartPlayer(
    AController* NewPlayer
)
{
    if (!IsValid(NewPlayer))
    {
        return;
    }

    if (!bMapReadyForPlayers)
    {
        WaitingPlayers.AddUnique(NewPlayer);

        UE_LOG(
            LogTemp,
            Log,
            TEXT(
                "[SPAWN] Holding player until "
                "generated chunks are visible."
            )
        );
        return;
    }

    if (!HasGroundAtPlayerStart(NewPlayer))
    {
        WaitingPlayers.AddUnique(NewPlayer);

        UE_LOG(
            LogTemp,
            Error,
            TEXT(
                "[SPAWN] Map is visible, but the selected "
                "PlayerStart has no suitable ground beneath it."
            )
        );
        return;
    }

    WaitingPlayers.Remove(NewPlayer);
    Super::RestartPlayer(NewPlayer);
}

void ATestGameProceduralGameMode::HandleGeneratedMapReady()
{
    bMapReadyForPlayers = true;

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[SPAWN] Generated map ready; releasing players.")
    );

    const TArray<TWeakObjectPtr<AController>> PlayersToStart =
        WaitingPlayers;

    WaitingPlayers.Reset();

    for (const TWeakObjectPtr<AController>& Player :
        PlayersToStart)
    {
        if (Player.IsValid())
        {
            RestartPlayer(Player.Get());
        }
    }
}

bool ATestGameProceduralGameMode::HasGroundAtPlayerStart(
    AController* Player
)
{
    UWorld* World = GetWorld();

    if (!World || !Player)
    {
        return false;
    }

    AActor* Start = FindPlayerStart(Player);

    if (!IsValid(Start))
    {
        return false;
    }

    const FVector StartLocation =
        Start->GetActorLocation();

    FHitResult Hit;

    const bool bHit =
        World->LineTraceSingleByChannel(
            Hit,
            StartLocation + FVector(0.0, 0.0, 50.0),
            StartLocation - FVector(0.0, 0.0, 300.0),
            ECC_WorldStatic
        );

    return bHit &&
        Hit.ImpactNormal.Z >= 0.7 &&
        StartLocation.Z - Hit.ImpactPoint.Z <= 300.0;
}

void ATestGameProceduralGameMode::EndPlay(
    const EEndPlayReason::Type EndPlayReason
)
{
    if (IsValid(LevelGenerator))
    {
        LevelGenerator->OnGeneratedMapReady.RemoveAll(this);
    }

    WaitingPlayers.Reset();

    Super::EndPlay(EndPlayReason);
}