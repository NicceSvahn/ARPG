#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "GrantedAbility.generated.h"

USTRUCT(BlueprintType)
struct FGrantedAbility
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UGameplayAbility> AbilityClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Input.Ability"))
    FGameplayTag InputTag;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        meta = (Categories = "Event.Ability")
    )
    FGameplayTag EventTag;
};