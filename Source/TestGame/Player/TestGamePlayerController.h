#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"

#include "../Characters/PlayerClass.h"

#include "TestGamePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UPlayerHudWidget;
class UCombatDebugWidget;
class UCharacterPanelWidget;
class UCameraOccludableComponent;
class AGenericCharacter;

DECLARE_DELEGATE_OneParam(
    FOnMoveIntoRangeCompleted,
    bool
);

#define ECC_MovementGround ECC_GameTraceChannel1

USTRUCT(BlueprintType)
struct FAbilityInputBinding
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> InputAction;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        meta = (Categories = "Input.Ability")
    )
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

    // Used by abilities that must attack in place. Stops both click movement
    // and target-chasing movement immediately.
    void StopMovementForAbility();

    // Combat debug helper.
    AGenericCharacter* GetCharacterUnderCursor() const;

    UFUNCTION(BlueprintCallable, Category = "Class")
    void RequestPlayerClass(EPlayerClass NewClass);

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnRep_Pawn() override;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> ClickMoveAction;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    TArray<FAbilityInputBinding> AbilityInputBindings;

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

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<UInputAction> IA_ToggleCharacterPanel;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UCharacterPanelWidget> CharacterPanelClass;

    UPROPERTY()
    TObjectPtr<UCharacterPanelWidget> CharacterPanelWidget;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float ClickMoveAcceptanceRadius = 50.0f;

    UFUNCTION(Server, Reliable)
    void ServerRequestPlayerClass(EPlayerClass NewClass);

private:
    void OnAbilityInputPressed(FGameplayTag InputTag);
    void OnAbilityInputReleased(FGameplayTag InputTag);

    void FinishMoveIntoRange(bool bSuccess);

    void TryInitializeHud();
    void TryInitializeCharacterPanel();

    void ToggleCombatDebug();
    bool bCombatDebugVisible = false;

    void UpdateCameraOcclusion();

    void OnClickMove();
    void StartClickMove(const FVector& Destination);
    void UpdateClickMove();
    void StopClickMove();

    void ToggleCharacterPanel();

    TWeakObjectPtr<AActor> MovementTarget;
    float MovementAcceptanceRadius = 0.0f;
    bool bIsMovingToTarget = false;
    FOnMoveIntoRangeCompleted MoveCompletedDelegate;

    TSet<TWeakObjectPtr<UCameraOccludableComponent>> OccludedComponents;

    FVector ClickMoveDestination = FVector::ZeroVector;
    bool bIsClickMoving = false;
};
