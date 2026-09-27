#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "EnemyKillProgress.generated.h"

class ATestGameGameState;
class UProgressBar;

UCLASS()
class TESTGAME_API UEnemyKillProgress : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // The child Widget Blueprint must contain a Progress Bar with this name.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> KillProgressBar = nullptr;

private:
    void TryBindGameState();
    void UnbindGameState();

    UFUNCTION()
    void RefreshProgress();

    TWeakObjectPtr<ATestGameGameState> BoundGameState;
    FTimerHandle GameStateRetryTimer;
};