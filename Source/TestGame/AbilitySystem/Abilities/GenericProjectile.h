#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GenericProjectile.generated.h"

UCLASS(Abstract)
class TESTGAME_API AGenericProjectile : public AActor
{
	GENERATED_BODY()

public:
	AGenericProjectile();

	void InitializeProjectile(
		UAbilitySystemComponent* InSourceASC,
		const FGameplayEffectSpecHandle& InEffectSpec,
		const FVector& InLaunchDirection);

protected:
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Projectile",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float LifeSeconds = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float MaxRange = 1800.0f;

	UFUNCTION()
	virtual void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse,
		const FHitResult& Hit);

	virtual bool CanHitActor(const AActor* OtherActor) const;
	virtual void HandleImpact(AActor* OtherActor, const FHitResult& Hit);

	virtual void Tick(float DeltaTime) override;

private:
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;
	FGameplayEffectSpecHandle EffectSpec;

	bool bHasImpacted = false;

	TWeakObjectPtr<AActor> SourceActor;

	FVector SpawnLocation = FVector::ZeroVector;

};
