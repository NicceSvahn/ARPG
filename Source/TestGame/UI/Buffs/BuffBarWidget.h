#pragma once

#include "CoreMinimal.h"
#include "../AbilitySystemWidget.h"
#include "GameplayTagContainer.h"
#include "BuffBarWidget.generated.h"

class UHorizontalBox;
class UBuffIconWidget;
class UBuffDisplayDataAsset;

UCLASS()
class TESTGAME_API UBuffBarWidget : public UAbilitySystemWidget
{
    GENERATED_BODY()

protected:
    virtual void OnAbilitySystemReady() override;
    virtual void UnbindFromAbilitySystem() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> BuffContainer;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Buffs"
    )
    TSubclassOf<UBuffIconWidget> BuffIconWidgetClass;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Buffs"
    )
    TObjectPtr<UBuffDisplayDataAsset> BuffDisplayData;

private:
    void HandleBuffTagChanged(
        FGameplayTag Tag,
        int32 NewCount
    );

    void AddBuff(
        const FGameplayTag& BuffTag
    );

    void RemoveBuff(
        const FGameplayTag& BuffTag
    );

    void RefreshExistingBuffs();

    void UpdateBuffDurations();

    float GetRemainingTimeForBuff(
        const FGameplayTag& BuffTag
    ) const;

    TMap<
        FGameplayTag,
        TObjectPtr<UBuffIconWidget>
    > ActiveBuffWidgets;

    TMap<
        FGameplayTag,
        FDelegateHandle
    > BuffTagDelegateHandles;

    FTimerHandle DurationUpdateTimer;
};