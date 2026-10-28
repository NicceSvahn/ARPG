#include "CombatDebugWidget.h"

#include "Components/TextBlock.h"

#include "../Characters/GenericCharacter.h"
#include "../Player/TestGamePlayerController.h"

void UCombatDebugWidget::InitializeDebugWidget(
    AGenericCharacter* InPlayerCharacter)
{
    PlayerCharacter = InPlayerCharacter;

    RefreshDebugInfo();
}

void UCombatDebugWidget::NativeTick(
    const FGeometry& MyGeometry,
    float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    RefreshDebugInfo();
}

void UCombatDebugWidget::RefreshDebugInfo()
{
    if (!PlayerCharacter)
    {
        return;
    }

    if (PlayerHealthText)
    {
        PlayerHealthText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("Health: %.0f / %.0f"),
                    PlayerCharacter->GetCurrentHealth(),
                    PlayerCharacter->GetMaxHealth()
                )
            )
        );
    }

    ATestGamePlayerController* Controller =
        Cast<ATestGamePlayerController>(
            GetOwningPlayer()
        );

    AGenericCharacter* TargetCharacter =
        Controller
        ? Controller->GetCharacterUnderCursor()
        : nullptr;

    if (TargetCharacter == PlayerCharacter)
    {
        TargetCharacter = nullptr;
    }

    if (PlayerResourceText)
    {
        PlayerResourceText->SetText(
            FText::FromString(TEXT("Resource: N/A"))
        );
    }

    if (!TargetCharacter)
    {
        if (TargetActorText)
        {
            TargetActorText->SetText(
                FText::FromString(TEXT("Actor: None"))
            );
        }

        if (TargetHealthText)
        {
            TargetHealthText->SetText(
                FText::FromString(TEXT("Health: -"))
            );
        }

        if (TargetDistanceText)
        {
            TargetDistanceText->SetText(
                FText::FromString(TEXT("Distance: -"))
            );
        }

        return;
    }

    if (TargetActorText)
    {
        TargetActorText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("Actor: %s"),
                    *GetNameSafe(TargetCharacter)
                )
            )
        );
    }

    if (TargetHealthText)
    {
        TargetHealthText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("Health: %.0f / %.0f"),
                    TargetCharacter->GetCurrentHealth(),
                    TargetCharacter->GetMaxHealth()
                )
            )
        );
    }

    if (TargetDistanceText)
    {
        const float Distance =
            FVector::Dist(
                PlayerCharacter->GetActorLocation(),
                TargetCharacter->GetActorLocation()
            );

        TargetDistanceText->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("Distance: %.0f"),
                    Distance
                )
            )
        );
    }
}