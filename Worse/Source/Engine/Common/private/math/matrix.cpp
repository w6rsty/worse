#include "math/matrix.hpp"

namespace Worse
{

    Matrix2 const Matrix2::IDENTITY{1.0f, 0.0f, 0.0f, 1.0f};
    Matrix2 const Matrix2::ZERO{0.0f, 0.0f, 0.0f, 0.0f};
    Matrix2 const Matrix2::NANM{kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN};

    Matrix3 const Matrix3::IDENTITY{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    Matrix3 const Matrix3::ZERO{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    Matrix3 const Matrix3::NANM{kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN};

    Matrix4 const Matrix4::IDENTITY{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    Matrix4 const Matrix4::ZERO{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    Matrix4 const Matrix4::NANM{kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN, kFloatNaN};

} // namespace Worse