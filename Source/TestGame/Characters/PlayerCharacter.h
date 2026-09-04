#pragma once

#include "CoreMinimal.h"
#include "GenericCharacter.h"
#include "PlayerCharacter.generated.h"

class UAbilitySystemComponent;
class UHealthAttributeSet;

UCLASS()
class TESTGAME_API APlayerCharacter : public AGenericCharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

protected:
    virtual void BeginPlay() override;
};