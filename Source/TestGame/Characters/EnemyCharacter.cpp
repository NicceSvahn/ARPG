#include "EnemyCharacter.h"

#include "../AI/EnemyAIController.h"
#include "../UI/EnemyHealthBarWidget.h"
#include "../UI/FloatingCombatText/DamageNumberActor.h"

#include "TestGame/AbilitySystem/TestGameAbilitySystemComponent.h"
#include "TestGame/AbilitySystem/Attributes/HealthAttributeSet.h"
#include "TestGame/AbilitySystem/Damage/DamageResult.h"
#include "TestGame/Characters/PlayerCharacter.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

#include "TimerManager.h"
#include "GameplayEffectExtension.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "GameFramework/Controller.h"

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    SetReplicateMovement(true);

    AIControllerClass =
        AEnemyAIController::StaticClass();

    AutoPossessAI =
        EAutoPossessAI::PlacedInWorldOrSpawned;

    AggroSphere =
        CreateDefaultSubobject<USphereComponent>(
            TEXT("AggroSphere")
        );

    AggroSphere->SetupAttachment(
        RootComponent
    );

    AggroSphere->SetCollisionEnabled(
        ECollisionEnabled::QueryOnly
    );

    AggroSphere->SetCollisionObjectType(
        ECC_WorldDynamic
    );

    AggroSphere->SetCollisionResponseToAllChannels(
        ECR_Ignore
    );

    AggroSphere->SetCollisionResponseToChannel(
        ECC_Pawn,
        ECR_Overlap
    );

    AggroSphere->SetGenerateOverlapEvents(
        true
    );

    AggroSphere->OnComponentBeginOverlap.AddDynamic(
        this,
        &AEnemyCharacter::HandleAggroBeginOverlap
    );

    AggroSphere->OnComponentEndOverlap.AddDynamic(
        this,
        &AEnemyCharacter::HandleAggroEndOverlap
    );

    EnemyHealthWidget =
        CreateDefaultSubobject<UWidgetComponent>(
            TEXT("EnemyHealthWidget")
        );

    EnemyHealthWidget->SetupAttachment(
        RootComponent
    );

    EnemyHealthWidget->SetWidgetSpace(
        EWidgetSpace::Screen
    );

    EnemyHealthWidget->SetDrawAtDesiredSize(
        true
    );

    EnemyHealthWidget->SetRelativeLocation(
        FVector(
            0.0f,
            0.0f,
            120.0f
        )
    );
}

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

    if (HasAuthority() && AbilitySystemComponent)
    {
        DamageResultHandle =
            AbilitySystemComponent
            ->OnDamageResult()
            .AddUObject(
                this,
                &AEnemyCharacter::HandleDamageResult
            );
    }

    if (HasAuthority() && AbilitySystemComponent)
    {
        AggroHealthChangedHandle =
            AbilitySystemComponent
            ->GetGameplayAttributeValueChangeDelegate(
                UHealthAttributeSet::GetHealthAttribute()
            )
            .AddUObject(
                this,
                &AEnemyCharacter::
                HandleHealthChangedForAggro
            );

        if (bIsElite)
        {
            ApplyAttributeEffect(
                EliteModifierEffect
            );
        }
    }

    InitializeHealthBar();

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

void AEnemyCharacter::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(
        AggroSearchTimer
    );

    GetWorldTimerManager().ClearTimer(
        AggroDropTimerHandle
    );

    if (
        AbilitySystemComponent &&
        AggroHealthChangedHandle.IsValid()
        )
    {
        AbilitySystemComponent
            ->GetGameplayAttributeValueChangeDelegate(
                UHealthAttributeSet::GetHealthAttribute()
            )
            .Remove(
                AggroHealthChangedHandle
            );

        AggroHealthChangedHandle.Reset();
    }

    if (
        AbilitySystemComponent &&
        DamageResultHandle.IsValid()
        )
    {
        AbilitySystemComponent
            ->OnDamageResult()
            .Remove(
                DamageResultHandle
            );

        DamageResultHandle.Reset();
    }


    Super::EndPlay(
        EndPlayReason
    );
}

void AEnemyCharacter::InitializeHealthBar()
{
    if (!EnemyHealthWidget)
    {
        return;
    }

    EnemyHealthWidget->InitWidget();

    UEnemyHealthBarWidget* HealthWidget =
        Cast<UEnemyHealthBarWidget>(
            EnemyHealthWidget
            ->GetUserWidgetObject()
        );

    if (!HealthWidget)
    {
        return;
    }

    HealthWidget->InitializeFromActor(
        this
    );
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

    // If we already have a valid living target, keep that target
    if (
        AEnemyAIController* EnemyController =
        Cast<AEnemyAIController>(
            GetController()
        )
        )
    {
        if (
            UBlackboardComponent* Blackboard =
            EnemyController
            ->GetBlackboardComponent()
            )
        {
            APlayerCharacter* CurrentTarget =
                Cast<APlayerCharacter>(
                    Blackboard->GetValueAsObject(
                        TEXT("TargetActor")
                    )
                );

            if (
                IsValid(CurrentTarget) &&
                !CurrentTarget->bIsDead
                )
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


    if (
        GetWorldTimerManager().IsTimerActive(
            AggroDropTimerHandle
        )
        )
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
    if (
        !GetWorldTimerManager().IsTimerActive(
            AggroDropTimerHandle
        )
        )
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
            FName(
                TEXT("State.Dead")
            )
        );


    APlayerCharacter* ClosestPlayer =
        nullptr;

    float ClosestDistanceSquared =
        TNumericLimits<float>::Max();

    const FVector EnemyLocation =
        GetActorLocation();


    for (
        AActor* OverlappingActor :
        OverlappingActors
        )
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

        if (
            DistanceSquared <
            ClosestDistanceSquared
            )
        {
            ClosestDistanceSquared =
                DistanceSquared;

            ClosestPlayer =
                PlayerCharacter;
        }
    }


    return ClosestPlayer;
}

void AEnemyCharacter::OnDeathStarted()
{
    GetWorldTimerManager().ClearTimer(
        AggroDropTimerHandle
    );

    Super::OnDeathStarted();

    if (HasAuthority())
    {
        OnEnemyDied.Broadcast(this);
    }

    if (AggroSphere)
    {
        AggroSphere->SetCollisionEnabled(
            ECollisionEnabled::NoCollision
        );
    }


    if (EnemyHealthWidget)
    {
        EnemyHealthWidget->SetVisibility(
            false
        );
    }


    if (
        AEnemyAIController* EnemyController =
        Cast<AEnemyAIController>(
            GetController()
        )
        )
    {
        EnemyController
            ->HandleControlledPawnDeath();
    }
}

void AEnemyCharacter::HandleHealthChangedForAggro(
    const FOnAttributeChangeData& Data)
{
    if (
        HasAuthority() &&
        !bIsDead &&
        Data.NewValue < Data.OldValue &&
        Data.GEModData
        )
    {
        const FGameplayEffectContextHandle& Context =
            Data.GEModData
            ->EffectSpec
            .GetContext();


        AActor* DamageInstigator =
            Context.GetOriginalInstigator();

        APlayerCharacter* Attacker =
            Cast<APlayerCharacter>(
                DamageInstigator
            );


        if (!Attacker)
        {
            if (
                const AController*
                AttackingController =
                Cast<AController>(
                    DamageInstigator
                )
                )
            {
                Attacker =
                    Cast<APlayerCharacter>(
                        AttackingController
                        ->GetPawn()
                    );
            }
        }


        if (!Attacker)
        {
            Attacker =
                Cast<APlayerCharacter>(
                    Context.GetEffectCauser()
                );
        }


        if (
            IsValid(Attacker) &&
            !Attacker->bIsDead
            )
        {
            CancelAggroDropTimer();


            if (
                AEnemyAIController*
                EnemyController =
                Cast<AEnemyAIController>(
                    GetController()
                )
                )
            {
                EnemyController->SetAggroTarget(
                    Attacker
                );


                if (
                    !AggroSphere ||
                    !AggroSphere
                    ->IsOverlappingActor(
                        Attacker
                    )
                    )
                {
                    GetWorldTimerManager().SetTimer(
                        AggroDropTimerHandle,
                        this,
                        &AEnemyCharacter::
                        HandleAggroDropTimerExpired,
                        AggroDropDelay,
                        false
                    );
                }
            }
        }
    }
}

void AEnemyCharacter::HandleDamageResult(
    const FDamageResult& DamageResult)
{
    if (!HasAuthority())
    {
        return;
    }

    if (DamageResult.DamageAmount <= 0.0f)
    {
        return;
    }

    MulticastShowDamageNumber(
        DamageResult.DamageAmount,
        DamageResult.bCritical
    );
}

void AEnemyCharacter::
MulticastShowDamageNumber_Implementation(
    float DamageAmount,
    bool bCritical)
{
    SpawnDamageNumber(
        DamageAmount,
        bCritical
    );
}

void AEnemyCharacter::SpawnDamageNumber(
    float DamageAmount,
    bool bCritical)
{
    if (!DamageNumberActorClass)
    {
        return;
    }

    UWorld* World =
        GetWorld();

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
        DamageAmount,
        bCritical
    );
}