#pragma once

#include "linalg.h"

struct Ray
{
    float3 origin;
    float3 direction; // Must be normalized
};

bool IntersectRaySphere(const Ray & ray, const float3 & center, float radius, float * outT=0, float3 * outNormal=0);
bool IntersectRayTriangle(const Ray & ray, const float3 & vertex0, const float3 & vertex1, const float3 & vertex2, float * outT=0, float2 * outUv=0);

struct Pose
{
    float3 position;
    float4 orientation; // Represented as a unit-length quaternion

    Pose() : orientation(0,0,0,1) {}
    Pose(const float3 & position, const float4 & orientation) : position(position), orientation(orientation) {}

    float3 GetXDir() const { return qxdir(orientation); }
    float3 GetYDir() const { return qydir(orientation); }
    float3 GetZDir() const { return qzdir(orientation); }

    float3 TransformPoint(const float3 & point) const { return position + TransformDirection(point); }
    float3 TransformDirection(const float3 & direction) const { return qrot(orientation, direction); }
};

inline Pose operator * (const Pose & a, const Pose & b) { return {a.TransformPoint(b.position), qmul(a.orientation, b.orientation)}; }
inline Ray operator * (const Pose & pose, const Ray & ray) { return {pose.TransformPoint(ray.origin), pose.TransformDirection(ray.direction)}; }
