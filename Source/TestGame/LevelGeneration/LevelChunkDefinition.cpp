#include "LevelChunkDefinition.h"

EChunkConnectionDirection
ULevelChunkDefinition::GetOppositeDirection(
    EChunkConnectionDirection Direction
)
{
    switch (Direction)
    {
    case EChunkConnectionDirection::North:
        return EChunkConnectionDirection::South;

    case EChunkConnectionDirection::East:
        return EChunkConnectionDirection::West;

    case EChunkConnectionDirection::South:
        return EChunkConnectionDirection::North;

    case EChunkConnectionDirection::West:
        return EChunkConnectionDirection::East;

    default:
        return Direction;
    }
}

EChunkConnectionDirection
ULevelChunkDefinition::GetRotatedDirection(
    EChunkConnectionDirection Direction,
    EChunkRotation Rotation
)
{
    const int32 DirectionIndex =
        static_cast<int32>(Direction);

    int32 RotationSteps = 0;

    switch (Rotation)
    {
    case EChunkRotation::Degrees0:
        RotationSteps = 0;
        break;

    case EChunkRotation::Degrees90:
        RotationSteps = 1;
        break;

    case EChunkRotation::Degrees180:
        RotationSteps = 2;
        break;

    case EChunkRotation::Degrees270:
        RotationSteps = 3;
        break;
    }

    return static_cast<EChunkConnectionDirection>(
        (DirectionIndex + RotationSteps) % 4
        );
}

EChunkEdgeType ULevelChunkDefinition::GetEdge(
    EChunkConnectionDirection Direction,
    EChunkRotation Rotation
) const
{
    const int32 DirectionIndex =
        static_cast<int32>(Direction);

    int32 RotationSteps = 0;

    switch (Rotation)
    {
    case EChunkRotation::Degrees0:
        RotationSteps = 0;
        break;

    case EChunkRotation::Degrees90:
        RotationSteps = 1;
        break;

    case EChunkRotation::Degrees180:
        RotationSteps = 2;
        break;

    case EChunkRotation::Degrees270:
        RotationSteps = 3;
        break;

    default:
        RotationSteps = 0;
        break;
    }

    /*
     * Physical Unreal rotation:
     *
     * Degrees0   -> Yaw   0 -> Local North faces World North
     * Degrees90  -> Yaw +90 -> Local North faces World East
     * Degrees180 -> Yaw 180 -> Local North faces World South
     * Degrees270 -> Yaw -90 -> Local North faces World West
     *
     * Direction is the WORLD direction we are querying.
     * Convert it back into the corresponding LOCAL direction.
     */
    const int32 LocalDirectionIndex =
        (DirectionIndex + RotationSteps) % 4;

    const EChunkConnectionDirection LocalDirection =
        static_cast<EChunkConnectionDirection>(
            LocalDirectionIndex
            );

    switch (LocalDirection)
    {
    case EChunkConnectionDirection::North:
        return Edges.North;

    case EChunkConnectionDirection::East:
        return Edges.East;

    case EChunkConnectionDirection::South:
        return Edges.South;

    case EChunkConnectionDirection::West:
        return Edges.West;

    default:
        return EChunkEdgeType::Closed;
    }
}