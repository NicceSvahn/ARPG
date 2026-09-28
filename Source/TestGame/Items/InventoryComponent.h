#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "ItemDefinition.h"
#include "InventoryComponent.generated.h"

class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);

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

    // The UI subscribes to this event.
    FOnInventoryChanged OnInventoryChanged;

    int32 GetInventoryCapacity() const
    {
        return InventoryCapacity;
    }

    int32 GetEquipmentIndex(EEquipmentSlot Slot) const
    {
        return InventoryCapacity + static_cast<int32>(Slot);
    }

    FInventoryItem GetItem(int32 Index) const;

    UFUNCTION(
        BlueprintCallable,
        BlueprintAuthorityOnly,
        Category = "Inventory"
    )
    bool AddItem(UItemDefinition* Definition);

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

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    TArray<TObjectPtr<UItemDefinition>> StartingItems;

    // Backpack slots first, followed by equipment slots.
    UPROPERTY(ReplicatedUsing = OnRep_Items)
    TArray<FInventoryItem> Items;

    UFUNCTION()
    void OnRep_Items();

    UFUNCTION(Server, Reliable)
    void ServerUseSlot(
        int32 Index,
        FGuid ExpectedItemId
    );

    UAbilitySystemComponent* GetASC() const;

    int32 FindEmptyInventorySlot() const;

    bool ApplyEquipmentEffect(
        UItemDefinition* Definition,
        FActiveGameplayEffectHandle& OutHandle
    );

    void NotifyChanged();

    // Server-only effect handles, one per equipment position.
    TArray<FActiveGameplayEffectHandle> EquipmentEffectHandles;
};
