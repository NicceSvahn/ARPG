#include "BuffDisplayDataAsset.h"

const FBuffDisplayData* UBuffDisplayDataAsset::FindBuff(const FGameplayTag& BuffTag) const
{
    return Buffs.FindByPredicate(
        [&BuffTag](const FBuffDisplayData& Data)
        {
            return Data.BuffTag == BuffTag;
        }
    );
}
