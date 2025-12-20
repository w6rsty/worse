#pragma once

#include "BaseTypes.hpp"
#include "Macro/Common.hpp"
#include "Math/Math.hpp"
#include "Math/Comparison.hpp"

namespace worse
{
    class Vector2;
    class Vector3;
    class Vector4;

    class Vector2
    {
    public:
        F32 x{0.0f}, y{0.0f};

    public:
        constexpr Vector2()
        {
        }
        explicit constexpr Vector2(F32 const scalar)
            : x{scalar}, y{scalar}
        {
        }
        explicit constexpr Vector2(F32 const* ptr)
            : x{ptr[0]}, y{ptr[1]}
        {
        }
        explicit constexpr Vector2(F32 f0, F32 f1)
            : x{f0}, y{f1}
        {
        }

        F32* ptr()
        {
            return &x;
        }
        F32 const* ptr() const
        {
            return &x;
        }

        F32 operator[](Size const index)
        {
            WORSE_ASSERT((index >= 0) && (index < 2));
            return ptr()[index];
        }
        F32 const operator[](Size const index) const
        {
            WORSE_ASSERT((index >= 0) && (index < 2));
            return ptr()[index];
        }

        bool operator==(Vector2 const& rhs) const
        {
            return Eq(x, rhs.x) && Eq(y, rhs.y);
        }
        bool operator!=(Vector2 const& rhs) const
        {
            return Neq(x, rhs.x) && Neq(y, rhs.y);
        }

        Vector2 operator+() const
        {
            return *this;
        }
        Vector2 operator-() const
        {
            return Vector2{-x, -y};
        }

        Vector2 operator+(F32 const rhs) const
        {
            return Vector2{x + rhs, y + rhs};
        }
        Vector2 operator+(Vector2 const& rhs) const
        {
            return Vector2{x + rhs.x, y + rhs.y};
        }
        Vector2 operator-(F32 const rhs) const
        {
            return Vector2{x - rhs, y - rhs};
        }
        Vector2 operator-(Vector2 const& rhs) const
        {
            return Vector2{x - rhs.x, y - rhs.y};
        }
        Vector2 operator*(F32 const rhs) const
        {
            return Vector2{x * rhs, y * rhs};
        }
        Vector2 operator*(Vector2 const& rhs) const
        {
            return Vector2{x * rhs.x, y * rhs.y};
        }
        Vector2 operator/(F32 const rhs) const
        {
            WORSE_ASSERT(rhs != 0.0f);
            F32 const inv = 1.0f / rhs;
            return Vector2{x * inv, y * inv};
        }
        Vector2 operator/(Vector2 const& rhs) const
        {
            return Vector2{x / rhs.x, y / rhs.y};
        }

        friend Vector2 operator+(F32 const lhs, Vector2 const& rhs)
        {
            return Vector2{lhs + rhs.x, lhs + rhs.y};
        }
        friend Vector2 operator-(F32 const lhs, Vector2 const& rhs)
        {
            return Vector2{lhs - rhs.x, lhs - rhs.y};
        }
        friend Vector2 operator*(F32 const lhs, Vector2 const& rhs)
        {
            return Vector2{lhs * rhs.x, lhs * rhs.y};
        }
        friend Vector2 operator/(F32 const lhs, Vector2 const& rhs)
        {
            return Vector2{lhs / rhs.x, lhs / rhs.y};
        }

        Vector2& operator+=(F32 const rhs)
        {
            x += rhs;
            y += rhs;
            return *this;
        }
        Vector2& operator+=(Vector2 const& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }
        Vector2& operator-=(F32 const rhs)
        {
            x -= rhs;
            y -= rhs;
            return *this;
        }
        Vector2& operator-=(Vector2 const& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }
        Vector2& operator*=(F32 const rhs)
        {
            x *= rhs;
            y *= rhs;
            return *this;
        }
        Vector2& operator*=(Vector2 const& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            return *this;
        }
        Vector2& operator/=(F32 const rhs)
        {
            WORSE_ASSERT(rhs != 0.0f);
            F32 const inv = 1.0f / rhs;
            x *= rhs;
            y *= rhs;
            return *this;
        }
        Vector2& operator/=(Vector2 const& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            return *this;
        }

        WORSE_FORCE_INLINE F32 LengthSquared() const
        {
            return x * x + y * y;
        }
        WORSE_FORCE_INLINE F32 Length() const
        {
            return FMath::Sqrt(x * x + y * y);
        }

        WORSE_FORCE_INLINE friend F32 Distance(Vector2 const& lhs, Vector2 const& rhs)
        {
            return FMath::Sqrt(FMath::Square(lhs.x - rhs.x) + FMath::Square(lhs.y - rhs.y));
        }
        WORSE_FORCE_INLINE friend F32 DistanceManhattan(Vector2 const& lhs, Vector2 const& rhs)
        {
            return FMath::Abs(lhs.x - rhs.x) + FMath::Abs(lhs.y - rhs.y);
        }
        WORSE_FORCE_INLINE friend F32 DistanceChebyshev(Vector2 const& lhs, Vector2 const& rhs)
        {
            return Max(abs(lhs.x - rhs.x), abs(lhs.y - rhs.y));
        }

        WORSE_FORCE_INLINE friend Vector2 Normalize(Vector2 const& v)
        {
            F32 const len = v.Length();
            WORSE_ASSERT(len != 0.0f);
            F32 const invLen = 1.0f / len;
            return Vector2{v.x * invLen, v.y * invLen};
        }

        WORSE_FORCE_INLINE friend F32 DotProduct(Vector2 const& lhs, Vector2 const& rhs)
        {
            return lhs.x * rhs.x + lhs.y * rhs.y;
        }
        WORSE_FORCE_INLINE friend F32 CrossProduct(Vector2 const& lhs, Vector2 const& rhs)
        {
            return lhs.x * rhs.y - lhs.y * rhs.x;
        }

        WORSE_FORCE_INLINE friend Vector2 Clamp(Vector2 const& v, Vector2 const& min, Vector2 const& max)
        {
            return Vector2{FMath::Clamp(v.x, min.x, max.x), FMath::Clamp(v.y, min.y, max.y)};
        }

        WORSE_FORCE_INLINE friend Vector2 Saturate(Vector2 const& v)
        {
            return Vector2{FMath::Saturate(v.x), FMath::Saturate(v.y)};
        }

        WORSE_FORCE_INLINE friend Vector2 Lerp(Vector2 const& a, Vector2 const& b, F32 const t)
        {
            return Vector2{FMath::Lerp(a.x, b.x, t), FMath::Lerp(a.y, b.y, t)};
        }

        WORSE_FORCE_INLINE F32 ElementMin() const
        {
            return Min(x, y);
        }
        WORSE_FORCE_INLINE F32 ElementMax() const
        {
            return Max(x, y);
        }
        WORSE_FORCE_INLINE F32 ElementSum() const
        {
            return x + y;
        }
        WORSE_FORCE_INLINE F32 ElementProduct() const
        {
            return x * y;
        }

        static Vector2 const ZERO;
        static Vector2 const ONE;
        static Vector2 const UNIT_X;
        static Vector2 const UNIT_Y;
        static Vector2 const NEGATIVE_UNIT_X;
        static Vector2 const NEGATIVE_UNIT_Y;
    };

    class Vector3
    {
    public:
        F32 x{0.0f}, y{0.0f}, z{0.0f};

    public:
        constexpr Vector3()
        {
        }
        explicit constexpr Vector3(F32 const scalar)
            : x{scalar}, y{scalar}, z{scalar}
        {
        }
        explicit constexpr Vector3(F32 const* ptr)
            : x{ptr[0]}, y{ptr[1]}, z{ptr[2]}
        {
        }
        explicit constexpr Vector3(F32 f0, F32 f1, F32 f2)
            : x{f0}, y{f1}, z{f2}
        {
        }
        explicit constexpr Vector3(F32 f0, Vector2 const& v2_1)
            : x{f0}, y{v2_1.x}, z{v2_1.y}
        {
        }
        explicit constexpr Vector3(Vector2 const& v2_0, F32 f1)
            : x{v2_0.x}, y{v2_0.y}, z{f1}
        {
        }

        F32* ptr()
        {
            return &x;
        }
        F32 const* ptr() const
        {
            return &x;
        }

        F32 operator[](Size const index)
        {
            WORSE_ASSERT((index >= 0) && (index < 3));
            return ptr()[index];
        }
        F32 const operator[](Size const index) const
        {
            WORSE_ASSERT((index >= 0) && (index < 3));
            return ptr()[index];
        }

        bool operator==(Vector3 const& rhs) const
        {
            return Eq(x, rhs.x) && Eq(y, rhs.y) && Eq(z, rhs.z);
        }
        bool operator!=(Vector3 const& rhs) const
        {
            return Neq(x, rhs.x) || Neq(y, rhs.y) || Neq(z, rhs.z);
        }

        Vector3 operator+() const
        {
            return *this;
        }
        Vector3 operator-() const
        {
            return Vector3{-x, -y, -z};
        }

        Vector3 operator+(F32 const rhs) const
        {
            return Vector3{x + rhs, y + rhs, z + rhs};
        }
        Vector3 operator+(Vector3 const& rhs) const
        {
            return Vector3{x + rhs.x, y + rhs.y, z + rhs.z};
        }
        Vector3 operator-(F32 const rhs) const
        {
            return Vector3{x - rhs, y - rhs, z - rhs};
        }
        Vector3 operator-(Vector3 const& rhs) const
        {
            return Vector3{x - rhs.x, y - rhs.y, z - rhs.z};
        }
        Vector3 operator*(F32 const rhs) const
        {
            return Vector3{x * rhs, y * rhs, z * rhs};
        }
        Vector3 operator*(Vector3 const& rhs) const
        {
            return Vector3{x * rhs.x, y * rhs.y, z * rhs.z};
        }
        Vector3 operator/(F32 const rhs) const
        {
            WORSE_ASSERT(rhs != 0.0f);
            F32 const inv = 1.0f / rhs;
            return Vector3{x * inv, y * inv, z * inv};
        }
        Vector3 operator/(Vector3 const& rhs) const
        {
            return Vector3{x / rhs.x, y / rhs.y, z / rhs.z};
        }

        friend Vector3 operator+(F32 const lhs, Vector3 const& rhs)
        {
            return Vector3{lhs + rhs.x, lhs + rhs.y, lhs + rhs.z};
        }
        friend Vector3 operator-(F32 const lhs, Vector3 const& rhs)
        {
            return Vector3{lhs - rhs.x, lhs - rhs.y, lhs - rhs.z};
        }
        friend Vector3 operator*(F32 const lhs, Vector3 const& rhs)
        {
            return Vector3{lhs * rhs.x, lhs * rhs.y, lhs * rhs.z};
        }
        friend Vector3 operator/(F32 const lhs, Vector3 const& rhs)
        {
            return Vector3{lhs / rhs.x, lhs / rhs.y, lhs / rhs.z};
        }

        Vector3& operator+=(F32 const scalar)
        {
            x += scalar;
            y += scalar;
            z += scalar;
            return *this;
        }
        Vector3& operator+=(Vector3 const& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            return *this;
        }
        Vector3& operator-=(F32 const scalar)
        {
            x -= scalar;
            y -= scalar;
            z -= scalar;
            return *this;
        }
        Vector3& operator-=(Vector3 const& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            return *this;
        }
        Vector3& operator*=(F32 const scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            return *this;
        }
        Vector3& operator*=(Vector3 const& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            return *this;
        }
        Vector3& operator/=(F32 const scalar)
        {
            WORSE_ASSERT(scalar != 0.0f);
            F32 const inv = 1.0f / scalar;
            x *= inv;
            y *= inv;
            z *= inv;
            return *this;
        }
        Vector3& operator/=(Vector3 const& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            return *this;
        }

        WORSE_FORCE_INLINE F32 LengthSquared() const
        {
            return x * x + y * y + z * z;
        }
        WORSE_FORCE_INLINE F32 Length() const
        {
            return FMath::Sqrt(x * x + y * y + z * z);
        }

        WORSE_FORCE_INLINE friend F32 Distance(Vector3 const& lhs, Vector3 const& rhs)
        {
            return FMath::Sqrt(FMath::Square(lhs.x - rhs.x) + FMath::Square(lhs.y - rhs.y) + FMath::Square(lhs.z - rhs.z));
        }
        WORSE_FORCE_INLINE friend F32 DistanceManhattan(Vector3 const& lhs, Vector3 const& rhs)
        {
            return FMath::Abs(lhs.x - rhs.x) + FMath::Abs(lhs.y - rhs.y) + FMath::Abs(lhs.z - rhs.z);
        }
        WORSE_FORCE_INLINE friend F32 DistanceChebyshev(Vector3 const& lhs, Vector3 const& rhs)
        {
            return Max(Max(FMath::Abs(lhs.x - rhs.x), FMath::Abs(lhs.y - rhs.y)), FMath::Abs(lhs.z - rhs.z));
        }

        WORSE_FORCE_INLINE friend Vector3 Normalize(Vector3 const& v)
        {
            F32 const len = v.Length();
            WORSE_ASSERT(len != 0.0f);
            F32 const invLen = 1.0f / len;
            return Vector3{v.x * invLen, v.y * invLen, v.z * invLen};
        }

        WORSE_FORCE_INLINE friend F32 DotProduct(Vector3 const& lhs, Vector3 const& rhs)
        {
            return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
        }
        WORSE_FORCE_INLINE friend Vector3 CrossProduct(Vector3 const& lhs, Vector3 const& rhs)
        {
            return Vector3{lhs.y * rhs.z - lhs.z * rhs.y,
                           lhs.z * rhs.x - lhs.x * rhs.z,
                           lhs.x * rhs.y - lhs.y * rhs.x};
        }

        WORSE_FORCE_INLINE friend Vector3 Clamp(Vector3 const& v, Vector3 const& min, Vector3 const& max)
        {
            return Vector3{FMath::Clamp(v.x, min.x, max.x), FMath::Clamp(v.y, min.y, max.y), FMath::Clamp(v.z, min.z, max.z)};
        }

        WORSE_FORCE_INLINE friend Vector3 Saturate(Vector3 const& v)
        {
            return Vector3{FMath::Saturate(v.x), FMath::Saturate(v.y), FMath::Saturate(v.z)};
        }

        WORSE_FORCE_INLINE friend Vector3 Lerp(Vector3 const& a, Vector3 const& b, F32 const t)
        {
            return Vector3{FMath::Lerp(a.x, b.x, t), FMath::Lerp(a.y, b.y, t), FMath::Lerp(a.z, b.z, t)};
        }

        WORSE_FORCE_INLINE F32 ElementMin() const
        {
            return Min(Min(x, y), z);
        }
        WORSE_FORCE_INLINE F32 ElementMax() const
        {
            return Max(Max(x, y), z);
        }
        WORSE_FORCE_INLINE F32 ElementSum() const
        {
            return x + y + z;
        }
        WORSE_FORCE_INLINE F32 ElementProduct() const
        {
            return x * y * z;
        }

        static Vector3 const ZERO;
        static Vector3 const ONE;
        static Vector3 const UNIT_X;
        static Vector3 const UNIT_Y;
        static Vector3 const UNIT_Z;
        static Vector3 const NEGATIVE_UNIT_X;
        static Vector3 const NEGATIVE_UNIT_Y;
        static Vector3 const NEGATIVE_UNIT_Z;
    };

    class Vector4
    {
    public:
        F32 x{0.0f}, y{0.0f}, z{0.0f}, w{0.0f};

    public:
        constexpr Vector4()
        {
        }
        explicit constexpr Vector4(F32 const scalar)
            : x{scalar}, y{scalar}, z{scalar}, w{scalar}
        {
        }
        explicit constexpr Vector4(F32 const* ptr)
            : x{ptr[0]}, y{ptr[1]}, z{ptr[2]}, w{ptr[3]}
        {
        }
        explicit constexpr Vector4(F32 f0, F32 f1, F32 f2, F32 f3)
            : x{f0}, y{f1}, z{f2}, w{f3}
        {
        }
        explicit constexpr Vector4(F32 f0, F32 f1, Vector2 const& v2_2)
            : x{f0}, y{f1}, z{v2_2.x}, w{v2_2.y}
        {
        }
        explicit constexpr Vector4(F32 f0, Vector2 const& v2_1, F32 f2)
            : x{f0}, y{v2_1.x}, z{v2_1.y}, w{f2}
        {
        }
        explicit constexpr Vector4(F32 f0, Vector3 const& v3_1)
            : x{f0}, y{v3_1.x}, z{v3_1.y}, w{v3_1.z}
        {
        }
        explicit constexpr Vector4(Vector2 const& v2_0, F32 f1, F32 f2)
            : x{v2_0.x}, y{v2_0.y}, z{f1}, w{f2}
        {
        }
        explicit constexpr Vector4(Vector2 const& v2_0, Vector2 const& v2_1)
            : x{v2_0.x}, y{v2_0.y}, z{v2_1.x}, w{v2_1.y}
        {
        }
        explicit constexpr Vector4(Vector3 const& v3_0, F32 f1)
            : x{v3_0.x}, y{v3_0.y}, z{v3_0.z}, w{f1}
        {
        }

        F32* ptr()
        {
            return &x;
        }
        F32 const* ptr() const
        {
            return &x;
        }

        F32 operator[](Size const index)
        {
            WORSE_ASSERT((index >= 0) && (index < 4));
            return ptr()[index];
        }
        F32 const operator[](Size const index) const
        {
            WORSE_ASSERT((index >= 0) && (index < 4));
            return ptr()[index];
        }

        bool operator==(Vector4 const& rhs) const
        {
            return Eq(x, rhs.x) && Eq(y, rhs.y) && Eq(z, rhs.z) && Eq(w, rhs.w);
        }
        bool operator!=(Vector4 const& rhs) const
        {
            return Neq(x, rhs.x) || Neq(y, rhs.y) || Neq(z, rhs.z) || Neq(w, rhs.w);
        }

        Vector4 operator+() const
        {
            return *this;
        }
        Vector4 operator-() const
        {
            return Vector4{-x, -y, -z, -w};
        }

        Vector4 operator+(F32 const rhs) const
        {
            return Vector4{x + rhs, y + rhs, z + rhs, w + rhs};
        }
        Vector4 operator+(Vector4 const& rhs) const
        {
            return Vector4{x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w};
        }
        Vector4 operator-(F32 const rhs) const
        {
            return Vector4{x - rhs, y - rhs, z - rhs, w - rhs};
        }
        Vector4 operator-(Vector4 const& rhs) const
        {
            return Vector4{x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w};
        }
        Vector4 operator*(F32 const rhs) const
        {
            return Vector4{x * rhs, y * rhs, z * rhs, w * rhs};
        }
        Vector4 operator*(Vector4 const& rhs) const
        {
            return Vector4{x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w};
        }
        Vector4 operator/(F32 const rhs) const
        {
            WORSE_ASSERT(rhs != 0.0f);
            F32 const inv = 1.0f / rhs;
            return Vector4{x * inv, y * inv, z * inv, w * inv};
        }
        Vector4 operator/(Vector4 const& rhs) const
        {
            return Vector4{x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w};
        }

        friend Vector4 operator+(F32 const lhs, Vector4 const& rhs)
        {
            return Vector4{lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w};
        }
        friend Vector4 operator-(F32 const lhs, Vector4 const& rhs)
        {
            return Vector4{lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.w};
        }
        friend Vector4 operator*(F32 const lhs, Vector4 const& rhs)
        {
            return Vector4{lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w};
        }
        friend Vector4 operator/(F32 const lhs, Vector4 const& rhs)
        {
            return Vector4{lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w};
        }

        Vector4& operator+=(F32 const scalar)
        {
            x += scalar;
            y += scalar;
            z += scalar;
            w += scalar;
            return *this;
        }
        Vector4& operator+=(Vector4 const& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            z += rhs.z;
            w += rhs.w;
            return *this;
        }
        Vector4& operator-=(F32 const scalar)
        {
            x -= scalar;
            y -= scalar;
            z -= scalar;
            w -= scalar;
            return *this;
        }
        Vector4& operator-=(Vector4 const& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            z -= rhs.z;
            w -= rhs.w;
            return *this;
        }
        Vector4& operator*=(F32 const scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;
            return *this;
        }
        Vector4& operator*=(Vector4 const& rhs)
        {
            x *= rhs.x;
            y *= rhs.y;
            z *= rhs.z;
            w *= rhs.w;
            return *this;
        }
        Vector4& operator/=(F32 const scalar)
        {
            WORSE_ASSERT(scalar != 0.0f);
            F32 const inv = 1.0f / scalar;
            x *= inv;
            y *= inv;
            z *= inv;
            w *= inv;
            return *this;
        }
        Vector4& operator/=(Vector4 const& rhs)
        {
            x /= rhs.x;
            y /= rhs.y;
            z /= rhs.z;
            w /= rhs.w;
            return *this;
        }

        WORSE_FORCE_INLINE F32 LengthSquared() const
        {
            return x * x + y * y + z * z + w * w;
        }
        WORSE_FORCE_INLINE F32 Length() const
        {
            return FMath::Sqrt(x * x + y * y + z * z + w * w);
        }

        WORSE_FORCE_INLINE friend F32 Distance(Vector4 const& lhs, Vector4 const& rhs)
        {
            return FMath::Sqrt(FMath::Square(lhs.x - rhs.x) + FMath::Square(lhs.y - rhs.y) + FMath::Square(lhs.z - rhs.z) + FMath::Square(lhs.w - rhs.w));
        }
        WORSE_FORCE_INLINE friend F32 DistanceManhattan(Vector4 const& lhs, Vector4 const& rhs)
        {
            return FMath::Abs(lhs.x - rhs.x) + FMath::Abs(lhs.y - rhs.y) + FMath::Abs(lhs.z - rhs.z) + FMath::Abs(lhs.w - rhs.w);
        }
        WORSE_FORCE_INLINE friend F32 DistanceChebyshev(Vector4 const& lhs, Vector4 const& rhs)
        {
            return Max(Max(FMath::Abs(lhs.x - rhs.x), FMath::Abs(lhs.y - rhs.y)), Max(FMath::Abs(lhs.z - rhs.z), FMath::Abs(lhs.w - rhs.w)));
        }

        WORSE_FORCE_INLINE friend Vector4 Normalize(Vector4 const& v)
        {
            F32 const len = v.Length();
            WORSE_ASSERT(len != 0.0f);
            F32 const invLen = 1.0f / len;
            return Vector4{v.x * invLen, v.y * invLen, v.z * invLen, v.w * invLen};
        }

        WORSE_FORCE_INLINE friend F32 DotProduct(Vector4 const& lhs, Vector4 const& rhs)
        {
            return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
        }

        WORSE_FORCE_INLINE friend Vector4 Clamp(Vector4 const& v, Vector4 const& min, Vector4 const& max)
        {
            return Vector4{FMath::Clamp(v.x, min.x, max.x), FMath::Clamp(v.y, min.y, max.y), FMath::Clamp(v.z, min.z, max.z), FMath::Clamp(v.w, min.w, max.w)};
        }

        WORSE_FORCE_INLINE friend Vector4 Saturate(Vector4 const& v)
        {
            return Vector4{FMath::Saturate(v.x), FMath::Saturate(v.y), FMath::Saturate(v.z), FMath::Saturate(v.w)};
        }

        WORSE_FORCE_INLINE friend Vector4 Lerp(Vector4 const& a, Vector4 const& b, F32 const t)
        {
            return Vector4{FMath::Lerp(a.x, b.x, t), FMath::Lerp(a.y, b.y, t), FMath::Lerp(a.z, b.z, t), FMath::Lerp(a.w, b.w, t)};
        }

        WORSE_FORCE_INLINE F32 ElementMin() const
        {
            return Min(Min(x, y), Min(z, w));
        }
        WORSE_FORCE_INLINE F32 ElementMax() const
        {
            return Max(Max(x, y), Max(z, w));
        }
        WORSE_FORCE_INLINE F32 ElementSum() const
        {
            return x + y + z + w;
        }
        WORSE_FORCE_INLINE F32 ElementProduct() const
        {
            return x * y * z * w;
        }

        static Vector4 const ZERO;
        static Vector4 const ONE;
        static Vector4 const UNIT_X;
        static Vector4 const UNIT_Y;
        static Vector4 const UNIT_Z;
        static Vector4 const UNIT_W;
        static Vector4 const NEGATIVE_UNIT_X;
        static Vector4 const NEGATIVE_UNIT_Y;
        static Vector4 const NEGATIVE_UNIT_Z;
        static Vector4 const NEGATIVE_UNIT_W;
    };

} // namespace worse