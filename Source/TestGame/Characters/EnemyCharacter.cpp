#include "EnemyCharacter.h"

#include "../AI/EnemyAIController.h"
#include "../UI/EnemyHealthBarWidget.h"
#include "../UI/FloatingCombatText/DamageNumberActor.h"

#include "TestGame/AbilitySystem/TestGameAbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameplayEffectExtension.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Controller.h"
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

        if (bIsElite)
        {
            AGenericCharacter::ApplyAttributeEffect(EliteModifierEffect);
        }
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

    if (AEnemyAIController* EnemyController =
        Cast<AEnemyAIController>(GetController()))
    {
        if (UBlackboardComponent* Blackboard =
            EnemyController->GetBlackboardComponent())
        {
            APlayerCharacter* CurrentTarget = Cast<APlayerCharacter>(
                Blackboard->GetValueAsObject(TEXT("TargetActor")));
            if (IsValid(CurrentTarget) && !CurrentTarget->bIsDead)
            {
                return;
            }
        }
    }

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

    CancelAggroDropTimer();
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

    GetWorldTimerManager().SetTimerForNextTick(
        this,
        &AEnemyCharacter::
        EvaluateAggroAfterPlayerLeft
    );
}

void AEnemyCharacter::
EvaluateAggroAfterPlayerLeft()
{
    if (!HasAuthority())
    {
        return;
    }

    APlayerCharacter* ClosestPlayer =
        FindClosestPlayer();

    if (IsValid(ClosestPlayer))
    {
        CancelAggroDropTimer();

        AEnemyAIController* EnemyController =
            Cast<AEnemyAIController>(
                GetController()
            );

        if (EnemyController)
        {
            EnemyController->SetAggroTarget(
                ClosestPlayer
            );
        }

        return;
    }

    if (GetWorldTimerManager().IsTimerActive(
        AggroDropTimerHandle))
    {
        return;
    }

    GetWorldTimerManager().SetTimer(
        AggroDropTimerHandle,
        this,
        &AEnemyCharacter::
        HandleAggroDropTimerExpired,
        AggroDropDelay,
        false
    );
}

void AEnemyCharacter::
HandleAggroDropTimerExpired()
{
    if (!HasAuthority())
    {
        return;
    }

    APlayerCharacter* ClosestPlayer =
        FindClosestPlayer();

    if (IsValid(ClosestPlayer))
    {
        AEnemyAIController* EnemyController =
            Cast<AEnemyAIController>(
                GetController()
            );

        if (EnemyController)
        {
            EnemyController->SetAggroTarget(
                ClosestPlayer
            );
        }

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

    EnemyController->ClearAggroTarget();
}

void AEnemyCharacter::CancelAggroDropTimer()
{
    if (!GetWorldTimerManager().IsTimerActive(
        AggroDropTimerHandle))
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(
        AggroDropTimerHandle
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
    GetWorldTimerManager().ClearTimer(AggroDropTimerHandle);

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


    //Updates aggro target on damage taken
    if (HasAuthority() && !bIsDead && Data.NewValue < Data.OldValue &&
        Data.GEModData)
    {
        const FGameplayEffectContextHandle& Context = Data.GEModData->EffectSpec.GetContext();
        AActor* DamageInstigator = Context.GetOriginalInstigator();
        APlayerCharacter* Attacker = Cast<APlayerCharacter>(DamageInstigator);

        if (!Attacker)
        {
            if (const AController* AttackingController = Cast<AController>(DamageInstigator))
            {
                Attacker = Cast<APlayerCharacter>(AttackingController->GetPawn());
            }
        }
        if (!Attacker)
        {
            Attacker = Cast<APlayerCharacter>(Context.GetEffectCauser());
        }
        if (IsValid(Attacker) && !Attacker->bIsDead)
        {
            CancelAggroDropTimer();

            if (AEnemyAIController* EnemyController =
                Cast<AEnemyAIController>(GetController()))
            {
                EnemyController->SetAggroTarget(Attacker);

                if (!AggroSphere || !AggroSphere->IsOverlappingActor(Attacker))
                {
                    GetWorldTimerManager().SetTimer(
                        AggroDropTimerHandle,
                        this,
                        &AEnemyCharacter::HandleAggroDropTimerExpired,
                        AggroDropDelay,
                        false);
                }
            }
        }
    }
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