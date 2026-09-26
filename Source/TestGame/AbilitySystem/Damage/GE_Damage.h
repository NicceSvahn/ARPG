#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_Damage.generated.h"

/**
 * Instant GameplayEffect that delegates damage calculation to UExecCalc_Damage.
 * Abilities only need to build the outgoing spec with FAbilityDamageData.
 */
UCLASS()
class TESTGAME_API UGE_Damage : public UGameplayEffect
{
    GENERATED_BODY()

public:
    UGE_Damage();
};
