#include "PlayerHudWidget.h"

#include "HealthBarWidget.h"

void UPlayerHudWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetHealth(100.0f, 100.0f);
}

void UPlayerHudWidget::SetHealth(float CurrentHealth, float MaxHealth)
{
    if (!IsValid(WBP_PlayerHealthBar))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("%s: HealthBarWidget is not bound."),
            *GetNameSafe(this));

        return;
    }

    WBP_PlayerHealthBar->SetHealth(CurrentHealth, MaxHealth);
}



