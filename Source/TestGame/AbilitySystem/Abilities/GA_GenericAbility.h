#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_GenericAbility.generated.h"

class AGenericCharacter;
class UTexture2D;

UCLASS(Abstract)
class TESTGAME_API UGA_GenericAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGA_GenericAbility();

    UFUNCTION(BlueprintPure, Category = "Ability|UI")
    FText GetAbilityName() const { return AbilityName; }

    UFUNCTION(BlueprintPure, Category = "Ability|UI")
    UTexture2D* GetAbilityIcon() const { return AbilityIcon; }

    UFUNCTION(BlueprintPure, Category = "Ability|UI")
    FText GetAbilityDescription() const { return AbilityDescription; }

protected:
    AGenericCharacter* GetGenericCharacter() const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|UI")
    FText AbilityName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|UI")
    TObjectPtr<UTexture2D> AbilityIcon = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|UI")
    FText AbilityDescription;
};