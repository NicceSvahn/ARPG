#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "ResourceAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UResourceAttributeSet : public UGenericAttributeSet
{
    GENERATED_BODY()

public:
    UResourceAttributeSet();

    virtual void PostGameplayEffectExecute(
        const FGameplayEffectModCallbackData& Data
    ) override;

    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_Resource,
        Category = "Resource"
    )
    FGameplayAttributeData Resource;

    PLAY_ATTRIBUTE_ACCESSORS(
        UResourceAttributeSet,
        Resource
    );

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_MaxResource,
        Category = "Resource"
    )
    FGameplayAttributeData MaxResource;

    PLAY_ATTRIBUTE_ACCESSORS(
        UResourceAttributeSet,
        MaxResource
    );

protected:
    UFUNCTION()
    void OnRep_Resource(
        const FGameplayAttributeData& OldResource
    );

    UFUNCTION()
    void OnRep_MaxResource(
        const FGameplayAttributeData& OldMaxResource
    );
};