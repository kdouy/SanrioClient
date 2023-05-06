#pragma once
#include "vecs.h"
#include <iostream>
#include <windows.h>
#include <vector>

float matrix[16];
using namespace std;
float PI = 3.141592653589793238462643383279502884197;

namespace functions
{
    bool WorldToScreen(Vec3 pos, Vec2& screen, float matrix[16], int windowWidth, int windowHeight) // 3D to 2D
    {
        //Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
        Vec4 clipCoords;
        clipCoords.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
        clipCoords.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
        clipCoords.z = pos.x * matrix[2] + pos.y * matrix[6] + pos.z * matrix[10] + matrix[14];
        clipCoords.w = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

        if (clipCoords.w < 0.1f)
            return false;

        //perspective division, dividing by clip.W = Normalized Device Coordinates
        Vec3 NDC;
        NDC.x = clipCoords.x / clipCoords.w;
        NDC.y = clipCoords.y / clipCoords.w;
        NDC.z = clipCoords.z / clipCoords.w;

        //Transform to window coordinates
        screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
        screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
        return true;
    }

    float GetDistance3D(Vec3 localPos, Vec3 enemyPos)
    {
        return sqrt(
            pow(double(enemyPos.x - localPos.x), 2.0) +
            pow(double(enemyPos.y - localPos.y), 2.0) +
            pow(double(enemyPos.z - localPos.z), 2.0)
        );
    }

    float GetDistance2D(Vec2 localPos, Vec2 enemyPos)
    {
        return sqrt(
            pow(double(enemyPos.x - localPos.x), 2.0) +
            pow(double(enemyPos.y - localPos.y), 2.0)
        );
    }

    Vec2 CalcAngle(Vec3 src, Vec3 dst)
    {
        Vec2 angle;
        angle.x = -atan2f(dst.x - src.x, dst.y - src.y) / PI * 180.0f + 180.0f;
        angle.y = asinf((dst.z - src.z) / GetDistance3D(src, dst)) * 180.0f / PI;

        return angle;
    }
}