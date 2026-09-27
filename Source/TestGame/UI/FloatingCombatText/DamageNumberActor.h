#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatTextData.h"
#include "DamageNumberActor.generated.h"

class UWidgetComponent;
class UFloatingDamageWidget;

UCLASS()
class TESTGAME_API ADamageNumberActor : public AActor
{
    GENERATED_BODY()

public:
    ADamageNumberActor();

    void InitializeCombatText(const FCombatTextData& Data);

    void InitializeDamage(float DamageAmount, bool bCritical = false);
    void InitializeHealing(float HealingAmount);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UWidgetComponent> WidgetComponent;

private:
};