#pragma once

#include "CoreMinimal.h"
#include "GenericCharacter.h"
#include "PlayerCharacter.generated.h"

class UAbilitySystemComponent;
class UHealthAttributeSet;
class ADamageNumberActor;


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
protected:
    virtual void BeginPlay() override;

    virtual void HandleAttributeChanged(
        FGameplayAttribute Attribute,
        float Magnitude,
        float NewHealth
    );

    virtual void OnRep_PlayerState() override;

    void LogPlayerIdentity() const;
};