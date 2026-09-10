#pragma once

#include "CoreMinimal.h"
#include "GenericCharacter.h"
#include "PlayerCharacter.generated.h"

class UAbilitySystemComponent;
class UHealthAttributeSet;
class UHealthBarWidget;
class UWidgetComponent;


UCLASS()
class TESTGAME_API APlayerCharacter : public AGenericCharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

protected:
    virtual void BeginPlay() override;

    void RefreshHealthBar(float CurrentHealth);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UWidgetComponent> HealthWidget;

    virtual void HandleAttributeChanged(
        FGameplayAttribute Attribute,
        float Magnitude,
        float NewHealth
    ) override;

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdatePlayerHealthUI(float CurrentHealth, float MaxHealth);

};