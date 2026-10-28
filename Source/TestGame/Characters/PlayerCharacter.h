#pragma once

#include "CoreMinimal.h"
#include "GenericCharacter.h"

#include "../Items/ItemDefinition.h"

#include "PlayerCharacter.generated.h"

class ADamageNumberActor;
class UInventoryComponent;
class UPlayerClassDefinitions;
class UStaticMeshComponent;


UCLASS()
class TESTGAME_API APlayerCharacter : public AGenericCharacter
{
    GENERATED_BODY()

public:
    APlayerCharacter();

    UPROPERTY(
        EditDefaultsOnly,
        Category = "Combat Text"
    )
    TSubclassOf<ADamageNumberActor>
        HealingNumberActorClass;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Inventory"
    )
    TObjectPtr<UInventoryComponent>
        InventoryComponent;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Equipment|Visual"
    )
    TObjectPtr<UStaticMeshComponent>
        MainHandVisual;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Equipment|Visual"
    )
    TObjectPtr<UStaticMeshComponent>
        OffHandVisual;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Equipment|Animation"
    )
    EWeaponAnimationType
        CurrentWeaponAnimationType =
        EWeaponAnimationType::None;

    UFUNCTION(
        BlueprintPure,
        Category = "Equipment"
    )
    UItemDefinition* GetEquippedItemDefinition(
        EEquipmentSlot Slot
    ) const;

    UFUNCTION(
        BlueprintPure,
        Category = "Equipment|Animation"
    )
    EWeaponAnimationType
        GetCurrentWeaponAnimationType() const
    {
        return CurrentWeaponAnimationType;
    }

    void ApplyPlayerClassDefinition(
        const UPlayerClassDefinitions* ClassDefinition
    );

    void RemovePlayerClassDefinition(
        const UPlayerClassDefinitions* ClassDefinition
    );

protected:
    virtual void BeginPlay() override;

    virtual void OnDeathStarted() override;

private:
    void RefreshEquipmentVisuals();

    void RefreshWeaponVisual(
        EEquipmentSlot Slot,
        UStaticMeshComponent* VisualComponent
    );

    void ClearWeaponVisual(
        UStaticMeshComponent* VisualComponent
    );

    bool DoesMeshAttachmentPointExist(
        FName AttachmentPoint
    ) const;
};