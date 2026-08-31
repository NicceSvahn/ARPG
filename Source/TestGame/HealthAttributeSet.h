#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HealthAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UHealthAttributeSet : public UGenericAttributeSet
{
    GENERATED_BODY()

public:
    void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

    UPROPERTY()
    FGenericAttributeEvent OnHealthChanged;

    UPROPERTY()
    FGameplayAttributeData Health;
    PLAY_ATTRIBUTE_ACCESSORS(UHealthAttributeSet, Health);
};