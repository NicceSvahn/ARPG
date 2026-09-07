#include "GenericProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AGenericProjectile::AGenericProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

    // The server creates the projectile and replicates it to clients.
    bReplicates = true;
    SetReplicateMovement(true);

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);

    Collision->InitSphereRadius(16.0f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    Collision->OnComponentHit.AddDynamic(
        this,
        &AGenericProjectile::OnProjectileHit);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

    ProjectileMovement->UpdatedComponent = Collision;
    ProjectileMovement->InitialSpeed = 1200.0f;
    ProjectileMovement->MaxSpeed = 1200.0f;
    ProjectileMovement->ProjectileGravityScale = 0.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bInitialVelocityInLocalSpace = false;
    ProjectileMovement->bShouldBounce = false;
}

void AGenericProjectile::InitializeProjectile(
    UAbilitySystemComponent* InSourceASC,
    const FGameplayEffectSpecHandle& InEffectSpec,
    const FVector& InDirection)
{
    SourceASC = InSourceASC;
    EffectSpec = InEffectSpec;

    // Prevent immediate collision with the caster.
    if (AActor* OwnerActor = GetOwner())
    {
        Collision->IgnoreActorWhenMoving(OwnerActor, true);
    }

    if (AActor* InstigatorActor = GetInstigator())
    {
        Collision->IgnoreActorWhenMoving(InstigatorActor, true);
    }

    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;
    ProjectileMovement->bInitialVelocityInLocalSpace = false;

    const FVector LaunchDirection =
        InDirection.GetSafeNormal2D();

    if (LaunchDirection.IsNearlyZero())
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("PROJECTILE: Invalid launch direction"));

        Destroy();
        return;
    }

    SetActorRotation(LaunchDirection.Rotation());

    ProjectileMovement->Velocity =
        LaunchDirection *
        ProjectileMovement->InitialSpeed;

    ProjectileMovement->UpdateComponentVelocity();
    ProjectileMovement->Activate(true);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("PROJECTILE: Direction=%s Velocity=%s"),
        *LaunchDirection.ToString(),
        *ProjectileMovement->Velocity.ToString());

    SetLifeSpan(LifeSeconds);
}

bool AGenericProjectile::CanHitActor(const AActor* OtherActor) const
{
    return IsValid(OtherActor)
        && OtherActor != this
        && OtherActor != GetOwner()
        && OtherActor != GetInstigator();
}

void AGenericProjectile::OnProjectileHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (bHasImpacted || !CanHitActor(OtherActor))
    {
        return;
    }

    bHasImpacted = true;

    // Gameplay Effects must be applied by the server.
    if (HasAuthority())
    {
        HandleImpact(OtherActor, Hit);
    }

    Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProjectileMovement->StopMovementImmediately();

    Destroy();
}

void AGenericProjectile::HandleImpact(
    AActor* OtherActor,
    const FHitResult& Hit)
{
    UAbilitySystemComponent* Source = SourceASC.Get();

    UAbilitySystemComponent* TargetASC =
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
            OtherActor);

    if (!IsValid(Source) ||
        !IsValid(TargetASC) ||
        !EffectSpec.IsValid())
    {
        return;
    }

    EffectSpec.Data->GetContext().AddHitResult(Hit, true);

    Source->ApplyGameplayEffectSpecToTarget(
        *EffectSpec.Data.Get(),
        TargetASC);
}
