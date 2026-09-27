#include "EncounterManager.h"

#include "../LevelGeneration/LevelGenerator.h"
#include "../LevelGeneration/LevelChunkDefinition.h"
#include "../Characters/EnemyCharacter.h"
#include "../Game/TestGameGameState.h"
#include "../Characters/PlayerCharacter.h"
#include "../AI/EnemyAIController.h"

#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "NavigationSystem.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"

AEncounterManager::AEncounterManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;
}

void AEncounterManager::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    if (!GetWorld() ||
        !GetWorld()->IsGameWorld() ||
        GetNetMode() == NM_Client ||
        !HasAuthority() ||
        !IsValid(LevelGenerator))
    {
        return;
    }

    RandomStream.Initialize(EncounterSeed);

    LevelGenerator->OnGeneratedMapReady.AddUObject(
        this,
        &AEncounterManager::HandleMapReady
    );

    LevelGenerator->OnGeneratedChunksClearing.AddUObject(
        this,
        &AEncounterManager::HandleChunksClearing
    );
}

void AEncounterManager::EndPlay(
    const EEndPlayReason::Type EndPlayReason
)
{
    if (IsValid(LevelGenerator))
    {
        LevelGenerator->OnGeneratedMapReady.RemoveAll(this);
        LevelGenerator->OnGeneratedChunksClearing.RemoveAll(this);
    }

    HandleChunksClearing();
    Super::EndPlay(EndPlayReason);
}

void AEncounterManager::HandleMapReady()
{
    if (!HasAuthority() || bMapPopulated)
    {
        return;
    }

    NavigationRetryCount = 0;

    // Streamed geometry and dynamic navigation need some time
    // after the last chunk becomes visible.
    GetWorldTimerManager().SetTimer(
        NavigationRetryTimer,
        this,
        &AEncounterManager::TryPopulateMap,
        0.5f,
        true
    );

    UE_LOG(
        LogTemp,
        Log,
        TEXT("[ENCOUNTER] Map visible; waiting for navigation.")
    );
}

void AEncounterManager::TryPopulateMap()
{
    if (!HasAuthority() ||
        !IsValid(LevelGenerator) ||
        bMapPopulated)
    {
        GetWorldTimerManager().ClearTimer(NavigationRetryTimer);
        return;
    }

    UNavigationSystemV1* NavSystem =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(
            GetWorld()
        );

    if (!NavSystem ||
        NavSystem->IsNavigationBuildInProgress())
    {
        ++NavigationRetryCount;

        if (NavigationRetryCount >= 80)
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT(
                    "[ENCOUNTER] Timed out waiting for "
                    "navigation to finish building."
                )
            );

            GetWorldTimerManager().ClearTimer(
                NavigationRetryTimer
            );
        }

        return;
    }

    GetWorldTimerManager().ClearTimer(NavigationRetryTimer);
    bMapPopulated = true;

    for (const FGeneratedChunkInstance& Chunk :
        LevelGenerator->GetChunkInstances())
    {
        if (IsValid(Chunk.Definition) &&
            Chunk.Definition->ChunkType ==
            ELevelChunkType::Combat)
        {
            SpawnInChunk(Chunk, NavSystem);
        }
    }

    if (ATestGameGameState* GameState =
        GetWorld()->GetGameState<ATestGameGameState>())
    {
        GameState->SetEnemyKillProgress(
            TotalSpawnedEnemies,
            DeadEnemies
        );
    }
}

void AEncounterManager::SpawnInChunk(
    const FGeneratedChunkInstance& Chunk,
    UNavigationSystemV1* NavSystem
)
{
    if (EnemyTypes.IsEmpty() || !NavSystem || !GetWorld())
    {
        return;
    }

    const float ChunkSize =
        LevelGenerator->GetChunkSize();

    const float UsableHalfSize =
        ChunkSize * 0.5f - SpawnInset;

    if (UsableHalfSize <= 0.0f)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[ENCOUNTER] SpawnInset is too large.")
        );
        return;
    }

    // Matches the generator's current GridToWorldLocation().
    const FVector ChunkCenter(
        Chunk.GridCoordinate.X * ChunkSize,
        Chunk.GridCoordinate.Y * ChunkSize,
        0.0f
    );

    const int32 MinCount =
        FMath::Max(0, MinEnemiesPerCombatChunk);

    const int32 MaxCount =
        FMath::Max(
            MinCount,
            MaxEnemiesPerCombatChunk
        );

    const int32 DesiredCount =
        RandomStream.RandRange(MinCount, MaxCount);

    int32 SpawnedCount = 0;

    const int32 MaxAttempts =
        FMath::Max(20, DesiredCount * 20);

    for (int32 Attempt = 0;
        Attempt < MaxAttempts &&
        SpawnedCount < DesiredCount;
        ++Attempt)
    {
        const FVector Candidate(
            ChunkCenter.X +
            RandomStream.FRandRange(
                -UsableHalfSize,
                UsableHalfSize
            ),
            ChunkCenter.Y +
            RandomStream.FRandRange(
                -UsableHalfSize,
                UsableHalfSize
            ),
            ChunkCenter.Z
        );

        FNavLocation NavPoint;

        if (!NavSystem->ProjectPointToNavigation(
            Candidate,
            NavPoint,
            FVector(75.0f, 75.0f, 300.0f)
        ))
        {
            continue;
        }

        const FVector NavPosition =
            NavPoint.Location;

        // A navigation projection must remain in this chunk.
        if (FMath::Abs(
            NavPosition.X - ChunkCenter.X
        ) > UsableHalfSize ||
            FMath::Abs(
                NavPosition.Y - ChunkCenter.Y
            ) > UsableHalfSize)
        {
            continue;
        }

        bool bNearPlayer = false;

        for (FConstPlayerControllerIterator It =
            GetWorld()->GetPlayerControllerIterator();
            It;
            ++It)
        {
            const APlayerController* Controller =
                It->Get();

            if (Controller &&
                Controller->GetPawn() &&
                FVector::Dist2D(
                    NavPosition,
                    Controller->GetPawn()->GetActorLocation()
                ) < MinDistanceFromPlayers)
            {
                bNearPlayer = true;
                break;
            }
        }

        if (bNearPlayer)
        {
            continue;
        }

        TSubclassOf<AEnemyCharacter> SelectedClass =
            ChooseEnemyClass();

        if (!SelectedClass)
        {
            break;
        }

        const AEnemyCharacter* EnemyDefaults =
            SelectedClass->GetDefaultObject<AEnemyCharacter>();

        const UCapsuleComponent* Capsule =
            EnemyDefaults
            ? EnemyDefaults->GetCapsuleComponent()
            : nullptr;

        const float CapsuleHalfHeight =
            Capsule
            ? Capsule->GetScaledCapsuleHalfHeight()
            : 100.0f;

        const FVector SpawnPosition =
            NavPosition +
            FVector(
                0.0f,
                0.0f,
                CapsuleHalfHeight + 2.0f
            );

        const FRotator SpawnRotation(
            0.0f,
            RandomStream.FRandRange(
                -180.0f,
                180.0f
            ),
            0.0f
        );

        FActorSpawnParameters Params;

        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::
            AdjustIfPossibleButDontSpawnIfColliding;

        AEnemyCharacter* Enemy =
            GetWorld()->SpawnActor<AEnemyCharacter>(
                SelectedClass,
                SpawnPosition,
                SpawnRotation,
                Params
            );

        if (IsValid(Enemy))
        {
            #if WITH_EDITOR
                const FName EnemyFolder(TEXT("Enemies"));

                Enemy->SetFolderPath(EnemyFolder);

                if (AController* EnemyController = Enemy->GetController())
                {
                    EnemyController->SetFolderPath(EnemyFolder);
                }
            #endif

            Enemy->OnEnemyDied.AddUObject(
                this,
                 &AEncounterManager::HandleEnemyDied
            );

            SpawnedEnemies.Add(Enemy);
            ++TotalSpawnedEnemies;
            ++SpawnedCount;
        }
    }
}


void AEncounterManager::HandleChunksClearing()
{
    bMapPopulated = false;

    if (GetWorld())
    {
        GetWorldTimerManager().ClearTimer(NavigationRetryTimer);
        GetWorldTimerManager().ClearTimer(BossSpawnRetryTimer);
    }

    const bool bServer =
        GetWorld() &&
        GetNetMode() != NM_Client &&
        HasAuthority();

    if (bServer)
    {
        if (AEnemyCharacter* Boss = SpawnedBoss.Get())
        {
            // Cache the controller because destroying the pawn
            // may unpossess it.
            AController* BossController = Boss->GetController();

            Boss->Destroy();

            if (IsValid(BossController))
            {
                BossController->Destroy();
            }
        }

        for (
            const TWeakObjectPtr<AEnemyCharacter>& Enemy :
            SpawnedEnemies
            )
        {
            if (Enemy.IsValid())
            {
                Enemy->OnEnemyDied.RemoveAll(this);
                Enemy->Destroy();
            }
        }
    }

    SpawnedBoss.Reset();
    BossTargetPlayer.Reset();
    SpawnedEnemies.Reset();

    bBossSpawned = false;
    NavigationRetryCount = 0;
    TotalSpawnedEnemies = 0;
    DeadEnemies = 0;

    if (bServer)
    {
        if (ATestGameGameState* GameState =
            GetWorld()->GetGameState<ATestGameGameState>())
        {
            GameState->SetEnemyKillProgress(0, 0);
        }
    }
}


TSubclassOf<AEnemyCharacter>
AEncounterManager::ChooseEnemyClass()
{
    float TotalWeight = 0.0f;

    for (const FRandomEnemyEntry& Entry : EnemyTypes)
    {
        if (Entry.EnemyClass && Entry.Weight > 0.0f)
        {
            TotalWeight += Entry.Weight;
        }
    }

    if (TotalWeight <= 0.0f)
    {
        return nullptr;
    }

    float Roll =
        RandomStream.FRand() * TotalWeight;

    for (const FRandomEnemyEntry& Entry : EnemyTypes)
    {
        if (!Entry.EnemyClass || Entry.Weight <= 0.0f)
        {
            continue;
        }

        Roll -= Entry.Weight;

        if (Roll <= 0.0f)
        {
            return Entry.EnemyClass;
        }
    }

    // Handles a possible floating-point rounding edge case.
    for (int32 Index = EnemyTypes.Num() - 1;
        Index >= 0;
        --Index)
    {
        if (EnemyTypes[Index].EnemyClass &&
            EnemyTypes[Index].Weight > 0.0f)
        {
            return EnemyTypes[Index].EnemyClass;
        }
    }

    return nullptr;
}

void AEncounterManager::HandleEnemyDied(
    AEnemyCharacter* Enemy
)
{
    if (!HasAuthority() || !IsValid(Enemy))
    {
        return;
    }

    Enemy->OnEnemyDied.RemoveAll(this);

    DeadEnemies = FMath::Min(
        DeadEnemies + 1,
        TotalSpawnedEnemies
    );

    if (ATestGameGameState* GameState =
        GetWorld()->GetGameState<ATestGameGameState>())
    {
        GameState->SetEnemyKillProgress(
            TotalSpawnedEnemies,
            DeadEnemies
        );

        const int32 RequiredKills =
            GameState->GetRequiredEnemyKills();

        if (
            RequiredKills > 0 &&
            DeadEnemies >= RequiredKills &&
            !bBossSpawned &&
            !GetWorldTimerManager().IsTimerActive(BossSpawnRetryTimer)
            )
        {
            GetWorldTimerManager().SetTimer(
                BossSpawnRetryTimer,
                this,
                &AEncounterManager::TrySpawnBossNearPlayer,
                1.0f,
                true
            );

            TrySpawnBossNearPlayer();
        }
    }
}


void AEncounterManager::TrySpawnBossNearPlayer()
{
    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    if (
        GetNetMode() == NM_Client ||
        !HasAuthority() ||
        !bMapPopulated ||
        bBossSpawned
        )
    {
        GetWorldTimerManager().ClearTimer(BossSpawnRetryTimer);
        return;
    }

    if (
        !BossClass ||
        BossMinSpawnDistance < 0.0f ||
        BossMaxSpawnDistance <= BossMinSpawnDistance
        )
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("[BOSS] Assign BossClass and valid spawn distances.")
        );

        GetWorldTimerManager().ClearTimer(BossSpawnRetryTimer);
        return;
    }

    // Retain the selected player while they remain alive.
    APlayerCharacter* Player = BossTargetPlayer.Get();

    if (!IsValid(Player) || Player->bIsDead)
    {
        BossTargetPlayer.Reset();
        Player = nullptr;

        for (
            FConstPlayerControllerIterator It =
            World->GetPlayerControllerIterator();
            It;
            ++It
            )
        {
            APlayerController* PC = It->Get();

            APlayerCharacter* Candidate =
                PC ? Cast<APlayerCharacter>(PC->GetPawn()) : nullptr;

            if (IsValid(Candidate) && !Candidate->bIsDead)
            {
                Player = Candidate;
                BossTargetPlayer = Candidate;
                break;
            }
        }
    }

    if (!Player)
    {
        return;
    }

    UNavigationSystemV1* NavSystem =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);

    if (!NavSystem || NavSystem->IsNavigationBuildInProgress())
    {
        return;
    }

    const AEnemyCharacter* BossDefaults =
        BossClass->GetDefaultObject<AEnemyCharacter>();

    const UCapsuleComponent* Capsule =
        BossDefaults ? BossDefaults->GetCapsuleComponent() : nullptr;

    if (!Capsule)
    {
        UE_LOG(LogTemp, Error, TEXT("[BOSS] Boss has no capsule."));
        GetWorldTimerManager().ClearTimer(BossSpawnRetryTimer);
        return;
    }

    const float Radius = Capsule->GetScaledCapsuleRadius();
    const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[BOSS SIZE] Radius=%.1f | HalfHeight=%.1f | CapsuleScale=%s"),
        Radius,
        HalfHeight,
        *Capsule->GetComponentScale().ToString()
    );

    // Request navigation appropriate for the boss's size.
    FNavAgentProperties AgentProperties =
        BossDefaults->GetNavAgentPropertiesRef();

    AgentProperties.AgentRadius = Radius;
    AgentProperties.AgentHeight = HalfHeight * 2.0f;

    ANavigationData* NavData = NavSystem->GetNavDataForProps(
        AgentProperties,
        Player->GetActorLocation()
    );

    if (!NavData)
    {
        return;
    }

    FNavLocation PlayerNavLocation;

    if (!NavSystem->ProjectPointToNavigation(
        Player->GetActorLocation(),
        PlayerNavLocation,
        FVector(100.0f, 100.0f, 500.0f),
        NavData
    ))
    {
        return;
    }

    // Several candidates per retry, with no tick required.
    for (int32 Attempt = 0; Attempt < 20; ++Attempt)
    {
        FNavLocation BossNavLocation;

        if (!NavSystem->GetRandomReachablePointInRadius(
            PlayerNavLocation.Location,
            BossMaxSpawnDistance,
            BossNavLocation,
            NavData
        ))
        {
            continue;
        }

        const float Distance = FVector::Dist2D(
            BossNavLocation.Location,
            Player->GetActorLocation()
        );

        if (
            Distance < BossMinSpawnDistance ||
            Distance > BossMaxSpawnDistance
            )
        {
            continue;
        }

        // Navigation positions are near the floor.
        // Spawn the character's capsule center above it.
        const FVector SpawnLocation =
            BossNavLocation.Location +
            FVector(0.0f, 0.0f, HalfHeight + 100.0f);

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);

        const FCollisionResponseParams ResponseParams(
            Capsule->GetCollisionResponseToChannels()
        );

        if (World->OverlapBlockingTestByChannel(
            SpawnLocation,
            FQuat::Identity,
            Capsule->GetCollisionObjectType(),
            FCollisionShape::MakeCapsule(Radius, HalfHeight),
            QueryParams,
            ResponseParams
        ))
        {
            continue;
        }

        const FVector ToPlayer =
            Player->GetActorLocation() - SpawnLocation;

        const FRotator SpawnRotation(
            0.0f,
            ToPlayer.Rotation().Yaw,
            0.0f
        );

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;

        AEnemyCharacter* Boss = World->SpawnActor<AEnemyCharacter>(
            BossClass,
            SpawnLocation,
            SpawnRotation,
            Params
        );

        if (!IsValid(Boss))
        {
            continue;
        }

        SpawnedBoss = Boss;
        bBossSpawned = true;

        GetWorldTimerManager().ClearTimer(BossSpawnRetryTimer);

        if (!Boss->GetController())
        {
            Boss->SpawnDefaultController();
        }

        AEnemyAIController* BossController =
            Cast<AEnemyAIController>(Boss->GetController());

        if (BossController)
        {
            BossController->SetAggroTarget(Player);
        }
        else
        {
            UE_LOG(
                LogTemp,
                Error,
                TEXT("[BOSS] Spawned boss has no EnemyAIController.")
            );
        }

#if WITH_EDITOR
        Boss->SetFolderPath(FName(TEXT("Enemies/Boss")));

        if (BossController)
        {
            BossController->SetFolderPath(
                FName(TEXT("Enemies/Boss"))
            );
        }
#endif

        UE_LOG(
            LogTemp,
            Log,
            TEXT("[BOSS] Spawned %s near %s"),
            *GetNameSafe(Boss),
            *GetNameSafe(Player)
        );

        // Do not add the boss to SpawnedEnemies,
        // increment TotalSpawnedEnemies,
        // or bind HandleEnemyDied to the boss.
        return;
    }

    // All candidates failed. The timer tries again next second.
}