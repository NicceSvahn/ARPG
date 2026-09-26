#include "EnemySpawnMarker.h"

#include "Components/ArrowComponent.h"

AEnemySpawnMarker::AEnemySpawnMarker()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = false;

    Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
    SetRootComponent(Arrow);

    Arrow->SetArrowColor(FColor::Red);
    Arrow->ArrowSize = 1.5f;
}