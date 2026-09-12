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

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "AI|Combat",
		meta = (Categories = "Input.Ability")
	)
	FGameplayTag PrimaryAttackInputTag;

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
	float AggroRange = 1200.0f;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> EnemyHealthWidget;

	void RefreshHealthBar(float CurrentHealth);

	virtual void HandleAttributeChanged(FGameplayAttribute Attribute, float Magnitude, float NewHealth) override;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Combat Text"
	)
	TSubclassOf<ADamageNumberActor> DamageNumberActorClass;

private:
	void InitializeAggroTarget();
	void HandleDamageReceived(float DamageAmount);
};
