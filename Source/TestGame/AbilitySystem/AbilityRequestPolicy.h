#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AbilityRequestPolicy.generated.h"

/**
 * Ability-owned request requirements. The ASC reads these values from the
 * granted ability object and remains responsible for enforcing them.
 */
UINTERFACE(MinimalAPI)
class UAbilityRequestPolicy : public UInterface
{
    GENERATED_BODY()
};

class TESTGAME_API IAbilityRequestPolicy
{
    GENERATED_BODY()

public:
    virtual bool RequiresTarget() const { return false; }
    virtual float GetMaximumRange() const { return 0.0f; }

    /**
     * If true, Shift + ability may bypass normal target/range requirements
     * and activate as a stationary directional attack toward the cursor.
     */
    virtual bool SupportsForceDirectionalAttack() const { return false; }
};
