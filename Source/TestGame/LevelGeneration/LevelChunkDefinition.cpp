#include "LevelChunkDefinition.h"

namespace
{
    int32 GetRotationSteps(EChunkRotation Rotation)
    {
        return static_cast<int32>(Rotation);
    }
}

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

EChunkEdgeType ULevelChunkDefinition::GetEdge(
    EChunkConnectionDirection Direction,
    EChunkRotation Rotation
) const
{
    const int32 WorldDirectionIndex =
        static_cast<int32>(Direction);

    const int32 RotationSteps =
        GetRotationSteps(Rotation);

    /*
     * Convert the queried world direction into the tile's
     * corresponding local direction.
     *
     * This matches the physical Unreal rotation used when
     * streaming the tile:
     *
     * 0   -> Yaw   0
     * 90  -> Yaw +90
     * 180 -> Yaw 180
     * 270 -> Yaw -90
     */
    const int32 LocalDirectionIndex =
        (WorldDirectionIndex + RotationSteps) % 4;

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