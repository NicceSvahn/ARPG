#pragma once

#include "CoreMinimal.h"
#include "GenericCharacter.h"
#include "../AI/EnemyAIController.h"
#include "EnemyCharacter.generated.h"

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

	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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

	void HandleDamageReceived(float DamageAmount);
};
