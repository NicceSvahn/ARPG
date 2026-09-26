#pragma once

#include "CoreMinimal.h"
#include "GenericAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "PrimaryAttributeSet.generated.h"

UCLASS()
class TESTGAME_API UPrimaryAttributeSet : public UGenericAttributeSet
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(
        TArray<FLifetimeProperty>& OutLifetimeProps
    ) const override;

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_Strength,
        Category = "Attributes|Primary"
    )
    FGameplayAttributeData Strength;

    PLAY_ATTRIBUTE_ACCESSORS(UPrimaryAttributeSet, Strength);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_Dexterity,
        Category = "Attributes|Primary"
    )
    FGameplayAttributeData Dexterity;

    PLAY_ATTRIBUTE_ACCESSORS(UPrimaryAttributeSet, Dexterity);

    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_Intellect,
        Category = "Attributes|Primary"
    )
    FGameplayAttributeData Intellect;

    PLAY_ATTRIBUTE_ACCESSORS(UPrimaryAttributeSet, Intellect);

protected:
    UFUNCTION()
    void OnRep_Strength(
        const FGameplayAttributeData& OldStrength
    );

    UFUNCTION()
    void OnRep_Dexterity(
        const FGameplayAttributeData& OldDexterity
    );

    UFUNCTION()
    void OnRep_Intellect(
        const FGameplayAttributeData& OldIntellect
    );

};
