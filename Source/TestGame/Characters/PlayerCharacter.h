#pragma once

#include "CoreMinimal.h"
#include "GenericCharacter.h"
#include "PlayerCharacter.generated.h"

class UAbilitySystemComponent;
class UHealthAttributeSet;
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
    TSubclassOf<ADamageNumberActor>HealingNumberActorClass;

    //Class
    void ApplyPlayerClassDefinition(
        const UPlayerClassDefinitions* ClassDefinition
    );

    void RemovePlayerClassDefinition(
        const UPlayerClassDefinitions* ClassDefinition
    );
protected:
    virtual void BeginPlay() override;

    virtual void HandleAttributeChanged(
        FGameplayAttribute Attribute,
        float Magnitude,
        float NewHealth
    );

    virtual void OnRep_PlayerState() override;

    void LogPlayerIdentity() const;

    virtual void OnDeathStarted() override;
};