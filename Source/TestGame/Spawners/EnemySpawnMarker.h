#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnMarker.generated.h"

class UArrowComponent;

UCLASS()
class TESTGAME_API AEnemySpawnMarker : public AActor
{
    GENERATED_BODY()

public:
    AEnemySpawnMarker();

private:
    UPROPERTY(VisibleAnywhere, Category = "Spawn Marker")
    TObjectPtr<UArrowComponent> Arrow;
};