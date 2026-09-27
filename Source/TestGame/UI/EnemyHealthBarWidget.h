#pragma once

#include "CoreMinimal.h"
#include "HealthBarWidget.h"
#include "EnemyHealthBarWidget.generated.h"

/**
 * World-space enemy health bar.
 * All GAS health observation lives in UHealthBarWidget; this class exists so
 * enemy-specific Blueprint styling can keep its own native parent class.
 */
UCLASS()
class TESTGAME_API UEnemyHealthBarWidget : public UHealthBarWidget
{
    GENERATED_BODY()
};
