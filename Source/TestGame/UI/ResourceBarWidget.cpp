#include "ResourceBarWidget.h"

#include "Components/ProgressBar.h"

void UResourceBarWidget::SetResource(
    float CurrentResource,
    float MaxResource)
{
    if (!ResourceBar)
    {
        return;
    }

    const float Percent =
        MaxResource > 0.0f
        ? CurrentResource / MaxResource
        : 0.0f;

    ResourceBar->SetPercent(
        FMath::Clamp(
            Percent,
            0.0f,
            1.0f
        )
    );
}