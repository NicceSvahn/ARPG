#pragma once

#include "CoreMinimal.h"
#include "GA_GenericMelee.h"
#include "GA_Cleave.generated.h"

class AGenericCharacter;

UCLASS()
class TESTGAME_API UGA_Cleave : public UGA_GenericMelee
{
    GENERATED_BODY()

public:
    UGA_Cleave();

    virtual float GetMaximumRange() const override
    {
        return CleaveRange;
    }

protected:
    virtual void OnMeleeHit(
        AActor* TargetActor
    ) override;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Cleave",
        meta = (
            ClampMin = "0.0",
            Units = "cm"
            )
    )
    float CleaveRange = 250.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Cleave",
        meta = (
            ClampMin = "0.0",
            ClampMax = "360.0"
            )
    )
    float CleaveArcDegrees = 180.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Ability|Cleave"
    )
    TSubclassOf<AGenericCharacter> AffectedCharacterClass;

private:
    void ApplyCleaveDamage(
        AGenericCharacter* SourceCharacter
    );
};