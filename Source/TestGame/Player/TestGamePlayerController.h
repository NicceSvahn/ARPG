#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TestGamePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

DECLARE_DELEGATE_OneParam(
    FOnMoveIntoRangeCompleted,
    bool
);

UCLASS()
class TESTGAME_API ATestGamePlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ATestGamePlayerController();

    virtual void Tick(float DeltaTime) override;

    void MoveIntoRange(
        AActor* Target,
        float DesiredRange,
        FOnMoveIntoRangeCompleted OnCompleted
    );

    void CancelMoveIntoRange();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> ClickMoveAction;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> BashAction;

private:
    void OnClickMove();
    void OnBashPressed();

    void FinishMoveIntoRange(bool bSuccess);

    TWeakObjectPtr<AActor> MovementTarget;

    float MovementAcceptanceRadius = 0.0f;

    bool bIsMovingToTarget = false;
    FOnMoveIntoRangeCompleted MoveCompletedDelegate;

};