#include "PlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "../Game/TestGameGameState.h"
#include "PlayerClassDefinitions.h"

APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    bUseControllerRotationYaw = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
}

void APlayerCharacter::OnDeathStarted()
{
    Super::OnDeathStarted();

    if (!HasAuthority())
    {
        return;
    }

    ATestGameGameState* GameState =
        GetWorld()
        ? GetWorld()->GetGameState<ATestGameGameState>()
        : nullptr;

    if (!GameState)
    {
        return;
    }

    GameState->SetLevelState(ELevelState::Failed);
}

void APlayerCharacter::ApplyPlayerClassDefinition(
    const UPlayerClassDefinitions* ClassDefinition)
{
    if (!HasAuthority() || !ClassDefinition)
    {
        return;
    }

    GrantAbilities(ClassDefinition->Abilities);
}

void APlayerCharacter::RemovePlayerClassDefinition(
    const UPlayerClassDefinitions* ClassDefinition)
{
    if (!HasAuthority() || !ClassDefinition)
    {
        return;
    }

    RemoveAbilities(ClassDefinition->Abilities);
}
