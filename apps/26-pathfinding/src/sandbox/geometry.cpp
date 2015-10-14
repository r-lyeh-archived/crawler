#include "geometry.h"
#include <cmath>

bool IntersectRaySphere(const Ray & ray, const float3 & center, float radius, float * outT, float3 * outNormal)
{
    auto delta = center - ray.origin;
    float b = dot(ray.direction, delta), disc = b*b + radius*radius - mag2(delta);
    if(disc < 0) return false;

    float t = b - std::sqrt(disc);
    if(t <= 0)
    {
        float t1 = 2*b - t;
        if(t1 <= 0) return false;
        t = 0;
    }

    if(outT) *outT = t;
    if(outNormal) *outNormal = t ? (ray.direction * t - delta) / radius : norm(ray.direction * t - delta);
    return true;
}

bool IntersectRayTriangle(const Ray & ray, const float3 & vertex0, const float3 & vertex1, const float3 & vertex2, float * outT, float2 * outUv)
{
    auto e1 = vertex1 - vertex0, e2 = vertex2 - vertex0;
    auto h = cross(ray.direction, e2);
    auto a = dot(e1, h);
    if (a < 0) return false;

    auto f = 1/a;
    auto s = ray.origin - vertex0;
    auto u = f * dot(s,h);
    if (u < 0 || u > 1) return false;

    auto q = cross(s,e1);
    auto v = f * dot(ray.direction,q);
    if (v < 0 || u + v > 1) return false;

    auto t = f * dot(e2,q);
    if(t < 0) return false;

    if(outT) *outT = t;
    if(outUv) *outUv = {u,v};
    return true;
}
