#include "AnimNotify_SpawnDirectionalNiagara.h"

#include "AnimationVFXUtils.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_SpawnDirectionalNiagara::Notify(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(
        MeshComp,
        Animation,
        EventReference
    );

    if (
        !MeshComp ||
        !NiagaraSystem
        )
    {
        return;
    }

    AActor* SourceActor =
        MeshComp->GetOwner();

    if (!SourceActor)
    {
        return;
    }

    AnimationVFXUtils::SpawnDirectionalNiagara(
        MeshComp,
        SourceActor,
        NiagaraSystem,
        LocalOffset,
        LocalRotationOffset,
        Scale,
        SpriteRotationParameterName,
        SpriteRotationOffsetDegrees,
        SpriteRotationMultiplier,
        bAutoDestroy,
        bPreCullCheck
    );
}
