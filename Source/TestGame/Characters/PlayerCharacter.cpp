#include "PlayerCharacter.h"

#include "../UI/HealthBarWidget.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TestGame/AbilitySystem/Attributes/HealthAttributeSet.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    bUseControllerRotationYaw = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void APlayerCharacter::HandleAttributeChanged(
    FGameplayAttribute Attribute,
    float Magnitude,
    float NewValue
)
{
    Super::HandleAttributeChanged(Attribute, Magnitude, NewValue);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PLAYER HEALTH CHANGED -> %f"),
        NewValue
    );

    if (Attribute == UHealthAttributeSet::GetHealthAttribute())
    {
        return;
    }

    return;
}
