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
struct FTimerHandle;

UCLASS()
class TESTGAME_API AEnemyCharacter : public AGenericCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();

	FGameplayTag GetPrimaryAttackInputTag() const
	{
		return PrimaryAttackInputTag;
	}

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnDeathStarted() override;

	//Elite
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

	//Ability
	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "AI|Combat",
		meta = (Categories = "Input.Ability")
	)
	FGameplayTag PrimaryAttackInputTag;

	//Aggro
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
		meta = (ClampMin = "0.0",
			NoGetter = "cm")
	)
	float AggroRange = 700.0f;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "AI|Aggro",
		meta = (ClampMin = "0.0", Units = "s")
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

	//Enemy healthbar
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> EnemyHealthWidget;

	void RefreshHealthBar(float CurrentHealth);

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Combat Text"
	)
	TSubclassOf<ADamageNumberActor> DamageNumberActorClass;

private:
	//Aggro
	void InitializeAggroTarget();

	FTimerHandle AggroSearchTimer;
	FTimerHandle AggroDropTimerHandle;

	void EvaluateAggroAfterPlayerLeft();
	void HandleAggroDropTimerExpired();
	void CancelAggroDropTimer();

	//rest
	void HandleDamageReceived(
		float DamageAmount
	);

	UFUNCTION(
		NetMulticast,
		Unreliable
	)
	void MulticastShowDamageNumber(
		float DamageAmount
	);

	void SpawnDamageNumber(
		float DamageAmount
	);

	void HandleHealthChanged(
		const FOnAttributeChangeData& Data
	);

	void HandleMaxHealthChanged(
		const FOnAttributeChangeData& Data
	);

	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
};
