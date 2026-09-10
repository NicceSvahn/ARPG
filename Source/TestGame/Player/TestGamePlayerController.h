#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "../AbilitySystem/AbilityInputContext.h"
#include "../UI/PlayerHudWidget.h"

#include "TestGamePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UPlayerHudWidget;

DECLARE_DELEGATE_OneParam(
    FOnMoveIntoRangeCompleted,
    bool
);

USTRUCT(BlueprintType)
struct FAbilityInputBinding
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> InputAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Input.Ability"))
    FGameplayTag InputTag;
};

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TArray<FAbilityInputBinding> AbilityInputBindings;

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnRep_Pawn() override;

    UPROPERTY(EditDefaultsOnly, Category = "HUD")
    TSubclassOf<UPlayerHudWidget> PlayerHudWidgetClass;

private:
    void OnClickMove();

    void OnAbilityInputPressed(FGameplayTag InputTag);

    void FinishMoveIntoRange(bool bSuccess);

    TWeakObjectPtr<AActor> MovementTarget;

    float MovementAcceptanceRadius = 0.0f;

    bool bIsMovingToTarget = false;
    FOnMoveIntoRangeCompleted MoveCompletedDelegate;

    void TryInitializeHud();

    UPROPERTY()
    TObjectPtr<UPlayerHudWidget> PlayerHudWidget;

};