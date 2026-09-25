#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
//#include "Abilities/GameplayAbility.h"
#include "PlayerClass.h"
#include "../AbilitySystem/GrantedAbility.h"

#include "PlayerClassDefinitions.generated.h"

UCLASS(BlueprintType)
class TESTGAME_API UPlayerClassDefinitions : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class")
    EPlayerClass PlayerClass = EPlayerClass::None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class")
    FGameplayTag ClassTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
    TArray<FGrantedAbility> Abilities;
};