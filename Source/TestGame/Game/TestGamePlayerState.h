#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TestGamePlayerState.generated.h"

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
};