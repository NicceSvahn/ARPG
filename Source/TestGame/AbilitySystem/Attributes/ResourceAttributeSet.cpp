#include "ResourceAttributeSet.h"

#include "GameplayEffectExtension.h"

UResourceAttributeSet::UResourceAttributeSet()
{
    InitMaxResource(100.0f);
    InitResource(100.0f);
}

void UResourceAttributeSet::PostGameplayEffectExecute(
    const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetResourceAttribute())
    {
        SetResource(
            FMath::Clamp(
                GetResource(),
                0.0f,
                GetMaxResource()
            )
        );
    }

    if (Data.EvaluatedData.Attribute == GetMaxResourceAttribute())
    {
        SetMaxResource(
            FMath::Max(
                GetMaxResource(),
                0.0f
            )
        );

        SetResource(
            FMath::Clamp(
                GetResource(),
                0.0f,
                GetMaxResource()
            )
        );
    }
}