#include "BuffBarWidget.h"

#include "BuffDisplayDataAsset.h"
#include "BuffIconWidget.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"

#include "Components/HorizontalBox.h"
#include "TimerManager.h"


void UBuffBarWidget::OnAbilitySystemReady()
{
    UAbilitySystemComponent* ASC =
        GetObservedAbilitySystem();

    if (!ASC)
    {
        return;
    }

    if (!BuffDisplayData)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("BuffBar: BuffDisplayData is null")
        );

        return;
    }


    // Register for every buff defined in the DataAsset.
    // BuffBar itself knows nothing about Warcry, Mend, etc.
    for (const FBuffDisplayData& DisplayData :
        BuffDisplayData->Buffs)
    {
        if (!DisplayData.BuffTag.IsValid())
        {
            continue;
        }

        const FGameplayTag BuffTag =
            DisplayData.BuffTag;

        const FDelegateHandle Handle =
            ASC->RegisterGameplayTagEvent(
                BuffTag,
                EGameplayTagEventType::NewOrRemoved
            )
            .AddUObject(
                this,
                &UBuffBarWidget::HandleBuffTagChanged
            );

        BuffTagDelegateHandles.Add(
            BuffTag,
            Handle
        );
    }


    // Covers case where HUD initializes while
    // one or more buffs are already active.
    RefreshExistingBuffs();


    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            DurationUpdateTimer,
            this,
            &UBuffBarWidget::UpdateBuffDurations,
            0.25f,
            true
        );
    }

}


void UBuffBarWidget::UnbindFromAbilitySystem()
{
    UAbilitySystemComponent* ASC =
        GetObservedAbilitySystem();

    if (ASC)
    {
        for (const TPair<
            FGameplayTag,
            FDelegateHandle>& Pair :
            BuffTagDelegateHandles)
        {
            if (!Pair.Value.IsValid())
            {
                continue;
            }

            ASC->RegisterGameplayTagEvent(
                Pair.Key,
                EGameplayTagEventType::NewOrRemoved
            )
                .Remove(
                    Pair.Value
                );
        }
    }

    BuffTagDelegateHandles.Reset();


    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(
            DurationUpdateTimer
        );
    }


    ActiveBuffWidgets.Reset();

    if (BuffContainer)
    {
        BuffContainer->ClearChildren();
    }
}


void UBuffBarWidget::HandleBuffTagChanged(
    FGameplayTag Tag,
    int32 NewCount)
{


    if (NewCount > 0)
    {
        AddBuff(Tag);
    }
    else
    {
        RemoveBuff(Tag);
    }
}


void UBuffBarWidget::AddBuff(
    const FGameplayTag& BuffTag)
{
    if (ActiveBuffWidgets.Contains(BuffTag))
    {
        return;
    }

    if (
        !BuffContainer ||
        !BuffIconWidgetClass ||
        !BuffDisplayData
        )
    {
        return;
    }


    const FBuffDisplayData* DisplayData =
        BuffDisplayData->FindBuff(
            BuffTag
        );

    if (!DisplayData)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "BuffBar: No display data found for %s"
            ),
            *BuffTag.ToString()
        );

        return;
    }


    UBuffIconWidget* BuffWidget =
        CreateWidget<UBuffIconWidget>(
            GetOwningPlayer(),
            BuffIconWidgetClass
        );

    if (!BuffWidget)
    {
        return;
    }


    BuffWidget->InitializeBuff(
        DisplayData->DisplayName,
        DisplayData->Icon
    );


    BuffContainer->AddChild(
        BuffWidget
    );


    ActiveBuffWidgets.Add(
        BuffTag,
        BuffWidget
    );


    BuffWidget->SetRemainingSeconds(
        GetRemainingTimeForBuff(
            BuffTag
        )
    );


}


void UBuffBarWidget::RemoveBuff(
    const FGameplayTag& BuffTag)
{
    TObjectPtr<UBuffIconWidget>* FoundWidget =
        ActiveBuffWidgets.Find(
            BuffTag
        );

    if (!FoundWidget || !(*FoundWidget))
    {
        return;
    }


    (*FoundWidget)->RemoveFromParent();

    ActiveBuffWidgets.Remove(
        BuffTag
    );


}


void UBuffBarWidget::RefreshExistingBuffs()
{
    UAbilitySystemComponent* ASC =
        GetObservedAbilitySystem();

    if (!ASC || !BuffDisplayData)
    {
        return;
    }


    for (const FBuffDisplayData& DisplayData :
        BuffDisplayData->Buffs)
    {
        if (!DisplayData.BuffTag.IsValid())
        {
            continue;
        }


        if (
            ASC->HasMatchingGameplayTag(
                DisplayData.BuffTag
            )
            )
        {
            AddBuff(
                DisplayData.BuffTag
            );
        }
    }
}


float UBuffBarWidget::GetRemainingTimeForBuff(
    const FGameplayTag& BuffTag) const
{
    UAbilitySystemComponent* ASC =
        GetObservedAbilitySystem();

    if (!ASC)
    {
        return -1.0f;
    }


    FGameplayTagContainer Tags;
    Tags.AddTag(
        BuffTag
    );


    const FGameplayEffectQuery Query =
        FGameplayEffectQuery::
        MakeQuery_MatchAnyOwningTags(
            Tags
        );


    const TArray<float> RemainingTimes =
        ASC->GetActiveEffectsTimeRemaining(
            Query
        );


    if (RemainingTimes.IsEmpty())
    {
        return -1.0f;
    }


    float LongestRemainingTime =
        -1.0f;


    for (const float RemainingTime :
    RemainingTimes)
    {
        LongestRemainingTime =
            FMath::Max(
                LongestRemainingTime,
                RemainingTime
            );
    }


    return LongestRemainingTime;
}


void UBuffBarWidget::UpdateBuffDurations()
{
    for (TPair<
        FGameplayTag,
        TObjectPtr<UBuffIconWidget>>&Pair :
        ActiveBuffWidgets)
    {
        if (!Pair.Value)
        {
            continue;
        }


        const float RemainingTime =
            GetRemainingTimeForBuff(
                Pair.Key
            );


        Pair.Value->SetRemainingSeconds(
            RemainingTime
        );
    }
}