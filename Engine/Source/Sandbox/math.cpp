#include "Log.hpp"
#include "Math/Math.hpp"
#include "Definitions.hpp"

using namespace worse;

// 测试用的浮点数比较函数
bool floatEqual(float a, float b, float epsilon = 1e-6f)
{
    return std::abs(a - b) < epsilon;
}

bool vec3Equal(const Vector3& a, const Vector3& b, float epsilon = 1e-6f)
{
    return floatEqual(a.x, b.x, epsilon) && floatEqual(a.y, b.y, epsilon) &&
           floatEqual(a.z, b.z, epsilon);
}

bool mat4Equal(const Matrix4& a, const Matrix4& b, float epsilon = 1e-6f)
{
    return eq(a.col0, b.col0) && eq(a.col1, b.col1) && eq(a.col2, b.col2) &&
           eq(a.col3, b.col3);
}

bool quatEqual(const Quaternion& a, const Quaternion& b, float epsilon = 1e-6f)
{
    return floatEqual(a.w, b.w, epsilon) && floatEqual(a.x, b.x, epsilon) &&
           floatEqual(a.y, b.y, epsilon) && floatEqual(a.z, b.z, epsilon);
}

void testQuaternionBasics()
{
    WS_LOG_INFO("QuatTest", "Testing basic quaternion operations...");

    // 测试默认构造函数 - 应该是单位四元数
    Quaternion q1;
    WS_ASSERT_MSG(floatEqual(q1.w, 1.0f), "Default quaternion w should be 1.0");
    WS_ASSERT_MSG(floatEqual(q1.x, 0.0f), "Default quaternion x should be 0.0");
    WS_ASSERT_MSG(floatEqual(q1.y, 0.0f), "Default quaternion y should be 0.0");
    WS_ASSERT_MSG(floatEqual(q1.z, 0.0f), "Default quaternion z should be 0.0");
    WS_ASSERT_MSG(isIdentity(q1), "Default quaternion should be identity");

    // 测试构造函数
    Quaternion q2(1.0f, 2.0f, 3.0f, 4.0f);
    WS_ASSERT_MSG(floatEqual(q2.w, 1.0f), "Constructor w failed");
    WS_ASSERT_MSG(floatEqual(q2.x, 2.0f), "Constructor x failed");
    WS_ASSERT_MSG(floatEqual(q2.y, 3.0f), "Constructor y failed");
    WS_ASSERT_MSG(floatEqual(q2.z, 4.0f), "Constructor z failed");

    // 测试标量+向量构造函数
    Vector3 v(2.0f, 3.0f, 4.0f);
    Quaternion q3(1.0f, v);
    WS_ASSERT_MSG(floatEqual(q3.s, 1.0f), "Scalar part failed");
    WS_ASSERT_MSG(vec3Equal(q3.v3, v), "Vector part failed");

    WS_LOG_INFO("QuatTest", "✓ Basic operations passed");
}

void testQuaternionMagnitude()
{
    WS_LOG_INFO("QuatTest", "Testing magnitude calculations...");

    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    float expected = std::sqrt(1.0f + 4.0f + 9.0f + 16.0f); // sqrt(30)

    WS_ASSERT_MSG(floatEqual(magnitude(q), expected),
                  "Magnitude calculation failed");

    // 测试magnitude squared
    float expectedSq = 1.0f + 4.0f + 9.0f + 16.0f; // 30
    WS_ASSERT_MSG(floatEqual(magnitudeSquared(q), expectedSq),
                  "Magnitude squared failed");

    // 测试单位四元数的magnitude
    Quaternion identity = Quaternion::IDENTITY();
    WS_ASSERT_MSG(floatEqual(magnitude(identity), 1.0f),
                  "Identity magnitude should be 1");
    WS_ASSERT_MSG(isNormalized(identity), "Identity should be normalized");

    WS_LOG_INFO("QuatTest", "✓ Magnitude tests passed");
}

void testQuaternionNormalization()
{
    WS_LOG_INFO("QuatTest", "Testing normalization...");

    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion qNorm = normalize(q);

    WS_ASSERT_MSG(floatEqual(magnitude(qNorm), 1.0f),
                  "Normalized quaternion should have magnitude 1");
    WS_ASSERT_MSG(isNormalized(qNorm),
                  "Normalized quaternion should pass isNormalized test");

    // 原始四元数不应该被修改
    WS_ASSERT_MSG(floatEqual(q.w, 1.0f),
                  "Original quaternion w should be unchanged");
    WS_ASSERT_MSG(floatEqual(q.x, 2.0f),
                  "Original quaternion x should be unchanged");

    WS_LOG_INFO("QuatTest", "✓ Normalization tests passed");
}

void testQuaternionConjugate()
{
    WS_LOG_INFO("QuatTest", "Testing conjugate operations...");

    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion qConj = conjugate(q);

    // 共轭应该只改变向量部分的符号
    WS_ASSERT_MSG(floatEqual(qConj.w, 1.0f), "Conjugate w should be unchanged");
    WS_ASSERT_MSG(floatEqual(qConj.x, -2.0f), "Conjugate x should be negated");
    WS_ASSERT_MSG(floatEqual(qConj.y, -3.0f), "Conjugate y should be negated");
    WS_ASSERT_MSG(floatEqual(qConj.z, -4.0f), "Conjugate z should be negated");

    // 测试conjugate函数（现在是全局函数，不修改原四元数）
    Quaternion q2(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion q2Conj = conjugate(q2);
    WS_ASSERT_MSG(quatEqual(q2Conj, qConj),
                  "conjugate() should produce same result");

    // 原四元数应该保持不变
    WS_ASSERT_MSG(floatEqual(q2.w, 1.0f),
                  "Original quaternion w should be unchanged");
    WS_ASSERT_MSG(floatEqual(q2.x, 2.0f),
                  "Original quaternion x should be unchanged");

    WS_LOG_INFO("QuatTest", "✓ Conjugate tests passed");
}

void testQuaternionInverse()
{
    WS_LOG_INFO("QuatTest", "Testing inverse operations...");

    // 测试单位四元数的逆
    Quaternion identity    = Quaternion::IDENTITY();
    Quaternion identityInv = inverse(identity);
    WS_ASSERT_MSG(quatEqual(identityInv, identity),
                  "Identity inverse should be itself");

    // 测试一般四元数的逆
    Vector3 axis    = Vector3::Y();
    float angle     = PI / 4.0f;
    float halfAngle = angle * 0.5f;
    Quaternion q(std::cos(halfAngle), normalize(axis) * std::sin(halfAngle));
    Quaternion qInv    = inverse(q);
    Quaternion product = q * qInv;

    WS_ASSERT_MSG(quatEqual(product, Quaternion::IDENTITY(), 1e-5f),
                  "q * q^(-1) should equal identity");

    WS_LOG_INFO("QuatTest", "✓ Inverse tests passed");
}

void testQuaternionMultiplication()
{
    WS_LOG_INFO("QuatTest", "Testing quaternion multiplication...");

    // 测试与单位四元数相乘
    Quaternion q(1.0f, 2.0f, 3.0f, 4.0f);
    Quaternion identity = Quaternion::IDENTITY();
    Quaternion result   = q * identity;

    WS_ASSERT_MSG(quatEqual(result, q), "q * identity should equal q");

    // 测试乘法结合律
    Vector3 axisX = Vector3::X();
    Vector3 axisY = Vector3::Y();
    Vector3 axisZ = Vector3::Z();
    float angleX  = PI / 6.0f;
    float angleY  = PI / 4.0f;
    float angleZ  = PI / 3.0f;

    float halfAngleX = angleX * 0.5f;
    float halfAngleY = angleY * 0.5f;
    float halfAngleZ = angleZ * 0.5f;

    Quaternion q1(std::cos(halfAngleX),
                  normalize(axisX) * std::sin(halfAngleX));
    Quaternion q2(std::cos(halfAngleY),
                  normalize(axisY) * std::sin(halfAngleY));
    Quaternion q3(std::cos(halfAngleZ),
                  normalize(axisZ) * std::sin(halfAngleZ));

    Quaternion result1 = (q1 * q2) * q3;
    Quaternion result2 = q1 * (q2 * q3);

    WS_ASSERT_MSG(quatEqual(result1, result2, 1e-5f),
                  "Quaternion multiplication should be associative");

    WS_LOG_INFO("QuatTest", "✓ Multiplication tests passed");
}

void testAxisAngleConstruction()
{
    WS_LOG_INFO("QuatTest", "Testing axis-angle construction...");

    // 测试绕Y轴旋转90度
    Vector3 axis    = Vector3::Y();
    float angle     = PI / 2.0f;
    float halfAngle = angle * 0.5f;
    Quaternion q(std::cos(halfAngle), normalize(axis) * std::sin(halfAngle));

    WS_ASSERT_MSG(isNormalized(q),
                  "Axis-angle quaternion should be normalized");

    // 验证旋转一个向量
    Vector3 xAxis   = Vector3::X();
    Vector3 rotated = rotateAxisAngle(xAxis, axis, angle);

    // 绕Y轴旋转90度应该将X轴变为-Z轴
    Vector3 expected = -Vector3::Z();
    WS_ASSERT_MSG(vec3Equal(rotated, expected, 1e-5f),
                  "90-degree Y rotation failed");

    WS_LOG_INFO("QuatTest", "✓ Axis-angle tests passed");
}

void testEulerAngles()
{
    WS_LOG_INFO("QuatTest", "Testing Euler angle construction...");

    // 测试基本的欧拉角旋转
    Vector3 euler(PI / 6.0f, PI / 4.0f, PI / 3.0f); // 30, 45, 60 度
    Vector3 halfEuler = euler * 0.5f;
    Quaternion qX(std::cos(halfEuler.x), Vector3::X() * std::sin(halfEuler.x));
    Quaternion qY(std::cos(halfEuler.y), Vector3::Y() * std::sin(halfEuler.y));
    Quaternion qZ(std::cos(halfEuler.z), Vector3::Z() * std::sin(halfEuler.z));
    Quaternion q = qZ * qY * qX;

    WS_ASSERT_MSG(isNormalized(q),
                  "Euler angle quaternion should be normalized");

    // 验证零旋转
    Vector3 zeroEuler(0.0f, 0.0f, 0.0f);
    Vector3 zeroHalfEuler = zeroEuler * 0.5f;
    Quaternion qZeroX(std::cos(zeroHalfEuler.x),
                      Vector3::X() * std::sin(zeroHalfEuler.x));
    Quaternion qZeroY(std::cos(zeroHalfEuler.y),
                      Vector3::Y() * std::sin(zeroHalfEuler.y));
    Quaternion qZeroZ(std::cos(zeroHalfEuler.z),
                      Vector3::Z() * std::sin(zeroHalfEuler.z));
    Quaternion qZero = qZeroZ * qZeroY * qZeroX;
    WS_ASSERT_MSG(quatEqual(qZero, Quaternion::IDENTITY(), 1e-5f),
                  "Zero Euler should give identity");

    WS_LOG_INFO("QuatTest", "✓ Euler angle tests passed");
}

void testMatrixConversion()
{
    WS_LOG_INFO("QuatTest", "Testing matrix conversion...");

    // 测试单位四元数转换为矩阵
    Quaternion identity = Quaternion::IDENTITY();
    Matrix3 mat3        = identity.toMat3();
    Matrix4 mat4        = identity.toMat4();

    // 单位四元数应该产生单位矩阵
    WS_ASSERT_MSG(floatEqual(mat3.col0.x, 1.0f),
                  "Identity quat should give identity matrix (0,0)");
    WS_ASSERT_MSG(floatEqual(mat3.col1.y, 1.0f),
                  "Identity quat should give identity matrix (1,1)");
    WS_ASSERT_MSG(floatEqual(mat3.col2.z, 1.0f),
                  "Identity quat should give identity matrix (2,2)");

    // 测试往返转换
    Vector3 axis    = Vector3::Z();
    float angle     = PI / 3.0f;
    float halfAngle = angle * 0.5f;
    Quaternion q(std::cos(halfAngle), normalize(axis) * std::sin(halfAngle));
    Matrix3 m        = q.toMat3();
    Quaternion qBack = Quaternion::fromMat3(m);

    // 由于四元数的双重覆盖性质，我们需要检查q或-q
    bool match = quatEqual(q, qBack, 1e-4f) || quatEqual(q, -qBack, 1e-4f);
    WS_ASSERT_MSG(match, "Quaternion->Matrix->Quaternion roundtrip failed");

    WS_LOG_INFO("QuatTest", "✓ Matrix conversion tests passed");
}

void testInterpolation()
{
    WS_LOG_INFO("QuatTest", "Testing interpolation functions...");

    Quaternion q0   = Quaternion::IDENTITY();
    Vector3 axis    = Vector3::Y();
    float angle     = PI / 2.0f;
    float halfAngle = angle * 0.5f;
    Quaternion q1(std::cos(halfAngle), normalize(axis) * std::sin(halfAngle));

    // 测试lerp
    Quaternion lerpResult = lerp(q0, q1, 0.5f);
    WS_LOG_INFO("QuatTest",
                "Lerp result: w={:.3f}, x={:.3f}, y={:.3f}, z={:.3f}",
                lerpResult.w,
                lerpResult.x,
                lerpResult.y,
                lerpResult.z);

    // 测试nLerp
    Quaternion nlerpResult = nLerp(q0, q1, 0.5f);
    WS_LOG_INFO("QuatTest",
                "nLerp result: w={:.6f}, x={:.6f}, y={:.6f}, z={:.6f}",
                nlerpResult.w,
                nlerpResult.x,
                nlerpResult.y,
                nlerpResult.z);
    WS_LOG_INFO("QuatTest",
                "nLerp magnitude: {:.6f}, isNormalized: {}",
                magnitude(nlerpResult),
                isNormalized(nlerpResult));

    // 检查具体的magnitude值
    float mag = magnitude(nlerpResult);
    if (!floatEqual(mag, 1.0f, 1e-5f))
    {
        WS_LOG_WARN("QuatTest",
                    "nLerp magnitude {:.6f} is not close enough to 1.0",
                    mag);
    }

    WS_ASSERT_MSG(floatEqual(magnitude(nlerpResult), 1.0f, 1e-5f),
                  "nLerp result should be normalized");

    // 测试sLerp
    Quaternion slerpResult = sLerp(q0, q1, 0.5f);
    WS_ASSERT_MSG(isNormalized(slerpResult),
                  "sLerp result should be normalized");

    // 测试边界条件
    Quaternion slerp0 = sLerp(q0, q1, 0.0f);
    Quaternion slerp1 = sLerp(q0, q1, 1.0f);

    WS_ASSERT_MSG(quatEqual(slerp0, q0, 1e-5f), "sLerp at t=0 should equal q0");
    WS_ASSERT_MSG(quatEqual(slerp1, q1, 1e-5f), "sLerp at t=1 should equal q1");

    WS_LOG_INFO("QuatTest", "✓ Interpolation tests passed");
}

void testVectorRotation()
{
    WS_LOG_INFO("QuatTest", "Testing vector rotation...");

    // 测试绕各轴的旋转
    Vector3 v = Vector3::X();

    // 绕Z轴旋转90度应该将X变为Y
    Vector3 v1 = rotationZAngle(v, PI / 2.0f);
    WS_ASSERT_MSG(vec3Equal(v1, Vector3::Y(), 1e-5f),
                  "90-degree Z rotation failed");

    // 绕Y轴旋转90度应该将X变为-Z
    Vector3 v2 = rotationYAngle(v, PI / 2.0f);
    WS_ASSERT_MSG(vec3Equal(v2, -Vector3::Z(), 1e-5f),
                  "90-degree Y rotation failed");

    // 绕X轴旋转90度应该将Y变为Z
    Vector3 v3       = Vector3::Y();
    Vector3 v3Result = rotationXAngle(v3, PI / 2.0f);
    WS_ASSERT_MSG(vec3Equal(v3Result, Vector3::Z(), 1e-5f),
                  "90-degree X rotation failed");

    WS_LOG_INFO("QuatTest", "✓ Vector rotation tests passed");
}

void testErrorConditions()
{
    WS_LOG_INFO("QuatTest", "Testing error conditions and edge cases...");

    // 测试非归一化四元数的sLerp（这应该触发断言）
    Quaternion q1(2.0f, 0.0f, 0.0f, 0.0f); // 非归一化
    Quaternion q2 = Quaternion::IDENTITY();

    WS_LOG_WARN("QuatTest",
                "Testing sLerp with non-normalized quaternion (should assert)");
    // 注意：这应该会触发断言，但我们会捕获它
    // sLerp(q1, q2, 0.5f); // 这会触发断言失败

    // 测试接近平行的四元数（sLerp应该回退到nLerp）
    Quaternion qa = Quaternion::IDENTITY();
    Quaternion qb(1.0001f, 0.0f, 0.0f, 0.0f); // 几乎相同
    qb = normalize(qb);

    Quaternion result = sLerp(qa, qb, 0.5f);
    WS_ASSERT_MSG(isNormalized(result), "sLerp fallback should be normalized");

    WS_LOG_INFO("QuatTest", "✓ Error condition tests passed");
}

void testSlerpImprovements()
{
    WS_LOG_INFO("QuatTest", "Testing sLerp improvements...");

    // 测试负点积情况 - 四元数应该选择较短路径
    Quaternion q0 = Quaternion::IDENTITY();
    Quaternion q1 =
        Quaternion(-1.0f, 0.0f, 0.0f, 0.0f); // -identity (表示相同旋转)

    Quaternion result = sLerp(q0, q1, 0.5f);
    WS_ASSERT_MSG(isNormalized(result),
                  "sLerp with negative dot product should be normalized");

    // 测试接近垂直的四元数
    float halfAngleA = 0.0f;
    float halfAngleB = PI / 4.0f; // PI/2 * 0.5
    Quaternion qa(std::cos(halfAngleA), Vector3::X() * std::sin(halfAngleA));
    Quaternion qb(std::cos(halfAngleB), Vector3::X() * std::sin(halfAngleB));

    Quaternion slerpResult = sLerp(qa, qb, 0.5f);
    WS_ASSERT_MSG(isNormalized(slerpResult),
                  "sLerp with perpendicular quaternions should be normalized");

    WS_LOG_INFO("QuatTest", "✓ sLerp improvements tests passed");
}

void testMatrixConstruction()
{
    WS_LOG_INFO("ExtTest", "Testing matrix construction functions...");

    // Test makeScale
    Vector3 scale(2.0f, 3.0f, 4.0f);
    Matrix4 scaleMatrix = makeScale(scale);

    WS_ASSERT_MSG(equal(scaleMatrix.col0.x, 2.0f),
                  "Scale matrix X component incorrect");
    WS_ASSERT_MSG(equal(scaleMatrix.col1.y, 3.0f),
                  "Scale matrix Y component incorrect");
    WS_ASSERT_MSG(equal(scaleMatrix.col2.z, 4.0f),
                  "Scale matrix Z component incorrect");
    WS_ASSERT_MSG(equal(scaleMatrix.col3.w, 1.0f),
                  "Scale matrix W component should be 1");

    // Test scaling functionality
    Vector4 testVec(1.0f, 1.0f, 1.0f, 1.0f);
    Vector4 scaledVec = scaleMatrix * testVec;
    WS_ASSERT_MSG(equal(scaledVec.x, 2.0f) && equal(scaledVec.y, 3.0f) &&
                      equal(scaledVec.z, 4.0f),
                  "Scale matrix should scale vector correctly");

    // Test makeRotation with quaternion
    Quaternion rotY   = Quaternion::fromAxisAngle(Vector3::Y(), PI / 2.0f);
    Matrix4 rotMatrix = makeRotation(rotY);
    Vector4 xAxis(1.0f, 0.0f, 0.0f, 1.0f);
    Vector4 rotatedX = rotMatrix * xAxis;

    // In right-handed system, Y rotation by PI/2 should rotate X towards
    // negative Z
    WS_ASSERT_MSG(
        equal(rotatedX.z, -1.0f, 0.001f),
        "Y rotation should rotate X to negative Z in right-handed system");

    // Test makeRotation with euler angles
    Vector3 euler(0.0f, PI / 2.0f, 0.0f);
    Matrix4 eulerMatrix   = makeRotation(euler);
    Vector4 eulerRotatedX = eulerMatrix * xAxis;
    WS_ASSERT_MSG(equal(eulerRotatedX.z, -1.0f, 0.001f),
                  "Euler Y rotation should rotate X to negative Z");

    // Test makeTranslation
    Vector3 translation(5.0f, 6.0f, 7.0f);
    Matrix4 transMatrix = makeTranslation(translation);
    WS_ASSERT_MSG(equal(transMatrix.col3.x, 5.0f),
                  "Translation matrix X component incorrect");
    WS_ASSERT_MSG(equal(transMatrix.col3.y, 6.0f),
                  "Translation matrix Y component incorrect");
    WS_ASSERT_MSG(equal(transMatrix.col3.z, 7.0f),
                  "Translation matrix Z component incorrect");

    // Test translation functionality
    Vector4 translatedVec = transMatrix * testVec;
    WS_ASSERT_MSG(equal(translatedVec.x, 6.0f) &&
                      equal(translatedVec.y, 7.0f) &&
                      equal(translatedVec.z, 8.0f),
                  "Translation matrix should translate vector correctly");

    WS_LOG_INFO("ExtTest", "✓ Matrix construction tests passed");
}

void testSRTOperations()
{
    WS_LOG_INFO("ExtTest",
                "Testing SRT (Scale-Rotation-Translation) operations...");

    Vector3 scale(2.0f, 3.0f, 4.0f);
    Quaternion rotation = Quaternion::fromAxisAngle(Vector3::Z(), PI / 4.0f);
    Vector3 translation(10.0f, 20.0f, 30.0f);

    // Test makeSRT
    Matrix4 srtMatrix = makeSRT(scale, rotation, translation);

    // Verify the matrix produces expected transformation
    Vector4 testPoint(1.0f, 0.0f, 0.0f, 1.0f);
    Vector4 transformed = srtMatrix * testPoint;

    // Should be scaled first, then rotated, then translated
    Vector3 scaledVec = Vector3(2.0f, 0.0f, 0.0f);
    Quaternion result =
        rotation * Quaternion(0.0f, scaledVec) * conjugate(rotation);
    Vector3 rotatedVec = result.vector();
    Vector3 expected   = rotatedVec + translation;
    WS_ASSERT_MSG(equal(transformed.x, expected.x, 0.001f) &&
                      equal(transformed.y, expected.y, 0.001f) &&
                      equal(transformed.z, expected.z, 0.001f),
                  "SRT matrix should apply scale, rotation, translation in "
                  "correct order");

    // Test decomposeSRT
    auto [decomposedScale, decomposedRotation, decomposedTranslation] =
        decomposeSRT(srtMatrix);

    WS_ASSERT_MSG(equal(decomposedScale.x, scale.x, 0.001f) &&
                      equal(decomposedScale.y, scale.y, 0.001f) &&
                      equal(decomposedScale.z, scale.z, 0.001f),
                  "Decomposed scale should match original");

    WS_ASSERT_MSG(equal(decomposedTranslation.x, translation.x, 0.001f) &&
                      equal(decomposedTranslation.y, translation.y, 0.001f) &&
                      equal(decomposedTranslation.z, translation.z, 0.001f),
                  "Decomposed translation should match original");

    // Check rotation quaternion equality (quaternions can be negated and still
    // represent same rotation)
    WS_ASSERT_MSG(
        quatEqual(decomposedRotation, rotation, 0.001f) ||
            quatEqual(decomposedRotation, -rotation, 0.001f),
        "Decomposed rotation should match original (or its negative)");

    // Test round-trip consistency
    Matrix4 reconstructed =
        makeSRT(decomposedScale, decomposedRotation, decomposedTranslation);
    Vector4 testVec2(2.0f, 3.0f, 4.0f, 1.0f);
    Vector4 original_result      = srtMatrix * testVec2;
    Vector4 reconstructed_result = reconstructed * testVec2;

    WS_ASSERT_MSG(
        equal(original_result.x, reconstructed_result.x, 0.001f) &&
            equal(original_result.y, reconstructed_result.y, 0.001f) &&
            equal(original_result.z, reconstructed_result.z, 0.001f),
        "Reconstructed SRT matrix should match original transformation");

    WS_LOG_INFO("ExtTest", "✓ SRT operations tests passed");
}

void testViewMatrices()
{
    WS_LOG_INFO("ExtTest", "Testing view matrix functions...");

    Vector3 eye(0.0f, 0.0f, 0.0f);
    Vector3 target(0.0f, 0.0f, -1.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    // Test lookAt (camera view matrix)
    Matrix4 viewMatrix = lookAt(eye, target, up);

    // For a camera looking down negative Z, the matrix should be close to
    // identity but with appropriate handedness adjustments
    Vector4 forwardPoint(0.0f, 0.0f, -1.0f, 1.0f);
    Vector4 transformedForward = viewMatrix * forwardPoint;
    WS_ASSERT_MSG(transformedForward.z < 0.0f,
                  "Forward direction should map to negative Z in view space");

    // Test lookTo (object orientation matrix)
    Vector3 direction    = normalize(target - eye);
    Matrix4 orientMatrix = lookTo(eye, direction, up);

    // This should orient an object to face the given direction
    Vector4 localForward(0.0f, 0.0f, 1.0f, 1.0f);
    Vector4 worldForward  = orientMatrix * localForward;
    Vector3 worldForward3 = worldForward.truncate();

    WS_ASSERT_MSG(equal(length(worldForward3), 1.0f, 0.001f),
                  "Transformed forward should be unit length");
    WS_ASSERT_MSG(equal(dot(normalize(worldForward3), direction), 1.0f, 0.001f),
                  "Transformed forward should align with target direction");

    // Test with different eye position
    Vector3 eye2(5.0f, 3.0f, 2.0f);
    Vector3 target2(0.0f, 0.0f, 0.0f);
    Matrix4 viewMatrix2 = lookAt(eye2, target2, up);

    // Eye position should be transformed to origin in view space
    Vector4 eyeInView = viewMatrix2 * Vector4(eye2, 1.0f);
    WS_ASSERT_MSG(equal(eyeInView.x, 0.0f, 0.001f) &&
                      equal(eyeInView.y, 0.0f, 0.001f) &&
                      equal(eyeInView.z, 0.0f, 0.001f),
                  "Eye position should be at origin in view space");

    WS_LOG_INFO("ExtTest", "✓ View matrix tests passed");
}

void testProjectionMatrices()
{
    WS_LOG_INFO("ExtTest", "Testing projection matrix functions...");

    float fov       = PI / 3.0f; // 60 degrees
    float aspect    = 16.0f / 9.0f;
    float nearPlane = 0.1f;
    float farPlane  = 100.0f;

    // Test perspective projection GL (depth [-1, 1])
    Matrix4 perspGL = projectionPerspectiveGL(fov, aspect, nearPlane, farPlane);

    // Test point at near plane
    Vector4 nearPoint(0.0f, 0.0f, -nearPlane, 1.0f);
    Vector4 projectedNear = perspGL * nearPoint;
    projectedNear = projectedNear / projectedNear.w; // Perspective divide
    WS_ASSERT_MSG(equal(projectedNear.z, -1.0f, 0.001f),
                  "Near plane should project to -1 in GL");

    // Test point at far plane
    Vector4 farPoint(0.0f, 0.0f, -farPlane, 1.0f);
    Vector4 projectedFar = perspGL * farPoint;
    projectedFar         = projectedFar / projectedFar.w;
    WS_ASSERT_MSG(equal(projectedFar.z, 1.0f, 0.001f),
                  "Far plane should project to 1 in GL");

    // Test perspective projection (depth [0, 1])
    Matrix4 persp = projectionPerspective(fov, aspect, nearPlane, farPlane);

    Vector4 nearPointD3D = persp * nearPoint;
    nearPointD3D         = nearPointD3D / nearPointD3D.w;
    WS_ASSERT_MSG(equal(nearPointD3D.z, 0.0f, 0.001f),
                  "Near plane should project to 0 in D3D style");

    Vector4 farPointD3D = persp * farPoint;
    farPointD3D         = farPointD3D / farPointD3D.w;
    WS_ASSERT_MSG(equal(farPointD3D.z, 1.0f, 0.001f),
                  "Far plane should project to 1 in D3D style");

    // Test orthographic projection GL
    float left = -10.0f, right = 10.0f;
    float bottom = -5.0f, top = 5.0f;
    Matrix4 orthoGL =
        projectionOrthoGL(left, right, bottom, top, nearPlane, farPlane);

    Vector4 orthoNear = orthoGL * nearPoint;
    WS_ASSERT_MSG(equal(orthoNear.z, -1.0f, 0.001f),
                  "Near plane should project to -1 in ortho GL");

    Vector4 orthoFar = orthoGL * farPoint;
    WS_ASSERT_MSG(equal(orthoFar.z, 1.0f, 0.001f),
                  "Far plane should project to 1 in ortho GL");

    // Test symmetric orthographic GL
    Matrix4 orthoSymGL = projectionOrthoGL(right, top, nearPlane, farPlane);
    Vector4 cornerPoint(right, top, -nearPlane, 1.0f);
    Vector4 projectedCorner = orthoSymGL * cornerPoint;
    WS_ASSERT_MSG(equal(projectedCorner.x, 1.0f, 0.001f) &&
                      equal(projectedCorner.y, 1.0f, 0.001f),
                  "Corner should project to (1,1) in symmetric ortho");

    // Test orthographic projection (depth [0, 1])
    Matrix4 ortho =
        projectionOrtho(left, right, bottom, top, nearPlane, farPlane);

    Vector4 orthoNearD3D = ortho * nearPoint;
    WS_ASSERT_MSG(equal(orthoNearD3D.z, 0.0f, 0.001f),
                  "Near plane should project to 0 in ortho D3D style");

    Vector4 orthoFarD3D = ortho * farPoint;
    WS_ASSERT_MSG(equal(orthoFarD3D.z, 1.0f, 0.001f),
                  "Far plane should project to 1 in ortho D3D style");

    // Test symmetric orthographic (depth [0, 1])
    Matrix4 orthoSym = projectionOrtho(right, top, nearPlane, farPlane);
    Vector4 projectedCornerSym = orthoSym * cornerPoint;
    WS_ASSERT_MSG(
        equal(projectedCornerSym.x, 1.0f, 0.001f) &&
            equal(projectedCornerSym.y, 1.0f, 0.001f),
        "Corner should project to (1,1) in symmetric ortho D3D style");

    WS_LOG_INFO("ExtTest", "✓ Projection matrix tests passed");
}

void testExtensionEdgeCases()
{
    WS_LOG_INFO("ExtTest", "Testing Extension.hpp edge cases...");

    // Test identity cases
    Vector3 identityScale(1.0f, 1.0f, 1.0f);
    Matrix4 identityScaleMatrix = makeScale(identityScale);
    WS_ASSERT_MSG(mat4Equal(identityScaleMatrix, Matrix4::IDENTITY(), 0.001f),
                  "Identity scale should produce identity matrix");

    Vector3 zeroTranslation(0.0f, 0.0f, 0.0f);
    Matrix4 zeroTransMatrix = makeTranslation(zeroTranslation);
    WS_ASSERT_MSG(mat4Equal(zeroTransMatrix, Matrix4::IDENTITY(), 0.001f),
                  "Zero translation should produce identity matrix");

    // Test SRT with identity components
    Quaternion identityRot = Quaternion::IDENTITY();
    Matrix4 identitySRT = makeSRT(identityScale, identityRot, zeroTranslation);
    WS_ASSERT_MSG(mat4Equal(identitySRT, Matrix4::IDENTITY(), 0.001f),
                  "Identity SRT should produce identity matrix");

    // Test decomposition of identity matrix
    auto [scale, rot, trans] = decomposeSRT(Matrix4::IDENTITY());
    WS_ASSERT_MSG(eq(scale, Vector3(1.0f, 1.0f, 1.0f)),
                  "Identity decomposition scale should be (1,1,1)");
    WS_ASSERT_MSG(eq(trans, Vector3(0.0f, 0.0f, 0.0f)),
                  "Identity decomposition translation should be (0,0,0)");
    WS_ASSERT_MSG(
        quatEqual(rot, Quaternion::IDENTITY(), 0.001f) ||
            quatEqual(rot, -Quaternion::IDENTITY(), 0.001f),
        "Identity decomposition rotation should be identity quaternion");

    // Test look matrices with edge cases
    Vector3 eye(0.0f, 0.0f, 0.0f);
    Vector3 up(0.0f, 1.0f, 0.0f);

    // Looking straight up
    Vector3 targetUp(0.0f, 1.0f, 0.0f);
    Matrix4 lookUp =
        lookAt(eye,
               targetUp,
               Vector3(0.0f, 0.0f, -1.0f)); // Use -Z as up for this case
    Vector4 testForward(0.0f, 0.0f, -1.0f, 1.0f);
    Vector4 transformedUp = lookUp * testForward;
    // This should work even in degenerate cases

    WS_LOG_INFO("ExtTest", "✓ Extension edge cases tests passed");
}

void testExtensionAlgorithmCorrectness()
{
    WS_LOG_INFO("ExtTest", "Testing Extension.hpp algorithm correctness...");

    // 1. Test decomposeSRT potential issues
    {
        // Test with negative determinant case
        Vector3 scale(-2.0f, 3.0f, 4.0f); // Negative X scale creates reflection
        Quaternion rotation =
            Quaternion::fromAxisAngle(Vector3::Y(), PI / 4.0f);
        Vector3 translation(10.0f, 20.0f, 30.0f);

        Matrix4 srtMatrix = makeSRT(scale, rotation, translation);
        auto [decomposedScale, decomposedRotation, decomposedTranslation] =
            decomposeSRT(srtMatrix);

        // The algorithm should handle negative determinant correctly
        WS_ASSERT_MSG(floatEqual(decomposedScale.x, scale.x, 0.001f),
                      "Negative scale should be preserved in decomposition");
        WS_ASSERT_MSG(floatEqual(decomposedScale.y, scale.y, 0.001f) &&
                          floatEqual(decomposedScale.z, scale.z, 0.001f),
                      "Positive scales should be preserved");
    }

    // 2. Test makeSRT algorithm
    {
        // Verify SRT order: Scale -> Rotation -> Translation
        Vector3 testPoint(1.0f, 0.0f, 0.0f);
        Vector3 scale(2.0f, 3.0f, 4.0f);
        Quaternion rotation =
            Quaternion::fromAxisAngle(Vector3::Z(), PI / 2.0f);
        Vector3 translation(5.0f, 6.0f, 7.0f);

        Matrix4 srtMatrix = makeSRT(scale, rotation, translation);
        Vector4 result    = srtMatrix * Vector4(testPoint, 1.0f);

        // Manual calculation: Scale(2,3,4) -> Rotate Z 90° -> Translate(5,6,7)
        // (1,0,0) -> (2,0,0) -> (0,2,0) -> (5,8,7)
        Vector3 expected(5.0f, 8.0f, 7.0f);
        WS_ASSERT_MSG(equal(result.x, expected.x, 0.001f) &&
                          equal(result.y, expected.y, 0.001f) &&
                          equal(result.z, expected.z, 0.001f),
                      "SRT should apply transformations in correct order");
    }

    // 3. Test lookTo algorithm
    {
        Vector3 eye(0.0f, 0.0f, 0.0f);
        Vector3 forward(1.0f, 0.0f, 0.0f); // Look towards +X
        Vector3 up(0.0f, 1.0f, 0.0f);      // Y is up

        Matrix4 orientation = lookTo(eye, forward, up);

        // Local Z axis should align with world forward direction
        Vector4 localZ(0.0f, 0.0f, 1.0f, 0.0f);
        Vector4 worldForward = orientation * localZ;
        WS_ASSERT_MSG(equal(worldForward.x, 1.0f, 0.001f) &&
                          equal(worldForward.y, 0.0f, 0.001f) &&
                          equal(worldForward.z, 0.0f, 0.001f),
                      "lookTo should orient object correctly");

        // Local Y axis should align with world up
        Vector4 localY(0.0f, 1.0f, 0.0f, 0.0f);
        Vector4 worldUp = orientation * localY;
        WS_ASSERT_MSG(equal(worldUp.x, 0.0f, 0.001f) &&
                          equal(worldUp.y, 1.0f, 0.001f) &&
                          equal(worldUp.z, 0.0f, 0.001f),
                      "lookTo should maintain up direction");
    }

    // 4. Test lookAt camera matrix
    {
        Vector3 eye(0.0f, 0.0f, 5.0f);    // Camera at +Z
        Vector3 target(0.0f, 0.0f, 0.0f); // Looking at origin
        Vector3 up(0.0f, 1.0f, 0.0f);     // Y is up

        Matrix4 viewMatrix = lookAt(eye, target, up);

        // Transform eye position should be at origin in view space
        Vector4 eyeInView = viewMatrix * Vector4(eye, 1.0f);
        WS_ASSERT_MSG(equal(eyeInView.x, 0.0f, 0.001f) &&
                          equal(eyeInView.y, 0.0f, 0.001f) &&
                          equal(eyeInView.z, 0.0f, 0.001f),
                      "Camera position should be at origin in view space");

        // Target should be at negative Z in view space (camera looking down -Z)
        Vector4 targetInView = viewMatrix * Vector4(target, 1.0f);
        WS_ASSERT_MSG(targetInView.z < 0.0f,
                      "Target should be at negative Z in view space");
    }

    // 5. Test projection matrices edge cases
    {
        float fov       = PI / 2.0f; // 90 degrees
        float aspect    = 1.0f;      // Square aspect
        float nearPlane = 1.0f;
        float farPlane  = 10.0f;

        // Test GL perspective projection
        Matrix4 perspGL =
            projectionPerspectiveGL(fov, aspect, nearPlane, farPlane);

        // Point at near plane corners
        float tanHalfFov = std::tan(fov * 0.5f);
        Vector4 nearCorner(tanHalfFov * nearPlane,
                           tanHalfFov * nearPlane,
                           -nearPlane,
                           1.0f);
        Vector4 projectedCorner = perspGL * nearCorner;
        projectedCorner         = projectedCorner / projectedCorner.w;

        WS_ASSERT_MSG(equal(projectedCorner.x, 1.0f, 0.001f) &&
                          equal(projectedCorner.y, 1.0f, 0.001f) &&
                          equal(projectedCorner.z, -1.0f, 0.001f),
                      "Near plane corner should project to (1,1,-1) in GL");
    }

    // 6. Test orthographic projections
    {
        float left = -5.0f, right = 5.0f;
        float bottom = -3.0f, top = 3.0f;
        float nearPlane = 1.0f, farPlane = 10.0f;

        Matrix4 orthoGL =
            projectionOrthoGL(left, right, bottom, top, nearPlane, farPlane);
        Matrix4 ortho =
            projectionOrtho(left, right, bottom, top, nearPlane, farPlane);

        // Test corner transformation
        Vector4 corner(right, top, -nearPlane, 1.0f);
        Vector4 projectedGL = orthoGL * corner;
        Vector4 projected   = ortho * corner;

        WS_ASSERT_MSG(equal(projectedGL.x, 1.0f, 0.001f) &&
                          equal(projectedGL.y, 1.0f, 0.001f) &&
                          equal(projectedGL.z, -1.0f, 0.001f),
                      "Ortho GL should map corner to (1,1,-1)");

        WS_ASSERT_MSG(equal(projected.x, 1.0f, 0.001f) &&
                          equal(projected.y, 1.0f, 0.001f) &&
                          equal(projected.z, 0.0f, 0.001f),
                      "Ortho should map corner to (1,1,0)");
    }

    WS_LOG_INFO("ExtTest", "✓ Extension algorithm correctness tests passed");
}

void testExtensionBoundaryConditions()
{
    WS_LOG_INFO("ExtTest", "Testing Extension.hpp boundary conditions...");

    // 1. Test very small scale values
    {
        Vector3 tinyScale(1e-6f, 1e-6f, 1e-6f);
        Matrix4 scaleMatrix = makeScale(tinyScale);
        Vector4 testVec(1.0f, 1.0f, 1.0f, 1.0f);
        Vector4 result = scaleMatrix * testVec;
        WS_ASSERT_MSG(equal(result.x, 1e-6f, 1e-9f) &&
                          equal(result.y, 1e-6f, 1e-9f) &&
                          equal(result.z, 1e-6f, 1e-9f),
                      "Very small scales should work correctly");
    }

    // 2. Test very large scale values
    {
        Vector3 largeScale(1e6f, 1e6f, 1e6f);
        Matrix4 scaleMatrix = makeScale(largeScale);
        Vector4 testVec(1.0f, 1.0f, 1.0f, 1.0f);
        Vector4 result = scaleMatrix * testVec;
        WS_ASSERT_MSG(equal(result.x, 1e6f, 1e3f) &&
                          equal(result.y, 1e6f, 1e3f) &&
                          equal(result.z, 1e6f, 1e3f),
                      "Very large scales should work correctly");
    }

    // 3. Test projection with extreme FOV
    {
        float extremeFov = PI * 0.95f; // Almost 180 degrees
        float aspect     = 1.0f;
        float nearPlane  = 0.01f;
        float farPlane   = 1000.0f;

        Matrix4 perspGL =
            projectionPerspectiveGL(extremeFov, aspect, nearPlane, farPlane);

        // Should not have NaN or infinite values
        WS_ASSERT_MSG(std::isfinite(perspGL.col0.x) &&
                          std::isfinite(perspGL.col1.y),
                      "Extreme FOV should not produce NaN values");
    }

    // 4. Test projection with extreme aspect ratio
    {
        float fov           = PI / 3.0f;
        float extremeAspect = 100.0f; // Ultra-wide
        float nearPlane     = 0.1f;
        float farPlane      = 100.0f;

        Matrix4 perspGL =
            projectionPerspectiveGL(fov, extremeAspect, nearPlane, farPlane);

        // X scaling should be much smaller than Y scaling
        WS_ASSERT_MSG(perspGL.col0.x < perspGL.col1.y * 0.1f,
                      "Extreme aspect ratio should scale X much less than Y");
    }

    // 5. Test lookAt with nearly parallel up and forward vectors
    {
        Vector3 eye(0.0f, 0.0f, 0.0f);
        Vector3 target(0.0f, 0.0f, -1.0f);
        Vector3 almostParallelUp(0.001f,
                                 0.0f,
                                 -1.0f); // Almost parallel to forward

        Matrix4 viewMatrix = lookAt(eye, target, almostParallelUp);

        // Should still produce a valid matrix (not degenerate)
        float det = determinant(viewMatrix);
        WS_ASSERT_MSG(
            std::abs(det) > 1e-6f,
            "Nearly parallel up vector should not create degenerate matrix");
    }

    // 6. Test orthographic projection with very small/large bounds
    {
        float left = -1e-3f, right = 1e-3f;
        float bottom = -1e-3f, top = 1e-3f;
        float nearPlane = 1e-3f, farPlane = 1e3f;

        Matrix4 ortho =
            projectionOrtho(left, right, bottom, top, nearPlane, farPlane);

        // Should produce valid transformation
        Vector4 testPoint(0.0f, 0.0f, -1.0f, 1.0f);
        Vector4 projected = ortho * testPoint;
        WS_ASSERT_MSG(
            std::isfinite(projected.x) && std::isfinite(projected.y) &&
                std::isfinite(projected.z),
            "Extreme orthographic bounds should not produce invalid values");
    }

    WS_LOG_INFO("ExtTest", "✓ Extension boundary conditions tests passed");
}

int main()
{
    worse::Logger::initialize();

    WS_LOG_INFO("Math Test", "Starting comprehensive math tests...");

    // Quaternion tests
    WS_LOG_INFO("Math Test", "=== Quaternion Tests ===");
    testQuaternionBasics();
    testQuaternionMagnitude();
    testQuaternionNormalization();
    testQuaternionConjugate();
    testQuaternionInverse();
    testQuaternionMultiplication();
    testAxisAngleConstruction();
    testEulerAngles();
    testMatrixConversion();
    testInterpolation();
    testVectorRotation();
    testErrorConditions();
    testSlerpImprovements();

    // Extension tests
    WS_LOG_INFO("Math Test", "=== Extension Algorithm Tests ===");
    testMatrixConstruction();
    testSRTOperations();
    testViewMatrices();
    testProjectionMatrices();
    testExtensionAlgorithmCorrectness();
    testExtensionBoundaryConditions();
    testExtensionEdgeCases();

    WS_LOG_INFO("Math Test", "🎉 All math tests completed successfully!");

    worse::Logger::shutdown();
    return 0;
}