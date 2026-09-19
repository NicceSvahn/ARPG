#include "EnemyCharacter.h"

#include "../AI/EnemyAIController.h"
#include "../UI/EnemyHealthBarWidget.h"
#include "../UI/FloatingCombatText/DamageNumberActor.h"

#include "TestGame/AbilitySystem/TestGameAbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "TestGame/AbilitySystem/Attributes/HealthAttributeSet.h"
#include "TestGame/Characters/PlayerCharacter.h"


// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    SetReplicateMovement(true);

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    AggroSphere =
        CreateDefaultSubobject<USphereComponent>(
            TEXT("AggroSphere")
        );

    AggroSphere->SetupAttachment(RootComponent);

    AggroSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AggroSphere->SetCollisionObjectType(ECC_WorldDynamic);
    AggroSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    AggroSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    AggroSphere->SetGenerateOverlapEvents(true);

    AggroSphere->OnComponentBeginOverlap.AddDynamic(
        this,
        &AEnemyCharacter::HandleAggroBeginOverlap
    );

    AggroSphere->OnComponentEndOverlap.AddDynamic(
        this,
        &AEnemyCharacter::HandleAggroEndOverlap
    );

	EnemyHealthWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyHealthWidget"));

	EnemyHealthWidget->SetupAttachment(RootComponent);
	EnemyHealthWidget->SetWidgetSpace(EWidgetSpace::Screen);
	EnemyHealthWidget->SetDrawAtDesiredSize(true);

    EnemyHealthWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));


}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();	

    if (AggroSphere)
    {
        AggroSphere->SetSphereRadius(
            AggroRange,
            true
        );
    }

    OnDamageReceived.AddUObject(
        this,
        &AEnemyCharacter::HandleDamageReceived
    );

    if (AbilitySystemComponent)
    {
        HealthChangedHandle =
            AbilitySystemComponent
            ->GetGameplayAttributeValueChangeDelegate(
                UHealthAttributeSet::GetHealthAttribute()
            )
            .AddUObject(
                this,
                &AEnemyCharacter::HandleHealthChanged
            );

        MaxHealthChangedHandle =
            AbilitySystemComponent
            ->GetGameplayAttributeValueChangeDelegate(
                UHealthAttributeSet::GetMaxHealthAttribute()
            )
            .AddUObject(
                this,
                &AEnemyCharacter::HandleMaxHealthChanged
            );
    }

    if (HealthAttributeSet)
    {
        RefreshHealthBar(HealthAttributeSet->GetHealth());
    }

    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(
            AggroSearchTimer,
            this,
            &AEnemyCharacter::InitializeAggroTarget,
            0.5f,
            true
        );
    }
}

void AEnemyCharacter::InitializeAggroTarget()
{
    if (!HasAuthority())
    {
        return;
    }

    if (!AggroSphere)
    {
        return;
    }

    AggroSphere->UpdateOverlaps();

    APlayerCharacter* PlayerCharacter =
        FindClosestPlayer();

    if (!IsValid(PlayerCharacter))
    {
        return;
    }

    AEnemyAIController* EnemyController =
        Cast<AEnemyAIController>(
            GetController()
        );

    if (!EnemyController)
    {
        return;
    }

    EnemyController->SetAggroTarget(
        PlayerCharacter
    );
}

void AEnemyCharacter::HandleAggroBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
        return;
    }

    APlayerCharacter* EnteringPlayer =
        Cast<APlayerCharacter>(
            OtherActor
        );

    if (!IsValid(EnteringPlayer))
    {
        return;
    }

    InitializeAggroTarget();
}

void AEnemyCharacter::HandleAggroEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    int32 OtherBodyIndex)
{
    if (!HasAuthority())
    {
        return;
    }

    APlayerCharacter* LeavingPlayer =
        Cast<APlayerCharacter>(
            OtherActor
        );

    if (!IsValid(LeavingPlayer))
    {
        return;
    }

    AEnemyAIController* EnemyController =
        Cast<AEnemyAIController>(
            GetController()
        );

    if (!EnemyController)
    {
        return;
    }

    EnemyController->ClearAggroTarget(
        LeavingPlayer
    );

    GetWorldTimerManager().SetTimerForNextTick(
        this,
        &AEnemyCharacter::InitializeAggroTarget
    );
}

APlayerCharacter*
AEnemyCharacter::FindClosestPlayer() const
{
    if (!AggroSphere)
    {
        return nullptr;
    }

    TArray<AActor*> OverlappingActors;

    AggroSphere->GetOverlappingActors(
        OverlappingActors,
        APlayerCharacter::StaticClass()
    );

    const FGameplayTag DeadStateTag =
        FGameplayTag::RequestGameplayTag(
            FName(TEXT("State.Dead"))
        );

    APlayerCharacter* ClosestPlayer =
        nullptr;

    float ClosestDistanceSquared =
        TNumericLimits<float>::Max();

    const FVector EnemyLocation =
        GetActorLocation();

    for (AActor* OverlappingActor :
        OverlappingActors)
    {
        APlayerCharacter* PlayerCharacter =
            Cast<APlayerCharacter>(
                OverlappingActor
            );

        if (!IsValid(PlayerCharacter))
        {
            continue;
        }

        UTestGameAbilitySystemComponent*
            PlayerAbilitySystem =
            PlayerCharacter
            ->GetAbilitySystemComponent();

        if (!PlayerAbilitySystem)
        {
            continue;
        }

        const bool bPlayerIsDead =
            PlayerAbilitySystem
            ->HasMatchingGameplayTag(
                DeadStateTag
            );

        if (bPlayerIsDead)
        {
            continue;
        }

        const float DistanceSquared =
            FVector::DistSquared(
                EnemyLocation,
                PlayerCharacter
                ->GetActorLocation()
            );

        if (DistanceSquared <
            ClosestDistanceSquared)
        {
            ClosestDistanceSquared =
                DistanceSquared;

            ClosestPlayer =
                PlayerCharacter;
        }
    }

    return ClosestPlayer;
}

void AEnemyCharacter::RefreshHealthBar(
    float CurrentHealth)
{
    if (!EnemyHealthWidget)
    {
        return;
    }

    UEnemyHealthBarWidget* HealthBarWidget =
        Cast<UEnemyHealthBarWidget>(
            EnemyHealthWidget->GetUserWidgetObject()
        );

    if (!HealthBarWidget)
    {
        return;
    }

    HealthBarWidget->SetHealth(
        CurrentHealth,
        GetMaxHealth()
    );
}

void AEnemyCharacter::HandleDamageReceived(
    float DamageAmount)
{
    if (!HasAuthority())
    {
        return;
    }

    MulticastShowDamageNumber(
        DamageAmount
    );
}

void AEnemyCharacter::OnDeathStarted()
{
    Super::OnDeathStarted();

    if (AggroSphere)
    {
        AggroSphere->SetCollisionEnabled(
            ECollisionEnabled::NoCollision);
    }

    if (EnemyHealthWidget)
    {
        EnemyHealthWidget->SetVisibility(false);
    }

    if (AEnemyAIController*
        EnemyController =
        Cast<AEnemyAIController>(
            GetController()))
    {
        EnemyController
            ->HandleControlledPawnDeath();
    }
}

void AEnemyCharacter::HandleHealthChanged(
    const FOnAttributeChangeData& Data)
{
    RefreshHealthBar(
        Data.NewValue
    );
}

void AEnemyCharacter::HandleMaxHealthChanged(
    const FOnAttributeChangeData& Data)
{
    RefreshHealthBar(
        GetCurrentHealth()
    );
}

void AEnemyCharacter::MulticastShowDamageNumber_Implementation(
    float DamageAmount)
{
    SpawnDamageNumber(
        DamageAmount
    );
}

void AEnemyCharacter::SpawnDamageNumber(
    float DamageAmount)
{
    if (!DamageNumberActorClass)
    {
        return;
    }

    UWorld* World = GetWorld();

    if (!World)
    {
        return;
    }

    const FVector SpawnLocation =
        GetActorLocation() +
        FVector(
            0.0f,
            0.0f,
            120.0f
        );

    ADamageNumberActor* DamageNumber =
        World->SpawnActor<ADamageNumberActor>(
            DamageNumberActorClass,
            SpawnLocation,
            FRotator::ZeroRotator
        );

    if (!DamageNumber)
    {
        return;
    }

    DamageNumber->InitializeDamage(
        DamageAmount
    );
}