#include "CameraOccludableComponent.h"

#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UCameraOccludableComponent::
UCameraOccludableComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCameraOccludableComponent::BeginPlay()
{
    Super::BeginPlay();

    CreateDynamicMaterials();

    ApplyFadeAmount(1.0f);
}

void UCameraOccludableComponent::CreateDynamicMaterials()
{
    AActor* Owner = GetOwner();

    if (!Owner)
    {
        return;
    }

    TArray<UMeshComponent*> MeshComponents;

    Owner->GetComponents<UMeshComponent>(
        MeshComponents
    );

    for (UMeshComponent* MeshComponent : MeshComponents)
    {
        if (!MeshComponent)
        {
            continue;
        }

        const int32 MaterialCount =
            MeshComponent->GetNumMaterials();

        for (int32 MaterialIndex = 0;
            MaterialIndex < MaterialCount;
            ++MaterialIndex)
        {
            UMaterialInstanceDynamic* DynamicMaterial =
                MeshComponent
                ->CreateAndSetMaterialInstanceDynamic(
                    MaterialIndex
                );

            if (!DynamicMaterial)
            {
                continue;
            }

            DynamicMaterials.Add(
                DynamicMaterial
            );
        }
    }
}

void UCameraOccludableComponent::SetOccluded(
    bool bOccluded)
{
    if (bIsOccluded == bOccluded)
    {
        return;
    }

    bIsOccluded = bOccluded;

    const float TargetFade =
        bIsOccluded
        ? OccludedFadeAmount
        : 1.0f;

    ApplyFadeAmount(TargetFade);

    AActor* Owner = GetOwner();

    if (!Owner)
    {
        return;
    }

    TArray<UMeshComponent*> MeshComponents;

    Owner->GetComponents<UMeshComponent>(
        MeshComponents
    );

    for (UMeshComponent* MeshComponent : MeshComponents)
    {
        if (!MeshComponent)
        {
            continue;
        }

        MeshComponent->SetCastShadow(!bIsOccluded);
    }
}

void UCameraOccludableComponent::ApplyFadeAmount(
    float FadeAmount)
{
    for (UMaterialInstanceDynamic* DynamicMaterial
        : DynamicMaterials)
    {
        if (!DynamicMaterial)
        {
            continue;
        }

        DynamicMaterial->SetScalarParameterValue(
            FadeParameterName,
            FadeAmount
        );
    }
}