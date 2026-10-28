#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpec.h"

#include "ItemDefinition.h"

#include "InventoryComponent.generated.h"

class UAbilitySystemComponent;


DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);

DECLARE_MULTICAST_DELEGATE(
    FOnEquipmentPresentationChanged
);


UCLASS(
    ClassGroup = (Inventory),
    meta = (BlueprintSpawnableComponent)
)
class TESTGAME_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    // Inventory UI subscribes to this event.
    FOnInventoryChanged OnInventoryChanged;

    /**
     * Character presentation subscribes to this.
     *
     * Fires when publicly visible equipment changes.
     */
    FOnEquipmentPresentationChanged
        OnEquipmentPresentationChanged;

    int32 GetInventoryCapacity() const
    {
        return InventoryCapacity;
    }

    int32 GetEquipmentIndex(
        EEquipmentSlot Slot) const
    {
        return
            InventoryCapacity +
            static_cast<int32>(Slot);
    }

    FInventoryItem GetItem(
        int32 Index
    ) const;

    /**
     * Returns the item definition currently equipped
     * in a given slot.
     *
     * Unlike GetItem(), this works for remote players too,
     * because EquippedDefinitions is replicated publicly.
     */
    UFUNCTION(
        BlueprintPure,
        Category = "Inventory|Equipment"
    )
    UItemDefinition* GetEquippedDefinition(
        EEquipmentSlot Slot
    ) const;

    UFUNCTION(
        BlueprintCallable,
        BlueprintAuthorityOnly,
        Category = "Inventory"
    )
    bool AddItem(
        UItemDefinition* Definition
    );

    void RequestUseSlot(
        int32 Index,
        FGuid ExpectedItemId
    );

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason
    ) override;

private:
    UPROPERTY(
        EditDefaultsOnly,
        Category = "Inventory",
        meta = (ClampMin = "1")
    )
    int32 InventoryCapacity = 60;

    UPROPERTY(
        EditDefaultsOnly,
        Category = "Inventory"
    )
    TArray<TObjectPtr<UItemDefinition>>
        StartingItems;

    /**
     * Private inventory.
     *
     * Backpack slots first, followed by equipment slots.
     *
     * Only the owning player receives this array.
     */
    UPROPERTY(
        ReplicatedUsing = OnRep_Items
    )
    TArray<FInventoryItem> Items;

    /**
     * Public equipment presentation state.
     *
     * One entry per EEquipmentSlot.
     *
     * Everyone receives this so remote clients can render
     * the correct equipment without receiving the full inventory.
     */
    UPROPERTY(
        ReplicatedUsing = OnRep_EquippedDefinitions
    )
    TArray<TObjectPtr<UItemDefinition>>
        EquippedDefinitions;

    UFUNCTION()
    void OnRep_Items();

    UFUNCTION()
    void OnRep_EquippedDefinitions();

    UFUNCTION(Server, Reliable)
    void ServerUseSlot(
        int32 Index,
        FGuid ExpectedItemId
    );

    UAbilitySystemComponent* GetASC() const;

    int32 FindEmptyInventorySlot() const;

    bool ApplyEquipmentEffect(
        const FInventoryItem& Item,
        FActiveGameplayEffectHandle& OutHandle
    );

    /**
     * Server-side.
     *
     * Converts the authoritative equipment section of Items
     * into the lightweight public equipment state.
     */
    void RebuildEquipmentPresentationState();

    void NotifyChanged();

    // Server-only effect handles, one per equipment position.
    TArray<FActiveGameplayEffectHandle>
        EquipmentEffectHandles;

    void GrantAndActivateEquipmentAbility(
        const FInventoryItem& Item,
        int32 EquipmentSlotIndex
    );

    void RemoveEquipmentAbility(
        int32 EquipmentSlotIndex
    );

    // Server-only ability handles, one per equipment position.
    TArray<FGameplayAbilitySpecHandle>
        EquipmentAbilityHandles;

    // Prevent ability callbacks from starting another equipment
    // change while the current change is still running.
    bool bChangingEquipment = false;
};