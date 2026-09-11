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
class UCombatDebugWidget;
class AGenericCharacter;

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
    
    // For Debugging
    AGenericCharacter* GetCharacterUnderCursor() const;

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

    UPROPERTY()
    TObjectPtr<UPlayerHudWidget> PlayerHudWidget;

    UPROPERTY(EditDefaultsOnly, Category = "Debug")
    TSubclassOf<UCombatDebugWidget> CombatDebugWidgetClass;

    UPROPERTY()
    TObjectPtr<UCombatDebugWidget> CombatDebugWidget;

    UPROPERTY(EditDefaultsOnly, Category = "Input|Debug")
    TObjectPtr<UInputAction> ToggleCombatDebugAction;

    bool bCombatDebugVisible = false;

    void ToggleCombatDebug();

private:
    void OnClickMove();

    void OnAbilityInputPressed(FGameplayTag InputTag);

    void FinishMoveIntoRange(bool bSuccess);

    void TryInitializeHud();

    TWeakObjectPtr<AActor> MovementTarget;

    float MovementAcceptanceRadius = 0.0f;

    bool bIsMovingToTarget = false;
    FOnMoveIntoRangeCompleted MoveCompletedDelegate;
};