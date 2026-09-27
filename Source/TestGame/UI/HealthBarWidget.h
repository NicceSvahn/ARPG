#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemWidget.h"
#include "HealthBarWidget.generated.h"

class UProgressBar;
struct FOnAttributeChangeData;

UCLASS()
class TESTGAME_API UHealthBarWidget : public UAbilitySystemWidget
{
    GENERATED_BODY()

public:
    // Kept for manual/Blueprint use. GAS-driven HUDs should use InitializeFromActor.
    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetHealth(float CurrentHealth, float MaxHealth);

protected:
    virtual void OnAbilitySystemReady() override;
    virtual void UnbindFromAbilitySystem() override;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Health")
    TObjectPtr<UProgressBar> HealthBar;

private:
    void RefreshHealth();
    void HandleHealthChanged(const FOnAttributeChangeData& Data);
    void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);

    FDelegateHandle HealthChangedHandle;
    FDelegateHandle MaxHealthChangedHandle;
};
