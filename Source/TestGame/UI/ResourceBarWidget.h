#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemWidget.h"
#include "ResourceBarWidget.generated.h"

class UProgressBar;
struct FOnAttributeChangeData;

UCLASS()
class TESTGAME_API UResourceBarWidget : public UAbilitySystemWidget
{
    GENERATED_BODY()

public:
    /**
     * Can still be called manually from Blueprint if needed,
     * but normal HUD operation is GAS-driven.
     */
    UFUNCTION(BlueprintCallable, Category = "HUD|Resource")
    void SetResource(
        float CurrentResource,
        float MaxResource
    );

protected:
    virtual void OnAbilitySystemReady() override;
    virtual void UnbindFromAbilitySystem() override;

    UPROPERTY(
        BlueprintReadOnly,
        meta = (BindWidget),
        Category = "Resource"
    )
    TObjectPtr<UProgressBar> ResourceBar;

private:
    void RefreshResource();

    void HandleResourceChanged(
        const FOnAttributeChangeData& Data
    );

    void HandleMaxResourceChanged(
        const FOnAttributeChangeData& Data
    );

    FDelegateHandle ResourceChangedHandle;
    FDelegateHandle MaxResourceChangedHandle;
};