#pragma once

#include "CoreMinimal.h"
#include "GenericCharacter.h"
#include "GameplayTagContainer.h"
#include "EnemyCharacter.generated.h"

class USphereComponent;
class UAbilitySystemComponent;
class UHealthAttributeSet;
class UWidgetComponent;
class UEnemyHealthBarWidget;
class ADamageNumberActor;
class APlayerCharacter;
class UGameplayEffect;
class UPrimitiveComponent;

struct FTimerHandle;
struct FDamageResult;
struct FOnAttributeChangeData;
struct FHitResult;
class AEnemyCharacter;

DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnEnemyDied,
	AEnemyCharacter*
);

UCLASS()
class TESTGAME_API AEnemyCharacter : public AGenericCharacter
{
    GENERATED_BODY()

public:
    AEnemyCharacter();

    FGameplayTag GetPrimaryAttackInputTag() const
    {
        return PrimaryAttackInputTag;
    }

	FOnEnemyDied OnEnemyDied;

protected:
    virtual void BeginPlay() override;

    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason
    ) override;

    virtual void OnDeathStarted() override;

    // ELITE
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Enemy|Elite"
    )
    bool bIsElite = false;

    UFUNCTION(BlueprintPure)
    bool IsElite() const
    {
        return bIsElite;
    }

    UPROPERTY(
        EditDefaultsOnly,
        Category = "Abilities|Attribute"
    )
    TSubclassOf<UGameplayEffect> EliteModifierEffect;

    // ABILITIES
    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "AI|Combat",
        meta = (Categories = "Input.Ability")
    )
    FGameplayTag PrimaryAttackInputTag;

    // AGGRO
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "AI|Aggro"
    )
    TObjectPtr<USphereComponent> AggroSphere;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "AI|Aggro",
        meta = (
            ClampMin = "0.0",
            Units = "cm"
            )
    )
    float AggroRange = 700.0f;

    UPROPERTY(
        EditDefaultsOnly,
        BlueprintReadOnly,
        Category = "AI|Aggro",
        meta = (
            ClampMin = "0.0",
            Units = "s"
            )
    )
    float AggroDropDelay = 5.0f;

    UFUNCTION()
    void HandleAggroBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComponent,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void HandleAggroEndOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComponent,
        int32 OtherBodyIndex
    );

    UFUNCTION()
    APlayerCharacter* FindClosestPlayer() const;


    // UI
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "UI"
    )
    TObjectPtr<UWidgetComponent> EnemyHealthWidget;

    UPROPERTY(
        EditDefaultsOnly,
        Category = "Combat Text"
    )
    TSubclassOf<ADamageNumberActor> DamageNumberActorClass;


private:

    // AGGRO
    void InitializeAggroTarget();

    FTimerHandle AggroSearchTimer;
    FTimerHandle AggroDropTimerHandle;

    void EvaluateAggroAfterPlayerLeft();
    void HandleAggroDropTimerExpired();
    void CancelAggroDropTimer();
    void HandleDamageAggro(const FDamageResult& DamageResult);

    // HEALTH BAR
    void InitializeHealthBar();


    // COMBAT TEXT
    void HandleDamageResult(
        const FDamageResult& DamageResult
    );

    FDelegateHandle DamageResultHandle;

    UFUNCTION(
        NetMulticast,
        Unreliable
    )
    void MulticastShowDamageNumber(
        float DamageAmount,
        bool bCritical
    );

    void SpawnDamageNumber(
        float DamageAmount,
        bool bCritical
    );
};