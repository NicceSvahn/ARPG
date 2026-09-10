#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageNumberActor.generated.h"

class UWidgetComponent;
class UFloatingDamageWidget;

UCLASS()
class TESTGAME_API ADamageNumberActor : public AActor
{
    GENERATED_BODY()

public:
    ADamageNumberActor();

    void InitializeDamage(float DamageAmount);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UWidgetComponent> WidgetComponent;
};