#include "PlayerCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "../Game/TestGameGameState.h"
#include "../Items/InventoryComponent.h"
#include "PlayerClassDefinitions.h"


APlayerCharacter::APlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    bUseControllerRotationYaw = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate =
        FRotator(
            0.0f,
            720.0f,
            0.0f
        );

    InventoryComponent =
        CreateDefaultSubobject<UInventoryComponent>(
            TEXT("InventoryComponent")
        );

    MainHandVisual =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("MainHandVisual")
        );

    MainHandVisual->SetupAttachment(
        GetMesh()
    );

    MainHandVisual->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    MainHandVisual->SetGenerateOverlapEvents(
        false
    );

    MainHandVisual->SetVisibility(
        false,
        true
    );

    OffHandVisual =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("OffHandVisual")
        );

    OffHandVisual->SetupAttachment(
        GetMesh()
    );

    OffHandVisual->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    OffHandVisual->SetGenerateOverlapEvents(
        false
    );

    OffHandVisual->SetVisibility(
        false,
        true
    );
}


void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (InventoryComponent)
    {
        InventoryComponent
            ->OnEquipmentPresentationChanged
            .AddUObject(
                this,
                &APlayerCharacter::RefreshEquipmentVisuals
            );
    }

    RefreshEquipmentVisuals();
}


UItemDefinition*
APlayerCharacter::GetEquippedItemDefinition(
    EEquipmentSlot Slot) const
{
    if (!InventoryComponent)
    {
        return nullptr;
    }

    return InventoryComponent
        ->GetEquippedDefinition(
            Slot
        );
}


void APlayerCharacter::RefreshEquipmentVisuals()
{
    RefreshWeaponVisual(
        EEquipmentSlot::MainHand,
        MainHandVisual
    );

    RefreshWeaponVisual(
        EEquipmentSlot::OffHand,
        OffHandVisual
    );

    const UItemDefinition* MainHandDefinition =
        GetEquippedItemDefinition(
            EEquipmentSlot::MainHand
        );

    CurrentWeaponAnimationType =
        MainHandDefinition
        ? MainHandDefinition
        ->EquipmentVisual
        .WeaponAnimationType
        : EWeaponAnimationType::None;
}


void APlayerCharacter::RefreshWeaponVisual(
    EEquipmentSlot Slot,
    UStaticMeshComponent* VisualComponent)
{
    if (!VisualComponent)
    {
        return;
    }

    UItemDefinition* Definition =
        GetEquippedItemDefinition(
            Slot
        );

    if (
        !IsValid(Definition) ||
        !IsValid(
            Definition
            ->EquipmentVisual
            .StaticMesh
        )
        )
    {
        ClearWeaponVisual(
            VisualComponent
        );

        return;
    }

    const FEquipmentVisualData& Visual =
        Definition->EquipmentVisual;

    FName AttachmentPoint =
        NAME_None;

    switch (Slot)
    {
    case EEquipmentSlot::MainHand:
    {
        AttachmentPoint =
            Visual.MainHandSocket;

        break;
    }

    case EEquipmentSlot::OffHand:
    {
        AttachmentPoint =
            Visual.OffHandSocket;

        break;
    }

    default:
    {
        ClearWeaponVisual(
            VisualComponent
        );

        return;
    }
    }

    if (
        AttachmentPoint.IsNone() ||
        !DoesMeshAttachmentPointExist(
            AttachmentPoint
        )
        )
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "[EQUIPMENT] Item '%s' has invalid attachment point '%s'"
            ),
            *GetNameSafe(Definition),
            *AttachmentPoint.ToString()
        );

        ClearWeaponVisual(
            VisualComponent
        );

        return;
    }

    VisualComponent->AttachToComponent(
        GetMesh(),
        FAttachmentTransformRules::
        SnapToTargetNotIncludingScale,
        AttachmentPoint
    );

    VisualComponent->SetStaticMesh(
        Visual.StaticMesh
    );

    VisualComponent->SetRelativeTransform(
        Visual.RelativeTransform
    );

    VisualComponent->SetVisibility(
        true,
        true
    );

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[EQUIPMENT] Equipped visual '%s' using mesh '%s' on '%s'"
        ),
        *GetNameSafe(Definition),
        *GetNameSafe(
            Visual.StaticMesh
        ),
        *AttachmentPoint.ToString()
    );
}


void APlayerCharacter::ClearWeaponVisual(
    UStaticMeshComponent* VisualComponent)
{
    if (!VisualComponent)
    {
        return;
    }

    VisualComponent->SetStaticMesh(
        nullptr
    );

    VisualComponent->SetVisibility(
        false,
        true
    );
}


bool APlayerCharacter::DoesMeshAttachmentPointExist(
    FName AttachmentPoint) const
{
    const USkeletalMeshComponent* CharacterMesh =
        GetMesh();

    if (
        !CharacterMesh ||
        AttachmentPoint.IsNone()
        )
    {
        return false;
    }

    /*
     * Unreal allows attachment to both sockets and bones.
     *
     * weapon_r / weapon_l in Manny's skeleton are bones,
     * so checking DoesSocketExist() alone is too restrictive.
     */
    const bool bIsSocket =
        CharacterMesh->DoesSocketExist(
            AttachmentPoint
        );

    const bool bIsBone =
        CharacterMesh->GetBoneIndex(
            AttachmentPoint
        ) != INDEX_NONE;

    return
        bIsSocket ||
        bIsBone;
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
        ? GetWorld()
        ->GetGameState<ATestGameGameState>()
        : nullptr;

    if (!GameState)
    {
        return;
    }

    GameState->SetLevelState(
        ELevelState::Failed
    );
}


void APlayerCharacter::ApplyPlayerClassDefinition(
    const UPlayerClassDefinitions* ClassDefinition)
{
    if (
        !HasAuthority() ||
        !ClassDefinition
        )
    {
        return;
    }

    GrantAbilities(
        ClassDefinition->Abilities
    );
}


void APlayerCharacter::RemovePlayerClassDefinition(
    const UPlayerClassDefinitions* ClassDefinition)
{
    if (
        !HasAuthority() ||
        !ClassDefinition
        )
    {
        return;
    }

    RemoveAbilities(
        ClassDefinition->Abilities
    );
}