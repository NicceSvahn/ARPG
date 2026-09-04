#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_GenericAbility.generated.h"

class AGenericCharacter;

UCLASS(Abstract)
class TESTGAME_API UGA_GenericAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_GenericAbility();

protected:
    AGenericCharacter* GetGenericCharacter() const;
};