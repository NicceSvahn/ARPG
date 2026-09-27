#pragma once

#include "CoreMinimal.h"
#include "DamageResult.generated.h"

USTRUCT()
struct TESTGAME_API FDamageResult
{
    GENERATED_BODY()

    float DamageAmount = 0.0f;

    bool bCritical = false;

    TWeakObjectPtr<AActor> AggroInstigator;
};