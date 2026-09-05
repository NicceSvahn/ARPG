// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericCharacter.h"
#include "../AI/EnemyAIController.h"
#include "EnemyCharacter.generated.h"

class UAbilitySystemComponent;
class UHealthAttributeSet;
class UWidgetComponent;
class UEnemyHealthBarWidget;

UCLASS()
class TESTGAME_API AEnemyCharacter : public AGenericCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> EnemyHealthWidget;

	void RefreshHealthBar(float CurrentHealth);

	virtual void HandleAttributeChanged(FGameplayAttribute Attribute, float Magnitude, float NewHealth) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
