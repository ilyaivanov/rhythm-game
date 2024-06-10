#pragma once
#include "types.h"
#include "math.c"

// taken from https://stackoverflow.com/a/306332/1283124
// visual verification at https://silentmatt.com/rectangle-intersection/
i32 CheckTwoSquareOverlap(V2f bottomLeftA, f32 sizeA, V2f bottomLeftB, f32 sizeB)
{
    f32 leftA = bottomLeftA.x;
    f32 rightA = bottomLeftA.x + sizeA;
    f32 topA = bottomLeftA.y + sizeA;
    f32 bottomA = bottomLeftA.y;

    f32 leftB = bottomLeftB.x;
    f32 rightB = bottomLeftB.x + sizeB;
    f32 topB = bottomLeftB.y + sizeB;
    f32 bottomB = bottomLeftB.y;

    return (leftA < rightB && rightA > leftB &&
            topA > bottomB && bottomA < topB);
}

i32 IsPointInsideRect(V2f rectBottomLeft, V2f rectSize, V2f point)
{
    f32 rectTopRightX = rectBottomLeft.x + rectSize.x;
    f32 rectTopRightY = rectBottomLeft.y + rectSize.y;

    return point.x >= rectBottomLeft.x && point.x <= rectTopRightX &&
           point.y >= rectBottomLeft.y && point.y <= rectTopRightY;
}