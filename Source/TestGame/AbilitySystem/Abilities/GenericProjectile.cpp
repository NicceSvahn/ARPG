#include "GenericProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AGenericProjectile::AGenericProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

    // The server creates the projectile and replicates it to clients.
    bReplicates = true;
    SetReplicateMovement(true);

    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);

    Collision->InitSphereRadius(16.0f);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
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

void AGenericProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HasAuthority())
    {
        return;
    }

    const float DistanceTravelled =
        FVector::Dist(
            SpawnLocation,
            GetActorLocation()
        );

    if (DistanceTravelled >= MaxRange)
    {
        Destroy();
    }
}

void AGenericProjectile::InitializeProjectile(
    UAbilitySystemComponent* InSourceASC,
    const FGameplayEffectSpecHandle& InEffectSpec,
    const FVector& InLaunchDirection)
{
    SourceASC = InSourceASC;
    EffectSpec = InEffectSpec;

    if (InSourceASC)
    {
        SourceActor = InSourceASC->GetAvatarActor();
    }

    if (AActor* Source = SourceActor.Get())
    {
        Collision->IgnoreActorWhenMoving(
            Source,
            true
        );

        if (UPrimitiveComponent* SourceRoot =
            Cast<UPrimitiveComponent>(
                Source->GetRootComponent()
            ))
        {
            SourceRoot->IgnoreActorWhenMoving(
                this,
                true
            );
        }
    }

    // Prevent immediate collision with the caster.
    if (AActor* OwnerActor = GetOwner())
    {
        Collision->IgnoreActorWhenMoving(
            OwnerActor,
            true
        );
    }

    if (AActor* InstigatorActor = GetInstigator())
    {
        Collision->IgnoreActorWhenMoving(
            InstigatorActor,
            true
        );
    }

    SpawnLocation = GetActorLocation();

    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    const FVector LaunchDirection =
        InLaunchDirection.GetSafeNormal2D();

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
    if (!IsValid(OtherActor))
    {
        return false;
    }

    if (OtherActor == this)
    {
        return false;
    }

    if (OtherActor == SourceActor.Get())
    {
        return false;
    }

    if (OtherActor == GetOwner())
    {
        return false;
    }

    if (OtherActor == GetInstigator())
    {
        return false;
    }

    return true;
}

void AGenericProjectile::OnProjectileHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (!HasAuthority() || bHasImpacted)
    {
        return;
    }

    // Ignore source without consuming projectile.
    if (OtherActor == SourceActor.Get() ||
        OtherActor == GetOwner() ||
        OtherActor == GetInstigator())
    {
        return;
    }

    bHasImpacted = true;

    Collision->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    ProjectileMovement->StopMovementImmediately();
    ProjectileMovement->Deactivate();

    // Only apply damage if this is a valid damage target.
    if (CanHitActor(OtherActor))
    {
        HandleImpact(
            OtherActor,
            Hit
        );
    }

    // Any other real impact consumes the projectile.
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
