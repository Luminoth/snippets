#include "src/pch.h"
#include "math_util.h"
#include "Matrix4.h"

namespace energonsoftware {

void Matrix4::destroy(Matrix4* const matrix, MemoryAllocator* const allocator)
{
    matrix->~Matrix4();
    operator delete(matrix, 16, *allocator);
}

Matrix4 Matrix4::perspective(float fov, float aspect, float n, float f)
{
    assert(aspect != 0.0f);
    assert(n != 0 && n != f);

    const float x = (DEG_RAD(fov) / 2.0f);
    const float t = std::tan(M_PI_2 - x);

    Matrix4 perspective;
    perspective[0] = t / aspect;
    perspective[5] = t;
    perspective[10] = (f + n) / (n - f);
    perspective[11] = (2.0f * f * n) / (n - f);
    perspective[14] = -1.0f;
    perspective[15] = 0.0f;
    return perspective;
}

Matrix4 Matrix4::orthographic(float left, float right, float bottom, float top, float n, float f)
{
    assert(left != right);
    assert(top != bottom);
    assert(n != 0 && n != f);

    const float tx = -((right + left) / (right - left));
    const float ty = -((top + bottom) / (top - bottom));
    const float tz = -((f + n) / (f - n));

    Matrix4 orthographic;
    orthographic[0] = 2.0f / (right - left);
    orthographic[3] = tx;
    orthographic[5] = 2.0f / (top - bottom);
    orthographic[7] = ty;
    orthographic[10] = -2.0f / (f - n);
    orthographic[11] = tz;
    return orthographic;
}

Matrix4 Matrix4::frustum(float left, float right, float bottom, float top, float n, float f)
{
    assert(left != right);
    assert(top != bottom);
    assert(n != 0.0f && n != f);

    Matrix4 frustum;
    frustum[0] = (2.0f * n) / (right - left);
    frustum[2] = (right + left) / (right - left);
    frustum[5] = (2.0f * n) / (top - bottom);
    frustum[6] = (top + bottom) / (top - bottom);
    frustum[10] = -((f + n) / (f - n));
    frustum[11] = -((2.0f * f * n) / (f - n));
    frustum[14] = -1.0f;
    frustum[15] = 0.0f;
    return frustum;
}

Matrix4 Matrix4::infinite_frustum(float left, float right, float bottom, float top, float n)
{
    assert(left != right);
    assert(top != bottom);
    assert(n != 0.0f);

    Matrix4 frustum;
    frustum[0] = (2.0f * n) / (right - left);
    frustum[2] = (right + left) / (right - left);
    frustum[5] = (2.0f * n) / (top - bottom);
    frustum[6] = (top + bottom) / (top - bottom);
    frustum[10] = -1;
    frustum[11] = -2.0f * n;
    frustum[14] = -1.0f;
    frustum[15] = 0.0f;
    return frustum;
}

Matrix4& Matrix4::rotate(float angle, const Vector3& around)
{
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    const Vector3 a(around.normalized());

    const float c1 = 1.0f - c;

    Matrix4 matrix;
    matrix._m[0] = c + c1 * a.x() * a.x();
    matrix._m[1] = c1 * a.x() * a.y() - s * a.z();
    matrix._m[2] = c1 * a.x() * a.z() + s * a.y();

    matrix._m[4] = c1 * a.x() * a.y() + s * a.z();
    matrix._m[5] = c + c1 * a.y() * a.y();
    matrix._m[6] = c1 * a.x() * a.z() - s * a.x();

    matrix._m[8] = c1 * a.x() * a.z() - s * a.y();
    matrix._m[9] = c1 * a.y() * a.z() + s * a.x();
    matrix._m[10] = c + c1 * a.z() * a.z();
    return (*this) *= matrix;
}

std::string Matrix4::str() const
{
    std::stringstream ss;
    ss << _m[0] << " " << _m[1] << " " << _m[2] << " " << _m[3] << "\n"
       << _m[4] << " " << _m[5] << " " << _m[6] << " " << _m[7] << "\n"
       << _m[8] << " " << _m[9] << " " << _m[10] << " " << _m[11] << "\n"
       << _m[12] << " " << _m[13] << " " << _m[14] << " " << _m[15];
    return ss.str();
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class Matrix4Test : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(Matrix4Test);
        CPPUNIT_TEST(test_perspective);
        CPPUNIT_TEST(test_orthographic);
        CPPUNIT_TEST(test_frustum);
        CPPUNIT_TEST(test_infinite_frustum);
        CPPUNIT_TEST(test_allocation);
        CPPUNIT_TEST(test_initialize);
        CPPUNIT_TEST(test_row);
        CPPUNIT_TEST(test_determinant);
        CPPUNIT_TEST(test_normal_matrix);
        CPPUNIT_TEST(test_translate);
        CPPUNIT_TEST(test_rotate);
        CPPUNIT_TEST(test_scale);
        CPPUNIT_TEST(test_uniform_scale);
        CPPUNIT_TEST(test_addition);
        CPPUNIT_TEST(test_subtraction);
        CPPUNIT_TEST(test_multiply);
        CPPUNIT_TEST(test_vector_multiply);
        CPPUNIT_TEST(test_scalar_multiply);
        CPPUNIT_TEST(test_scalar_divide);
        CPPUNIT_TEST(test_equality);
        CPPUNIT_TEST(test_inverse);
        CPPUNIT_TEST(test_transpose);
    CPPUNIT_TEST_SUITE_END();

public:
    Matrix4Test() : CppUnit::TestFixture() {}
    virtual ~Matrix4Test() noexcept {}

public:
    void test_perspective()
    {
        static const energonsoftware::Matrix4 m1(energonsoftware::Matrix4::perspective(91.5f, 1024.25f / 861.2f));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.8191f, m1(0, 0), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.9742f, m1(1, 1), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0002f, m1(2, 2), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.2000f, m1(2, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(-1.0f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 3));

        static const energonsoftware::Matrix4 m2(energonsoftware::Matrix4::perspective(76.5f, 1281.57f / 975.3f, 13.5f, 125.6f));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.9653f, m2(0, 0), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(0, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(0, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(0, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(1, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.2685f, m2(1, 1), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(1, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(1, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(2, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.2409f, m2(2, 2), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-30.2516f, m2(2, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(3, 1));
        CPPUNIT_ASSERT_EQUAL(-1.0f, m2(3, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(3, 3));
    }

    void test_orthographic()
    {
        static const energonsoftware::Matrix4 m1(energonsoftware::Matrix4::orthographic(12.4f, 100.24f, 55.0f, 4000.0f));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0228f, m1(0, 0), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.2823f, m1(0, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0005f, m1(1, 1), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0279f, m1(1, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0f, m1(2, 2), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0f, m1(2, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(3, 3));

        static const energonsoftware::Matrix4 m2(energonsoftware::Matrix4::orthographic(57.213f, 234.23f, 12.23f, 1245.4f, -13.5f, 125.6f));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0113f, m2(0, 0), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(0, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(0, 2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.6464f, m2(0, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(1, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0016f, m2(1, 1), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(1, 2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0198f, m2(1, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(2, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.0144f, m2(2, 2), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.8059f, m2(2, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(3, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(3, 2));
        CPPUNIT_ASSERT_EQUAL(1.0f, m2(3, 3));
    }

    void test_frustum()
    {
        static const energonsoftware::Matrix4 m1(energonsoftware::Matrix4::frustum(23.525f, 235.524f, 23.235f, 2334.55f));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0009f, m1(0, 0), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.2219f, m1(0, 2), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0001f, m1(1, 1), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0201f, m1(1, 2), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0002f, m1(2, 2), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.2000f, m1(2, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(-1.0f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 3));

        static const energonsoftware::Matrix4 m2(energonsoftware::Matrix4::frustum(2.325f, 235.535f, 35.535f, 3525.35f, -123.34f, 1245.23f));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-1.0578f, m2(0, 0), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(0, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0199f, m2(0, 2), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(0, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(1, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.07069f, m2(1, 1), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0204f, m2(1, 2), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(1, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(2, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.8198f, m2(2, 2), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(224.4483f, m2(2, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(3, 1));
        CPPUNIT_ASSERT_EQUAL(-1.0f, m2(3, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(3, 3));
    }

    void test_infinite_frustum()
    {
        static const energonsoftware::Matrix4 m1(energonsoftware::Matrix4::infinite_frustum(26.345f, 3754.3f, 26.63f, 663.36f));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0001f, m1(0, 0), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0141f, m1(0, 2), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0003f, m1(1, 1), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0836f, m1(1, 2), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 1));
        CPPUNIT_ASSERT_EQUAL(-1.0f, m1(2, 2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-0.2f, m1(2, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(-1.0f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 3));

        static const energonsoftware::Matrix4 m2(energonsoftware::Matrix4::infinite_frustum(27.74f, 457.36f, 43.643f, 3745.345f, 23.364f));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1088f, m2(0, 0), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(0, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.1291f, m2(0, 2), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(0, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(1, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0126f, m2(1, 1), 0.0001f);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0236f, m2(1, 2), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(1, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(2, 1));
        CPPUNIT_ASSERT_EQUAL(-1.0f, m2(2, 2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(-46.728f, m2(2, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(3, 1));
        CPPUNIT_ASSERT_EQUAL(-1.0f, m2(3, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m2(3, 3));
    }

    void test_allocation()
    {
        std::shared_ptr<energonsoftware::MemoryAllocator> allocator(energonsoftware::MemoryAllocator::new_allocator(energonsoftware::MemoryAllocator::Type::System, 10 * 1024));
        std::shared_ptr<energonsoftware::Matrix4> m1(new(16, *allocator) energonsoftware::Matrix4(),
            std::bind(energonsoftware::Matrix4::destroy, std::placeholders::_1, allocator.get()));
        CPPUNIT_ASSERT(m1);
        CPPUNIT_ASSERT(m1->is_identity());

        m1.reset();
    }

    void test_initialize()
    {
    }

    void test_row()
    {
        static const energonsoftware::Matrix4 m1;
        energonsoftware::Vector r1 = m1.row(1);
        CPPUNIT_ASSERT_EQUAL(0.0f, r1.x());
        CPPUNIT_ASSERT_EQUAL(1.0f, r1.y());
        CPPUNIT_ASSERT_EQUAL(0.0f, r1.z());
        CPPUNIT_ASSERT_EQUAL(0.0f, r1.w());
    }

    void test_determinant()
    {
        static const energonsoftware::Matrix4 m1;
        CPPUNIT_ASSERT_EQUAL(1.0f, m1.determinant());

#if !defined _MSC_VER || (defined _MSC_VER && _MSC_VER > 1700)
        static const energonsoftware::Matrix4 m2({
            1.0f, 2.0f, 3.0f, 9.0f,
            4.0f, 5.0f, 4.0f, 8.0f,
            3.0f, 2.0f, 1.0f, 7.0f,
            6.0f, 7.0f, 8.0f, 9.0f
        });
        CPPUNIT_ASSERT_EQUAL(160.0f, m2.determinant());
#endif

        static const float M3_MATRIX[] = { 1.0f, 2.0f, 3.0f, 9.0f, 4.0f, 5.0f, 4.0f, 8.0f, 3.0f, 2.0f, 1.0f, 7.0f, 6.0f, 7.0f, 8.0f, 9.0f };
        static const energonsoftware::Matrix4 m3(M3_MATRIX);
        CPPUNIT_ASSERT_EQUAL(160.0f, m3.determinant());
    }

    void test_normal_matrix()
    {
    }

    void test_translate()
    {
        static const energonsoftware::Position p(123.35f, 345.35f, 312.3f);
        energonsoftware::Matrix4 m1;
        m1.translate(p);
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(0, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(p.x(), m1(0, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 0));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(1, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(p.y(), m1(1, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 1));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(2, 2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(p.z(), m1(2, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(3, 3));

        m1.translate(p);
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(0, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f * p.x(), m1(0, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 0));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(1, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f * p.y(), m1(1, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 1));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(2, 2));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0f * p.z(), m1(2, 3), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(3, 3));
    }

    void test_rotate()
    {
    }

    void test_scale()
    {
        static const energonsoftware::Vector3 s1(2.0f, 3.0f, 4.0f);
        energonsoftware::Matrix4 m1;
        m1.scale(s1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(s1.x(), m1(0, 0), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(s1.y(), m1(1, 1), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(s1.z(), m1(2, 2), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(3, 3));

        m1.scale(s1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(s1.x() * s1.x(), m1(0, 0), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(s1.y() * s1.y(), m1(1, 1), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(s1.z() * s1.z(), m1(2, 2), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(3, 3));
    }

    void test_uniform_scale()
    {
        static const float s1 = 4.0f;
        energonsoftware::Matrix4 m1;
        m1.uniform_scale(s1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(s1, m1(0, 0), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(s1, m1(1, 1), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(s1, m1(2, 2), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(3, 3));

        m1.uniform_scale(s1);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(s1 * s1, m1(0, 0), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(0, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 0));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(s1 * s1, m1(1, 1), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 2));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(1, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 1));
        CPPUNIT_ASSERT_DOUBLES_EQUAL(s1 * s1, m1(2, 2), 0.0001f);
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(2, 3));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(0.0f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(3, 3));
    }

    void test_addition()
    {
        static const float MATRIX[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f };

        energonsoftware::Matrix4 m1 = energonsoftware::Matrix4(MATRIX) + energonsoftware::Matrix4(MATRIX);
        CPPUNIT_ASSERT_EQUAL(2.0f, m1(0, 0));
        CPPUNIT_ASSERT_EQUAL(4.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(6.0f, m1(0, 2));
        CPPUNIT_ASSERT_EQUAL(8.0f, m1(0, 3));
        CPPUNIT_ASSERT_EQUAL(10.0f, m1(1, 0));
        CPPUNIT_ASSERT_EQUAL(12.0f, m1(1, 1));
        CPPUNIT_ASSERT_EQUAL(14.0f, m1(1, 2));
        CPPUNIT_ASSERT_EQUAL(16.0f, m1(1, 3));
        CPPUNIT_ASSERT_EQUAL(18.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(20.0f, m1(2, 1));
        CPPUNIT_ASSERT_EQUAL(22.0f, m1(2, 2));
        CPPUNIT_ASSERT_EQUAL(24.0f, m1(2, 3));
        CPPUNIT_ASSERT_EQUAL(26.0f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(28.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(30.0f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(32.0f, m1(3, 3));

// TODO: test +=
    }

    void test_subtraction()
    {
        static const float MATRIX[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f };

        energonsoftware::Matrix4 m1 = energonsoftware::Matrix4(MATRIX) - energonsoftware::Matrix4(MATRIX);
        CPPUNIT_ASSERT(m1.is_zero());

// TODO: test -=
    }

    void test_multiply()
    {
    }

    void test_vector_multiply()
    {
    }

    void test_scalar_multiply()
    {
        static const float MATRIX[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f };

        energonsoftware::Matrix4 m1 = 2.0f * energonsoftware::Matrix4(MATRIX);
        CPPUNIT_ASSERT_EQUAL(2.0f, m1(0, 0));
        CPPUNIT_ASSERT_EQUAL(4.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(6.0f, m1(0, 2));
        CPPUNIT_ASSERT_EQUAL(8.0f, m1(0, 3));
        CPPUNIT_ASSERT_EQUAL(10.0f, m1(1, 0));
        CPPUNIT_ASSERT_EQUAL(12.0f, m1(1, 1));
        CPPUNIT_ASSERT_EQUAL(14.0f, m1(1, 2));
        CPPUNIT_ASSERT_EQUAL(16.0f, m1(1, 3));
        CPPUNIT_ASSERT_EQUAL(18.0f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(20.0f, m1(2, 1));
        CPPUNIT_ASSERT_EQUAL(22.0f, m1(2, 2));
        CPPUNIT_ASSERT_EQUAL(24.0f, m1(2, 3));
        CPPUNIT_ASSERT_EQUAL(26.0f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(28.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(30.0f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(32.0f, m1(3, 3));

// TODO: test *=
    }

    void test_scalar_divide()
    {
        static const float MATRIX[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f };

        energonsoftware::Matrix4 m1 = energonsoftware::Matrix4(MATRIX) / 2.0f;
        CPPUNIT_ASSERT_EQUAL(0.5f, m1(0, 0));
        CPPUNIT_ASSERT_EQUAL(1.0f, m1(0, 1));
        CPPUNIT_ASSERT_EQUAL(1.5f, m1(0, 2));
        CPPUNIT_ASSERT_EQUAL(2.0f, m1(0, 3));
        CPPUNIT_ASSERT_EQUAL(2.5f, m1(1, 0));
        CPPUNIT_ASSERT_EQUAL(3.0f, m1(1, 1));
        CPPUNIT_ASSERT_EQUAL(3.5f, m1(1, 2));
        CPPUNIT_ASSERT_EQUAL(4.0f, m1(1, 3));
        CPPUNIT_ASSERT_EQUAL(4.5f, m1(2, 0));
        CPPUNIT_ASSERT_EQUAL(5.0f, m1(2, 1));
        CPPUNIT_ASSERT_EQUAL(5.5f, m1(2, 2));
        CPPUNIT_ASSERT_EQUAL(6.0f, m1(2, 3));
        CPPUNIT_ASSERT_EQUAL(6.5f, m1(3, 0));
        CPPUNIT_ASSERT_EQUAL(7.0f, m1(3, 1));
        CPPUNIT_ASSERT_EQUAL(7.5f, m1(3, 2));
        CPPUNIT_ASSERT_EQUAL(8.0f, m1(3, 3));

// TODO: test /=
    }

    void test_equality()
    {
        static const float MATRIX1[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f };
        static const float MATRIX2[] = { 16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f };
        CPPUNIT_ASSERT_EQUAL(energonsoftware::Matrix4(MATRIX1), energonsoftware::Matrix4(MATRIX1));
        CPPUNIT_ASSERT(energonsoftware::Matrix4(MATRIX1) != energonsoftware::Matrix4(MATRIX2));
    }

    void test_inverse()
    {
    }

    void test_transpose()
    {
        static const float MATRIX[] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f };
        static const float TRANSPOSE[] = { 1.0f, 5.0f, 9.0f, 13.0f, 2.0f, 6.0f, 10.0f, 14.0f, 3.0f, 7.0f, 11.0f, 15.0f, 4.0f, 8.0f, 12.0f, 16.0f };
        CPPUNIT_ASSERT_EQUAL(~(energonsoftware::Matrix4(MATRIX)), energonsoftware::Matrix4(TRANSPOSE));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Matrix4Test);

#endif
