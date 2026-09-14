#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ResourceAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class TESTGAME_API UResourceAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UResourceAttributeSet();

    virtual void PostGameplayEffectExecute(
        const FGameplayEffectModCallbackData& Data
    ) override;

    UPROPERTY(BlueprintReadOnly, Category = "Resource")
    FGameplayAttributeData Resource;

    ATTRIBUTE_ACCESSORS(
        UResourceAttributeSet,
        Resource
    )

        UPROPERTY(BlueprintReadOnly, Category = "Resource")
    FGameplayAttributeData MaxResource;

    ATTRIBUTE_ACCESSORS(
        UResourceAttributeSet,
        MaxResource
    )
};