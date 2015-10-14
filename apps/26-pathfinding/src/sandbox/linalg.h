#pragma once

#include <tuple>

template<class T, int N> struct vec;

template<class T> struct vec<T,2>
{ 
    T x,y;

    vec() : x(), y() {}
    vec(T x, T y) : x(x), y(y) {}
    template<class U> explicit vec(const vec<U,2> & r) : x(T(r.x)), y(T(r.y)) {}
    
    std::tuple<T,T> tuple() const { return std::make_tuple(x,y); }
    template<class F> vec zip(T r, F f) const { return {f(x,r), f(y,r)}; }
    template<class F> vec zip(const vec & r, F f) const { return {f(x,r.x), f(y,r.y)}; }
};

template<class T> struct vec<T,3>
{ 
    T x,y,z;

    vec() : x(), y(), z() {}
    vec(T x, T y, T z) : x(x), y(y), z(z) {}
    vec(vec<T,2> xy, T z) : x(xy.x), y(xy.y), z(z) {}
    template<class U> explicit vec(const vec<U,3> & r) : x(T(r.x)), y(T(r.y)), z(T(r.z)) {}

    std::tuple<T,T,T> tuple() const { return std::make_tuple(x,y,z); }
    template<class F> vec zip(T r, F f) const { return {f(x,r), f(y,r), f(z,r)}; }
    template<class F> vec zip(const vec & r, F f) const { return {f(x,r.x), f(y,r.y), f(z,r.z)}; }
};

template<class T> struct vec<T,4>
{
    T x,y,z,w;

    vec() : x(), y(), z(), w() {}
    vec(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    vec(vec<T,3> xyz, T w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}
    template<class U> explicit vec(const vec<U,4> & r) : x(T(r.x)), y(T(r.y)), z(T(r.z)), w(T(r.w)) {}

    std::tuple<T,T,T,T> tuple() const { return std::make_tuple(x,y,z,w); }
    template<class F> vec zip(T r, F f) const { return {f(x,r), f(y,r), f(z,r)}; }
    template<class F> vec zip(const vec & r, F f) const { return {f(x,r.x), f(y,r.y), f(z,r.z), f(w,r.w)}; }
};

template<class T, int N> bool operator == (const vec<T,N> & a, const vec<T,N> & b) { return a.tuple() == b.tuple(); }
template<class T, int N> bool operator != (const vec<T,N> & a, const vec<T,N> & b) { return a.tuple() != b.tuple(); }
template<class T, int N> bool operator <  (const vec<T,N> & a, const vec<T,N> & b) { return a.tuple() <  b.tuple(); }
template<class T, int N> bool operator <= (const vec<T,N> & a, const vec<T,N> & b) { return a.tuple() <= b.tuple(); }
template<class T, int N> bool operator >  (const vec<T,N> & a, const vec<T,N> & b) { return a.tuple() >  b.tuple(); }
template<class T, int N> bool operator >= (const vec<T,N> & a, const vec<T,N> & b) { return a.tuple() >= b.tuple(); }

template<class T, int N> vec<T,N> operator - (const vec<T,N> & a) { return a.zip(T(), [](T a, T) { return -a; }); }

template<class T, int N> vec<T,N> operator + (const vec<T,N> & a, const vec<T,N> & b) { return a.zip(b, [](T a, T b) { return a+b; }); }
template<class T, int N> vec<T,N> operator - (const vec<T,N> & a, const vec<T,N> & b) { return a.zip(b, [](T a, T b) { return a-b; }); }
template<class T, int N> vec<T,N> operator * (const vec<T,N> & a, const vec<T,N> & b) { return a.zip(b, [](T a, T b) { return a*b; }); }
template<class T, int N> vec<T,N> operator / (const vec<T,N> & a, const vec<T,N> & b) { return a.zip(b, [](T a, T b) { return a/b; }); }
template<class T, int N> vec<T,N> operator % (const vec<T,N> & a, const vec<T,N> & b) { return a.zip(b, [](T a, T b) { return a%b; }); }

template<class T, int N> vec<T,N> operator + (const vec<T,N> & a, T b) { return a.zip(b, [](T a, T b) { return a+b; }); }
template<class T, int N> vec<T,N> operator - (const vec<T,N> & a, T b) { return a.zip(b, [](T a, T b) { return a-b; }); }
template<class T, int N> vec<T,N> operator * (const vec<T,N> & a, T b) { return a.zip(b, [](T a, T b) { return a*b; }); }
template<class T, int N> vec<T,N> operator / (const vec<T,N> & a, T b) { return a.zip(b, [](T a, T b) { return a/b; }); }
template<class T, int N> vec<T,N> operator % (const vec<T,N> & a, T b) { return a.zip(b, [](T a, T b) { return a%b; }); }

template<class T, int N, class U> vec<T,N> & operator += (vec<T,N> & a, const U & b) { return a = a+b; }
template<class T, int N, class U> vec<T,N> & operator -= (vec<T,N> & a, const U & b) { return a = a-b; }
template<class T, int N, class U> vec<T,N> & operator *= (vec<T,N> & a, const U & b) { return a = a*b; }
template<class T, int N, class U> vec<T,N> & operator /= (vec<T,N> & a, const U & b) { return a = a/b; }
template<class T, int N, class U> vec<T,N> & operator %= (vec<T,N> & a, const U & b) { return a = a%b; }

template<class T> T cross(const vec<T,2> & a, const vec<T,2> & b) { return a.x*b.y - a.y*b.x; }
template<class T> vec<T,3> cross(const vec<T,3> & a, const vec<T,3> & b) { return vec<T,3>{a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x}; }
template<class T> T dot(const vec<T,2> & a, const vec<T,2> & b) { return a.x*b.x + a.y*b.y; }
template<class T> T dot(const vec<T,3> & a, const vec<T,3> & b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
template<class T> T dot(const vec<T,4> & a, const vec<T,4> & b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
template<class T, int N> T mag2(const vec<T,N> & a) { return dot(a,a); }
template<class T, int N> T mag(const vec<T,N> & a) { return std::sqrt(mag2(a)); }
template<class T, int N> vec<T,N> norm(const vec<T,N> & a) { return a/mag(a); }

template<class T> vec<T,4> qconj(const vec<T,4> & q) { return {-q.x,-q.y,-q.z,q.w}; }
template<class T> vec<T,4> qinv(const vec<T,4> & q) { return qconj(q)/mag2(q); }
template<class T> vec<T,4> qmul(const vec<T,4> & a, const vec<T,4> & b) { return {a.x*b.w+a.w*b.x+a.y*b.z-a.z*b.y, a.y*b.w+a.w*b.y+a.z*b.x-a.x*b.z, a.z*b.w+a.w*b.z+a.x*b.y-a.y*b.x, a.w*b.w-a.x*b.x-a.y*b.y-a.z*b.z}; }
template<class T> vec<T,3> qrot(const vec<T,4> & q, const vec<T,3> & v) { return qxdir(q)*v.x + qydir(q)*v.y + qzdir(q)*v.z; } // qvq*    
template<class T> vec<T,3> qxdir(const vec<T,4> & q) { return {q.w*q.w+q.x*q.x-q.y*q.y-q.z*q.z, (q.x*q.y+q.z*q.w)*2, (q.z*q.x-q.y*q.w)*2}; } // qrot(q,{1,0,0})
template<class T> vec<T,3> qydir(const vec<T,4> & q) { return {(q.x*q.y-q.z*q.w)*2, q.w*q.w-q.x*q.x+q.y*q.y-q.z*q.z, (q.y*q.z+q.x*q.w)*2}; } // qrot(q,{0,1,0})
template<class T> vec<T,3> qzdir(const vec<T,4> & q) { return {(q.z*q.x+q.y*q.w)*2, (q.y*q.z-q.x*q.w)*2, q.w*q.w-q.x*q.x-q.y*q.y+q.z*q.z}; } // qrot(q,{0,0,1})

typedef vec<int,2> int2; typedef vec<float,2> float2; typedef vec<double,2> double2;
typedef vec<int,3> int3; typedef vec<float,3> float3; typedef vec<double,3> double3;
typedef vec<int,4> int4; typedef vec<float,4> float4; typedef vec<double,4> double4;
