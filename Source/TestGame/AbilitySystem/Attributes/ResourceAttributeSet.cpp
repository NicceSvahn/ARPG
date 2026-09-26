#include "ResourceAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

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

void UResourceAttributeSet::OnRep_ResourceRegeneration(
    const FGameplayAttributeData& OldResourceRegeneration)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UResourceAttributeSet,
        ResourceRegeneration,
        OldResourceRegeneration
    );
}

void UResourceAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(
        OutLifetimeProps
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UResourceAttributeSet,
        Resource,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UResourceAttributeSet,
        MaxResource,
        COND_None,
        REPNOTIFY_Always
    );

    DOREPLIFETIME_CONDITION_NOTIFY(
        UResourceAttributeSet,
        ResourceRegeneration,
        COND_None,
        REPNOTIFY_Always
    );
}

void UResourceAttributeSet::OnRep_Resource(
    const FGameplayAttributeData& OldResource)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UResourceAttributeSet,
        Resource,
        OldResource
    );
}

void UResourceAttributeSet::OnRep_MaxResource(
    const FGameplayAttributeData& OldMaxResource)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(
        UResourceAttributeSet,
        MaxResource,
        OldMaxResource
    );
}