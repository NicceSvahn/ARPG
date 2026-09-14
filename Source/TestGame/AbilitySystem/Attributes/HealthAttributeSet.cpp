#include "HealthAttributeSet.h"
#include "AbilitySystemComponent.h"

bool UHealthAttributeSet::PreGameplayEffectExecute(
    FGameplayEffectModCallbackData& Data)
{
    UAbilitySystemComponent* ASC =
        GetOwningAbilitySystemComponent();

    const FGameplayTag DeadTag =
        FGameplayTag::RequestGameplayTag(
            FName(TEXT("State.Dead"))
        );

    if (Data.EvaluatedData.Attribute ==
        GetHealthAttribute() &&
        ASC &&
        ASC->HasMatchingGameplayTag(DeadTag))
    {
        return false;
    }

    return Super::PreGameplayEffectExecute(
        Data
    );
}

void UHealthAttributeSet::PostGameplayEffectExecute(
    const FGameplayEffectModCallbackData& Data)
{
    if (Data.EvaluatedData.Attribute ==
        GetHealthAttribute())
    {
        SetHealth(FMath::Max(0.0f, GetHealth()));
    }

    Super::PostGameplayEffectExecute(Data);
}


