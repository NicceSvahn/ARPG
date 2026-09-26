#include "GE_Damage.h"

#include "ExecCalc_Damage.h"

UGE_Damage::UGE_Damage()
{
    DurationPolicy = EGameplayEffectDurationType::Instant;

    FGameplayEffectExecutionDefinition ExecutionDefinition;
    ExecutionDefinition.CalculationClass = UExecCalc_Damage::StaticClass();
    Executions.Add(ExecutionDefinition);
}
