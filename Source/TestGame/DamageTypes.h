// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DamageTypes.generated.h"

USTRUCT(BlueprintType)
struct FDamageSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CriticalMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> SourceActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanCrit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> SourceActor = nullptr;

};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTGAME_API UDamageTypes : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDamageTypes();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
