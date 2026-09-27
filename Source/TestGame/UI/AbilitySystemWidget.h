#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemWidget.generated.h"

class UAbilitySystemComponent;

UCLASS(Abstract)
class TESTGAME_API UAbilitySystemWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Ability System")
    virtual void InitializeFromActor(AActor* InActor);

protected:
    virtual void NativeDestruct() override;

    /**
     * Called after InitializeFromActor has successfully resolved
     * the actor's AbilitySystemComponent.
     *
     * Derived widgets bind their GAS delegates here.
     */
    virtual void OnAbilitySystemReady();

    /**
     * Derived widgets remove their GAS delegates here.
     */
    virtual void UnbindFromAbilitySystem();

    UAbilitySystemComponent* GetObservedAbilitySystem() const;

    AActor* GetObservedActor() const;

private:
    TWeakObjectPtr<AActor> ObservedActor;

    TWeakObjectPtr<UAbilitySystemComponent> ObservedAbilitySystem;
};