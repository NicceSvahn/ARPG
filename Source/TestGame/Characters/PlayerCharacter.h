#pragma once

#include "CoreMinimal.h"
#include "GenericCharacter.h"
#include "PlayerCharacter.generated.h"

class ADamageNumberActor;
class UPlayerClassDefinitions;

UCLASS()
class TESTGAME_API APlayerCharacter : public AGenericCharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

    UPROPERTY(
        EditDefaultsOnly,
        Category = "Combat Text"
    )
    TSubclassOf<ADamageNumberActor> HealingNumberActorClass;

    void ApplyPlayerClassDefinition(
        const UPlayerClassDefinitions* ClassDefinition
    );

    void RemovePlayerClassDefinition(
        const UPlayerClassDefinitions* ClassDefinition
    );

protected:
    virtual void OnDeathStarted() override;
};
