#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraOccludableComponent.generated.h"

class UMaterialInstanceDynamic;
class UMeshComponent;

UCLASS(
    ClassGroup = (Camera),
    meta = (BlueprintSpawnableComponent)
)
class TESTGAME_API UCameraOccludableComponent
    : public UActorComponent
{
    GENERATED_BODY()

public:
    UCameraOccludableComponent();

    virtual void BeginPlay() override;

    void SetOccluded(bool bOccluded);

protected:
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Camera Occlusion",
        meta = (ClampMin = "0.0", ClampMax = "1.0")
    )
    float OccludedFadeAmount = 0.2f;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Camera Occlusion"
    )
    FName FadeParameterName = TEXT("FadeAmount");

private:
    UPROPERTY()
    TArray<TObjectPtr<UMaterialInstanceDynamic>>
        DynamicMaterials;

    bool bIsOccluded = false;

    void CreateDynamicMaterials();
    void ApplyFadeAmount(float FadeAmount);
};