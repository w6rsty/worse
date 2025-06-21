#pragma once
#include "Base.hpp"

#include <cmath>
#include <tuple>
#include <cstring>
#include <algorithm>

namespace worse
{

    // clang-format off
    class Vector2
    {
        friend class Vector3;
        friend class Vector4;

    public:
        union
        {
            struct { float x, y; };
            float data[2];
        };

        constexpr Vector2()
            : x(0.0f), y(0.0f) {}
        constexpr Vector2(float const x, float const y)
            : x(x), y(y) {}
        explicit Vector2(float const* raw) { std::memcpy(data, raw, sizeof(float) * 2); }

        constexpr float elementSum() const     { return x + y; }
        constexpr float elementProduct() const { return x * y; }
        constexpr float elementMax() const     { return std::fmax(x, y); }
        constexpr float elementMin() const     { return std::fmin(x, y); }
        
        constexpr Vector2  operator+(Vector2 const& other) const { return Vector2(x + other.x, y + other.y); }
        constexpr Vector2  operator-(Vector2 const& other) const { return Vector2(x - other.x, y - other.y); }
        constexpr Vector2  operator*(float const scalar) const   { return Vector2(x * scalar, y * scalar); }
        constexpr Vector2  operator/(float const scalar) const {
            WS_ASSERT_MATH(scalar != 0.0f, "Division by zero in Vector2 division");
            float invScalar = 1.0f / scalar;
            return this->operator*(invScalar);
        }
        constexpr Vector2  operator-() const                     { return Vector2(-x, -y); }
        constexpr Vector2& operator+=(Vector2 const& other)      { x += other.x; y += other.y; return *this; }
        constexpr Vector2& operator-=(Vector2 const& other)      { x -= other.x; y -= other.y; return *this; }
        constexpr Vector2& operator*=(float const scalar)        { x *= scalar; y *= scalar; return *this; }
        constexpr Vector2& operator/=(float const scalar) {
            WS_ASSERT_MATH(scalar != 0.0f, "Division by zero in Vector2 division");
            float invScalar = 1.0f / scalar;
            this->operator*=(invScalar);
            return *this;
        }
        
        static constexpr Vector2 splat(float const v) { return Vector2(v, v); }
        static constexpr Vector2 X()       { return Vector2(1.0f, 0.0f); }
        static constexpr Vector2 Y()       { return Vector2(0.0f, 1.0f); }
        static constexpr Vector2 ZERO()    { return splat(0.0f); }
        static constexpr Vector2 ONE()     { return splat(1.0f); }
        static constexpr Vector2 NEG_X()   { return Vector2(-1.0f, 0.0f); }
        static constexpr Vector2 NEG_Y()   { return Vector2(0.0f, -1.0f); }
        static constexpr Vector2 NEG_ONE() { return Vector2(-1.0f, -1.0f); }
        static constexpr Vector2 MIN()     { return splat(F32MIN); }
        static constexpr Vector2 MAX()     { return splat(F32MAX); }
        static constexpr Vector2 INF()     { return splat(F32INF); }
        static constexpr Vector2 NANM()    { return splat(F32NAN); }

        // group of axis
        static constexpr std::tuple<Vector2, Vector2> AXES() { return std::make_tuple(X(), Y()); }
    };

    class Vector3
    {
        friend class Vector4;

    public:
        union
        {
            struct { float x, y, z; };
            float data[3];
        };

        constexpr Vector3()
            : x(0.0f), y(0.0f), z(0.0f) {}
        constexpr Vector3(float x, float y, float z)
            : x(x), y(y), z(z) {}
        explicit Vector3(float const* raw) { std::memcpy(data, raw, sizeof(float) * 3); }

        constexpr Vector3(Vector2 const& v2, float const z) : x(v2.x), y(v2.y), z(z) {}
        constexpr Vector3(float const x, Vector2 const& v2) : x(x), y(v2.x), z(v2.y) {}

        constexpr Vector2 xy() const { return Vector2(x, y); }
        constexpr Vector2 yz() const { return Vector2(y, z); }

        constexpr float elementSum() const     { return x + y + z; }
        constexpr float elementProduct() const { return x * y * z; }
        constexpr float elementMax() const     { return std::fmax(x, std::fmax(y, z)); }
        constexpr float elementMin() const     { return std::fmin(x, std::fmin(y, z)); }

        constexpr Vector3 operator+(Vector3 const& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
        constexpr Vector3 operator-(Vector3 const& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
        constexpr Vector3 operator*(float const scalar) const   { return Vector3(x * scalar, y * scalar, z * scalar); }
        constexpr Vector3 operator/(float const scalar) const {
            WS_ASSERT_MATH(scalar != 0.0f, "Division by zero in Vector3 division");
            float invScalar = 1.0f / scalar;
            return this->operator*(invScalar);
        }
        constexpr Vector3  operator-() const                { return Vector3(-x, -y, -z); }
        constexpr Vector3& operator+=(Vector3 const& other) { x += other.x; y += other.y; z += other.z; return *this; }
        constexpr Vector3& operator-=(Vector3 const& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
        constexpr Vector3& operator*=(float const scalar)   { x *= scalar; y *= scalar; z *= scalar; return *this; }
        constexpr Vector3& operator/=(float const scalar) {
            WS_ASSERT_MATH(scalar != 0.0f, "Division by zero in Vector3 division");
            float invScalar = 1.0f / scalar;
            this->operator*=(invScalar); 
            return *this;
        }

        static constexpr Vector3 splat(float const v) { return Vector3(v, v, v); }
        static constexpr Vector3 X()       { return Vector3(1.0f, 0.0f, 0.0f); }
        static constexpr Vector3 Y()       { return Vector3(0.0f, 1.0f, 0.0f); }
        static constexpr Vector3 Z()       { return Vector3(0.0f, 0.0f, 1.0f); }
        static constexpr Vector3 ZERO()    { return Vector3(0.0f, 0.0f, 0.0f); }
        static constexpr Vector3 ONE()     { return Vector3(1.0f, 1.0f, 1.0f); }
        static constexpr Vector3 NEG_X()   { return Vector3(-1.0f, 0.0f, 0.0f); }
        static constexpr Vector3 NEG_Y()   { return Vector3(0.0f, -1.0f, 0.0f); }
        static constexpr Vector3 NEG_Z()   { return Vector3(0.0f, 0.0f, -1.0f); }
        static constexpr Vector3 NEG_ONE() { return Vector3(-1.0f, -1.0f, -1.0f); }
        static constexpr Vector3 MIN()     { return splat(F32MIN); }
        static constexpr Vector3 MAX()     { return splat(F32MAX); }
        static constexpr Vector3 INF()     { return splat(F32INF); }
        static constexpr Vector3 NANM()    { return splat(F32NAN); }

        // group of axis
        static constexpr std::tuple<Vector3, Vector3, Vector3> AXES() { return std::make_tuple(X(), Y(), Z()); }
    };

    class Vector4
    {
    public:
        union
        {
            struct { float x, y, z, w; };
            float data[4];
        };

        constexpr Vector4()
            : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
        constexpr Vector4(float const x, float const y, float const z, float const w)
            : x(x), y(y), z(z), w(w) {}
        explicit Vector4(float const* raw) { std::memcpy(data, raw, sizeof(float) * 4); }

        constexpr Vector4(Vector2 const& v, Vector2 const& u) : x(v.x), y(v.y), z(u.x), w(u.y) {}
        constexpr Vector4(Vector2 const& v, float const z, float const w) : x(v.x), y(v.y), z(z), w(w) {}
        constexpr Vector4(Vector3 const& v3, float const w) : x(v3.x), y(v3.y), z(v3.z), w(w) {}

        constexpr Vector3 truncate() const { return Vector3(x, y, z); }
        constexpr Vector3 xyz() const      { return Vector3(x, y, z); }
        constexpr Vector2 xy() const       { return Vector2(x, y); }
        constexpr Vector2 zw() const       { return Vector2(z, w); }
        
        constexpr float elementSum() const     { return x + y + z + w; }
        constexpr float elementProduct() const { return x * y * z * w; }
        constexpr float elementMax() const     { return std::fmax(x, std::fmax(y, std::fmax(z, w))); }
        constexpr float elementMin() const     { return std::fmin(x, std::fmin(y, std::fmin(z, w))); }
        
        constexpr Vector4  operator+(Vector4 const& other) const { return Vector4(x + other.x, y + other.y, z + other.z, w + other.w); }
        constexpr Vector4  operator-(Vector4 const& other) const { return Vector4(x - other.x, y - other.y, z - other.z, w - other.w); }
        constexpr Vector4  operator*(float const scalar) const   { return Vector4(x * scalar, y * scalar, z * scalar, w * scalar); }
        constexpr Vector4  operator/(float const scalar) const {
            WS_ASSERT_MATH(scalar != 0.0f, "Division by zero in Vector4 division");
            float invScalar = 1.0f / scalar;
            return this->operator*(invScalar);
        }
        constexpr Vector4  operator-()const                 { return Vector4(-x, -y, -z, -w); }
        constexpr Vector4& operator+=(Vector4 const& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
        constexpr Vector4& operator-=(Vector4 const& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
        constexpr Vector4& operator*=(float const scalar)   { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
        constexpr Vector4& operator/=(float const scalar) {
            WS_ASSERT_MATH(scalar != 0.0f, "Division by zero in Vector4 division");
            float invScalar = 1.0f / scalar;
            this->operator*=(invScalar);
            return *this;
        }

        static constexpr Vector4 Splat(float const v) { return Vector4(v, v, v, v); }
        static constexpr Vector4 X()       { return Vector4(1.0f, 0.0f, 0.0f, 0.0f); }
        static constexpr Vector4 Y()       { return Vector4(0.0f, 1.0f, 0.0f, 0.0f); }
        static constexpr Vector4 Z()       { return Vector4(0.0f, 0.0f, 1.0f, 0.0f); }
        static constexpr Vector4 W()       { return Vector4(0.0f, 0.0f, 0.0f, 1.0f); }
        static constexpr Vector4 ZERO()    { return Vector4(0.0f, 0.0f, 0.0f, 0.0f); }
        static constexpr Vector4 ONE()     { return Vector4(1.0f, 1.0f, 1.0f, 1.0f); }
        static constexpr Vector4 NEG_X()   { return Vector4(-1.0f, 0.0f, 0.0f, 0.0f); }
        static constexpr Vector4 NEG_Y()   { return Vector4(0.0f, -1.0f, 0.0f, 0.0f); }
        static constexpr Vector4 NEG_Z()   { return Vector4(0.0f, 0.0f, -1.0f, 0.0f); }
        static constexpr Vector4 NEG_W()   { return Vector4(0.0f, 0.0f, 0.0f, -1.0f); }
        static constexpr Vector4 NEG_ONE() { return Vector4(-1.0f, -1.0f, -1.0f, -1.0f); }
        static constexpr Vector4 MIN()     { return Splat(F32MIN); }
        static constexpr Vector4 MAX()     { return Splat(F32MAX); }
        static constexpr Vector4 INF()     { return Splat(F32INF); }
        static constexpr Vector4 NANM()    { return Splat(F32NAN); }

        // group of axis
        static constexpr std::tuple<Vector4, Vector4, Vector4, Vector4> AXES() { return std::make_tuple(X(), Y(), Z(), W()); }
    };



    class Vector2i
    {
    public:
        union
        {
            struct { std::int32_t x, y; };
            std::int32_t data[2];
        };

        constexpr Vector2i()
            : x(0), y(0) {}
        constexpr Vector2i(std::int32_t const x, std::int32_t const y)
            : x(x), y(y) {}
        explicit Vector2i(std::int32_t const* raw) { std::memcpy(data, raw, sizeof(std::int32_t) * 2); }

        constexpr std::int32_t elementSum() const     { return x + y; }
        constexpr std::int32_t elementProduct() const { return x * y; }
        constexpr std::int32_t elementMax() const     { return std::max(x, y); }
        constexpr std::int32_t elementMin() const     { return std::min(x, y); }

        constexpr Vector2i operator+(Vector2i const& other) const     { return Vector2i(x + other.x, y + other.y); }
        constexpr Vector2i operator-(Vector2i const& other) const     { return Vector2i(x - other.x, y - other.y); }
        constexpr Vector2i operator*(std::int32_t const scalar) const { return Vector2i(x * scalar, y * scalar); }
        constexpr Vector2i operator/(std::int32_t const scalar) const {
            WS_ASSERT_MATH(scalar != 0, "Division by zero in Vector2i division");
            return Vector2i(x / scalar, y / scalar);
        }
    };

    class Vector3i
    {
    public:
        union
        {
            struct{ std::int32_t x, y, z; };
            std::int32_t data[3];
        };

        constexpr Vector3i()
            : x(0), y(0), z(0) {}
        constexpr Vector3i(std::int32_t const x, std::int32_t const y, std::int32_t const z)
            : x(x), y(y), z(z) {}
        explicit Vector3i(std::int32_t const* raw) { std::memcpy(data, raw, sizeof(std::int32_t) * 3); }

        constexpr std::int32_t elementSum() const     { return x + y + z; }
        constexpr std::int32_t elementProduct() const { return x * y * z; }
        constexpr std::int32_t elementMax() const     { return std::max(x, std::max(y, z)); }
        constexpr std::int32_t elementMin() const     { return std::min(x, std::min(y, z)); }

        constexpr Vector3i operator+(Vector3i const& other) const     { return Vector3i(x + other.x, y + other.y, z + other.z); }
        constexpr Vector3i operator-(Vector3i const& other) const     { return Vector3i(x - other.x, y - other.y, z - other.z); }
        constexpr Vector3i operator*(std::int32_t const scalar) const { return Vector3i(x * scalar, y * scalar, z * scalar); }
        constexpr Vector3i operator/(std::int32_t const scalar) const {
            WS_ASSERT_MATH(scalar != 0, "Division by zero in Vector3i division");
            return Vector3i(x / scalar, y / scalar, z / scalar);
        }
    };


    class Vector4i
    {
    public:
        union
        {
            struct { std::int32_t x, y, z, w; };
            std::int32_t data[4];
        };

        constexpr Vector4i()
            : x(0), y(0), z(0), w(0) {}
        constexpr Vector4i(std::int32_t const x, std::int32_t const y, std::int32_t const z, std::int32_t const w)
            : x(x), y(y), z(z), w(w) {}
        explicit Vector4i(std::int32_t const* raw) { std::memcpy(data, raw, sizeof(std::int32_t) * 4); }

        constexpr std::int32_t elementSum() const     { return x + y + z + w; }
        constexpr std::int32_t elementProduct() const { return x * y * z * w; }
        constexpr std::int32_t elementMax() const     { return std::max(x, std::max(y, std::max(z, w))); }
        constexpr std::int32_t elementMin() const     { return std::min(x, std::min(y, std::min(z, w))); }

        constexpr Vector4i operator+(Vector4i const& other) const     { return Vector4i(x + other.x, y + other.y, z + other.z, w + other.w); }
        constexpr Vector4i operator-(Vector4i const& other) const     { return Vector4i(x - other.x, y - other.y, z - other.z, w - other.w); }
        constexpr Vector4i operator*(std::int32_t const scalar) const { return Vector4i(x * scalar, y * scalar, z * scalar, w * scalar); }
        constexpr Vector4i operator/(std::int32_t const scalar) const {
            WS_ASSERT_MATH(scalar != 0, "Division by zero in Vector4i division");
            return Vector4i(x / scalar, y / scalar, z / scalar, w / scalar);
        }
    };

    inline constexpr float lengthSquared(Vector2 const& v) { return v.x * v.x + v.y * v.y; }
    inline constexpr float lengthSquared(Vector3 const& v) { return v.x * v.x + v.y * v.y + v.z * v.z; }
    inline constexpr float lengthSquared(Vector4 const& v) { return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; }
    
    inline constexpr float length(Vector2 const& v) { return std::sqrt(lengthSquared(v)); }
    inline constexpr float length(Vector3 const& v) { return std::sqrt(lengthSquared(v)); }
    inline constexpr float length(Vector4 const& v) { return std::sqrt(lengthSquared(v)); }

    inline constexpr Vector2 normalize(Vector2 const& v) { return v / length(v); }
    inline constexpr Vector3 normalize(Vector3 const& v) { return v / length(v); }
    inline constexpr Vector4 normalize(Vector4 const& v) { return v / length(v); }

    inline constexpr Vector2 abs(Vector2 const& v) { return Vector2(std::abs(v.x), std::abs(v.y)); }
    inline constexpr Vector3 abs(Vector3 const& v) { return Vector3(std::abs(v.x), std::abs(v.y), std::abs(v.z)); }
    inline constexpr Vector4 abs(Vector4 const& v) { return Vector4(std::abs(v.x), std::abs(v.y), std::abs(v.z), std::abs(v.w)); }

    inline constexpr Vector2 reciprocal(Vector2 const& v) { return Vector2(1.0f / v.x, 1.0f / v.y); }
    inline constexpr Vector3 reciprocal(Vector3 const& v) { return Vector3(1.0f / v.x, 1.0f / v.y, 1.0f / v.z); }
    inline constexpr Vector4 reciprocal(Vector4 const& v) { return Vector4(1.0f / v.x, 1.0f / v.y, 1.0f / v.z, 1.0f / v.w); }

    inline constexpr bool eq(Vector2 const& a, Vector2 const& b) { return worse::equal(a.x, b.x) && worse::equal(a.y, b.y); }
    inline constexpr bool eq(Vector3 const& a, Vector3 const& b) { return worse::equal(a.x, b.x) && worse::equal(a.y, b.y) && worse::equal(a.z, b.z); }
    inline constexpr bool eq(Vector4 const& a, Vector4 const& b) { return worse::equal(a.x, b.x) && worse::equal(a.y, b.y) && worse::equal(a.z, b.z) && worse::equal(a.w, b.w); }
    inline constexpr bool eq(Vector2i const& a, Vector2i const& b) { return a.x == b.x && a.y == b.y; }
    inline constexpr bool eq(Vector3i const& a, Vector3i const& b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
    inline constexpr bool eq(Vector4i const& a, Vector4i const& b) { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }

    inline constexpr bool neq(Vector2 const& a, Vector2 const& b) { return !eq(a, b); }
    inline constexpr bool neq(Vector3 const& a, Vector3 const& b) { return !eq(a, b); }
    inline constexpr bool neq(Vector4 const& a, Vector4 const& b) { return !eq(a, b); }
    inline constexpr bool neq(Vector2i const& a, Vector2i const& b) { return !eq(a, b); }
    inline constexpr bool neq(Vector3i const& a, Vector3i const& b) { return !eq(a, b); }
    inline constexpr bool neq(Vector4i const& a, Vector4i const& b) { return !eq(a, b); }

    inline constexpr float dot(Vector2 const& a, Vector2 const& b) { return a.x * b.x + a.y * b.y; }
    inline constexpr float dot(Vector3 const& a, Vector3 const& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
    inline constexpr float dot(Vector4 const& a, Vector4 const& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

    inline constexpr float cross(Vector2 const& a, Vector2 const& b) { return a.x * b.y - a.y * b.x; }
    inline constexpr Vector3 cross(Vector3 const& a, Vector3 const& b) { return Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }

    inline constexpr float distanceSquared(Vector2 const& a, Vector2 const& b) { return lengthSquared(a - b); }
    inline constexpr float distanceSquared(Vector3 const& a, Vector3 const& b) { return lengthSquared(a - b); }

    inline constexpr float distance(Vector2 const& a, Vector2 const& b) { return length(a - b); }
    inline constexpr float distance(Vector3 const& a, Vector3 const& b) { return length(a - b); }

    inline constexpr bool isNormalized(Vector2 const& v) { return worse::equal(lengthSquared(v), 1.0f); }
    inline constexpr bool isNormalized(Vector3 const& v) { return worse::equal(lengthSquared(v), 1.0f); }
    inline constexpr bool isNormalized(Vector4 const& v) { return worse::equal(lengthSquared(v), 1.0f); }

    inline constexpr bool isIdentity(Vector2 const& v) { return worse::equal(v.x, 1.0f) && worse::equal(v.y, 1.0f); }
    inline constexpr bool isIdentity(Vector3 const& v) { return worse::equal(v.x, 1.0f) && worse::equal(v.y, 1.0f) && worse::equal(v.z, 1.0f); }
    inline constexpr bool isIdentity(Vector4 const& v) { return worse::equal(v.x, 1.0f) && worse::equal(v.y, 1.0f) && worse::equal(v.z, 1.0f) && worse::equal(v.w, 1.0f); }
    inline constexpr bool isIdentity(Vector2i const& v) { return v.x == 1 && v.y == 1; }
    inline constexpr bool isIdentity(Vector3i const& v) { return v.x == 1 && v.y == 1 && v.z == 1; }
    inline constexpr bool isIdentity(Vector4i const& v) { return v.x == 1 && v.y == 1 && v.z == 1 && v.w == 1; }

    inline constexpr bool isZero(Vector2 const& v) { return worse::equal(v.x, 0.0f) && worse::equal(v.y, 0.0f); }
    inline constexpr bool isZero(Vector3 const& v) { return worse::equal(v.x, 0.0f) && worse::equal(v.y, 0.0f) && worse::equal(v.z, 0.0f); }
    inline constexpr bool isZero(Vector4 const& v) { return worse::equal(v.x, 0.0f) && worse::equal(v.y, 0.0f) && worse::equal(v.z, 0.0f) && worse::equal(v.w, 0.0f); }
    inline constexpr bool isZero(Vector2i const& v) { return v.x == 0 && v.y == 0; }
    inline constexpr bool isZero(Vector3i const& v) { return v.x == 0 && v.y == 0 && v.z == 0; }
    inline constexpr bool isZero(Vector4i const& v) { return v.x == 0 && v.y == 0 && v.z == 0 && v.w == 0; }

    inline constexpr Vector2  max(Vector2 const& a, Vector2 const& b)   { return Vector2(std::fmax(a.x, b.x), std::fmax(a.y, b.y)); }
    inline constexpr Vector3  max(Vector3 const& a, Vector3 const& b)   { return Vector3(std::fmax(a.x, b.x), std::fmax(a.y, b.y), std::fmax(a.z, b.z)); }
    inline constexpr Vector4  max(Vector4 const& a, Vector4 const& b)   { return Vector4(std::fmax(a.x, b.x), std::fmax(a.y, b.y), std::fmax(a.z, b.z), std::fmax(a.w, b.w)); }
    inline constexpr Vector2i max(Vector2i const& a, Vector2i const& b) { return Vector2i(std::fmax(a.x, b.x), std::fmax(a.y, b.y)); }
    inline constexpr Vector3i max(Vector3i const& a, Vector3i const& b) { return Vector3i(std::fmax(a.x, b.x), std::fmax(a.y, b.y), std::fmax(a.z, b.z)); }

    inline constexpr Vector2  min(Vector2 const& a, Vector2 const& b)   { return Vector2(std::min(a.x, b.x), std::min(a.y, b.y)); }
    inline constexpr Vector3  min(Vector3 const& a, Vector3 const& b)   { return Vector3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)); }
    inline constexpr Vector4  min(Vector4 const& a, Vector4 const& b)   { return Vector4(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w)); }
    inline constexpr Vector2i min(Vector2i const& a, Vector2i const& b) { return Vector2i(std::min(a.x, b.x), std::min(a.y, b.y)); }
    inline constexpr Vector3i min(Vector3i const& a, Vector3i const& b) { return Vector3i(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)); }

    inline constexpr Vector2 clamp(Vector2 const& value, Vector2 const& min, Vector2 const& max) { return Vector2(worse::clamp(value.x, min.x, max.x), worse::clamp(value.y, min.y, max.y)); }
    inline constexpr Vector3 clamp(Vector3 const& value, Vector3 const& min, Vector3 const& max) { return Vector3(worse::clamp(value.x, min.x, max.x), worse::clamp(value.y, min.y, max.y), worse::clamp(value.z, min.z, max.z)); }

    inline constexpr Vector2 lerp(Vector2 const& a, Vector2 const& b, float const t) { return a + (b - a) * t; }
    inline constexpr Vector3 lerp(Vector3 const& a, Vector3 const& b, float const t) { return a + (b - a) * t; }

    inline constexpr Vector2 saturate(Vector2 const& v) { return Vector2(worse::clamp(v.x, 0.0f, 1.0f), worse::clamp(v.y, 0.0f, 1.0f)); }
    inline constexpr Vector3 saturate(Vector3 const& v) { return Vector3(worse::clamp(v.x, 0.0f, 1.0f), worse::clamp(v.y, 0.0f, 1.0f), worse::clamp(v.z, 0.0f, 1.0f)); }

    // clang-format on
} // namespace worse