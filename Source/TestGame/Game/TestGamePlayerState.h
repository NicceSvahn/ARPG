#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "../Characters/PlayerClass.h"
#include "TestGamePlayerState.generated.h"

class UPlayerClassDefinitions;

UCLASS()
class TESTGAME_API ATestGamePlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ATestGamePlayerState();

    UFUNCTION(BlueprintPure, Category = "Player")
    FString GetDebugPlayerName() const;

    UFUNCTION(BlueprintPure, Category = "Player")
    FString GetDebugPlayerLabel() const;

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    // Class
    UFUNCTION(BlueprintPure, Category = "Class")
    EPlayerClass GetPlayerClass() const
    {
        return SelectedClass;
    }

    UFUNCTION(BlueprintCallable, Category = "Class")
    void SetPlayerClass(EPlayerClass NewClass);

protected:

    // Class
    UPROPERTY(
        ReplicatedUsing = OnRep_SelectedClass,
        BlueprintReadOnly,
        Category = "Class"
    )
    EPlayerClass SelectedClass = EPlayerClass::None;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "Class"
    )
    TArray<TObjectPtr<UPlayerClassDefinitions>> ClassDefinitions;

    UFUNCTION()
    void OnRep_SelectedClass();

    void SyncPlayerClassTag();
    void ApplyPlayerClassTagToASC();

    const UPlayerClassDefinitions*
        FindClassDefinition(EPlayerClass PlayerClass) const;
};