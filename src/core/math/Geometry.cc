#include "src/pch.h"
#include "Geometry.h"

namespace energonsoftware {

Vertex::Vertex()
    : index(-1), position(), normal(), tangent(), bitangent(), texture_coords(),
        weight_start(0), weight_count(0)
{
}

std::string Vertex::str() const
{
    std::stringstream ss;
    ss << "Vertex(index: " << index << ", position:" << position.str() << ", texture_coords: " << texture_coords.str() << ")";
    return ss.str();
}

Triangle::Triangle()
    : index(-1), v1(-1), v2(-1), v3(-1),
        normal()
{
}

Weight::Weight()
    : index(-1), joint(-1), weight(0.0f),
        position(), normal(), tangent(), bitangent()
{
}

Edge::Edge()
    : v1(-1), v2(-1), t1(-1), t2(-1)
{
}

std::string Edge::str() const
{
    std::stringstream ss;
    ss << "Edge(v1: " << v1 << ", v2:" << v2 << ", t1: " << t1 << ", t2: " << t2 << ")";
    return ss.str();
}

void compute_tangents(Triangle* const triangles, size_t triangle_count, Vertex* const vertices, size_t vertex_count, MemoryAllocator& allocator, bool smooth)
{
    std::shared_ptr<Vector3> narray(MemoryAllocator_new_aligned<Vector3, 16>(vertex_count, allocator),
        MemoryAllocator_delete_aligned<Vector[], 16>(vertex_count, &allocator));
    std::shared_ptr<Vector3> tarray(MemoryAllocator_new_aligned<Vector3, 16>(vertex_count, allocator),
        MemoryAllocator_delete_aligned<Vector[], 16>(vertex_count, &allocator));
    std::shared_ptr<Vector3> btarray(MemoryAllocator_new_aligned<Vector3, 16>(vertex_count, allocator),
        MemoryAllocator_delete_aligned<Vector[], 16>(vertex_count, &allocator));

    // calculate the vertex normals and tangents (sum of each face normal/tangent)
    // Mathematics for 3D Game Programming and Computer Graphics, section 7.8.3
    for(size_t i=0; i<triangle_count; ++i) {
        Triangle& triangle(triangles[i]);
        const Vertex &p0(vertices[triangle.v1]), &p1(vertices[triangle.v2]), &p2(vertices[triangle.v3]);

        const int idx0 = p0.index, idx1 = p1.index, idx2 = p2.index;

        // face normal
        const Vector3 q1(p1.position - p0.position), q2(p2.position - p0.position);
        Vector3 normal(q1 ^ q2);
        if(smooth) {
            normal.normalize();
        }
        triangle.normal = normal.normalized();

        narray.get()[idx0] += normal;
        narray.get()[idx1] += normal;
        narray.get()[idx2] += normal;

        const float s1 = p1.texture_coords.x() - p0.texture_coords.x(), t1 = p1.texture_coords.y() - p0.texture_coords.y();
        const float s2 = p2.texture_coords.x() - p0.texture_coords.x(), t2 = p2.texture_coords.y() - p0.texture_coords.y();

        // face tangent (coefficient times the texture matrix times the position matrix)
        const float coef = 1.0f / (s1 * t2 - s2 * t1);
        Vector3 tangent(coef * Vector3(t2 * q1.x() - t1 * q2.x(), t2 * q1.y() - t1 * q2.y(), t2 * q1.z() - t1 * q2.z()));
        if(smooth) {
            tangent.normalize();
        }

        // Gram-Schmidt orthogonalize the tangent
        Vector3 object_tangent(tangent - (normal * tangent) * normal);
        if(smooth) {
            object_tangent.normalize();
        }

        tarray.get()[idx0] += object_tangent;
        tarray.get()[idx1] += object_tangent;
        tarray.get()[idx2] += object_tangent;

        Vector3 bitangent(coef * Vector3(s1 * q2.x() - s2 * q1.x(), s1 * q2.y() - s2 * q1.y(), s1 * q2.z() - s2 * q1.z()));
        if(smooth) {
            bitangent.normalize();
        }

        btarray.get()[idx0] += bitangent;
        btarray.get()[idx1] += bitangent;
        btarray.get()[idx2] += bitangent;
    }

    // store the vertex data
    for(size_t i=0; i<vertex_count; ++i) {
        Vertex& vertex(vertices[i]);
        vertex.normal = narray.get()[i].normalized();
        vertex.tangent = tarray.get()[i].normalized();
        vertex.bitangent = btarray.get()[i].normalized();
    }
}

Geometry::Geometry(size_t vertex_count, MemoryAllocator& allocator)
    : _mutex(), _vertex_count(0),
        _vertex_buffer_size(0), _vertex_buffer(),
        _normal_buffer_size(0), _normal_buffer(),
        _tangent_buffer_size(0), _tangent_buffer(),
        _texture_buffer_size(0), _texture_buffer(),
        _normal_line_buffer(), _tangent_line_buffer()
{
    allocate_buffers(vertex_count, allocator);
}

Geometry::Geometry(const Vertex* const vertices, size_t vertex_count, MemoryAllocator& allocator)
    : _mutex(), _vertex_count(0),
        _vertex_buffer_size(0), _vertex_buffer(),
        _normal_buffer_size(0), _normal_buffer(),
        _tangent_buffer_size(0), _tangent_buffer(),
        _texture_buffer_size(0), _texture_buffer(),
        _normal_line_buffer(), _tangent_line_buffer()
{
    allocate_buffers(vertex_count, allocator);
    copy_vertices(vertices, vertex_count, 0);
}

Geometry::Geometry(size_t triangle_count, size_t vertex_count, MemoryAllocator& allocator)
    : _mutex(), _vertex_count(0),
        _vertex_buffer_size(0), _vertex_buffer(),
        _normal_buffer_size(0), _normal_buffer(),
        _tangent_buffer_size(0), _tangent_buffer(),
        _texture_buffer_size(0), _texture_buffer(),
        _normal_line_buffer(), _tangent_line_buffer()
{
    allocate_buffers(triangle_count, vertex_count, allocator);
}

Geometry::Geometry(const Triangle* const triangles, size_t triangle_count, const Vertex* const vertices, size_t vertex_count, MemoryAllocator& allocator)
    : _mutex(), _vertex_count(0),
        _vertex_buffer_size(0), _vertex_buffer(),
        _normal_buffer_size(0), _normal_buffer(),
        _tangent_buffer_size(0), _tangent_buffer(),
        _texture_buffer_size(0), _texture_buffer(),
        _normal_line_buffer(), _tangent_line_buffer()
{
    allocate_buffers(triangle_count, vertex_count, allocator);
    copy_triangles(triangles, triangle_count, vertices, vertex_count, 0);
}

Geometry::~Geometry() noexcept
{
}

void Geometry::allocate_buffers(size_t vertex_count, MemoryAllocator& allocator)
{
    std::lock_guard<std::recursive_mutex> guard(_mutex);

    _vertex_count = vertex_count;

    _vertex_buffer_size = _vertex_count * 3;
    _vertex_buffer.reset(new(allocator) float[_vertex_buffer_size], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));

    _normal_buffer_size = _vertex_count * 3;
    _normal_buffer.reset(new(allocator) float[_normal_buffer_size], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));
    _normal_line_buffer.reset(new(allocator) float[_vertex_count * 2 * 3], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));

    _tangent_buffer_size = _vertex_count * 4;
    _tangent_buffer.reset(new(allocator) float[_tangent_buffer_size], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));
    _tangent_line_buffer.reset(new(allocator) float[_vertex_count * 2 * 3], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));

    _texture_buffer_size = _vertex_count * 2;
    _texture_buffer.reset(new(allocator) float[_texture_buffer_size], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));
}

void Geometry::allocate_buffers(size_t triangle_count, size_t vertex_count, MemoryAllocator& allocator)
{
    std::lock_guard<std::recursive_mutex> guard(_mutex);

    _vertex_count = triangle_count * 3;

    _vertex_buffer_size = _vertex_count * 3;
    _vertex_buffer.reset(new(allocator) float[_vertex_buffer_size], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));

    _normal_buffer_size = _vertex_count * 3;
    _normal_buffer.reset(new(allocator) float[_normal_buffer_size], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));
    _normal_line_buffer.reset(new(allocator) float[_vertex_count * 2 * 3], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));

    _tangent_buffer_size = _vertex_count * 4;
    _tangent_buffer.reset(new(allocator) float[_tangent_buffer_size], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));
    _tangent_line_buffer.reset(new(allocator) float[_vertex_count * 2 * 3], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));

    _texture_buffer_size = _vertex_count * 2;
    _texture_buffer.reset(new(allocator) float[_tangent_buffer_size], std::bind(&MemoryAllocator::release, &allocator, std::placeholders::_1));
}

void Geometry::copy_vertices(const Vertex* const vertices, size_t vertex_count, size_t start)
{
    std::lock_guard<std::recursive_mutex> guard(_mutex);

    // fill the normal/tangent line buffers (for debugging)
    float *nlb = _normal_line_buffer.get(), *tnlb = _tangent_line_buffer.get();
    for(size_t i=0; i<_vertex_count; ++i) {
        const Vertex& vertex(vertices[i]);

        const size_t idx = (start * 2 * 3) + (i * 2 * 3);
        const Position& p(vertex.position);
        const Vector3 &n(vertex.normal), &t(vertex.tangent);

        *(nlb + idx + 0) = p.x(); *(nlb + idx + 3) = p.x() + n.x();
        *(nlb + idx + 1) = p.y(); *(nlb + idx + 4) = p.y() + n.y();
        *(nlb + idx + 2) = p.z(); *(nlb + idx + 5) = p.z() + n.z();

        *(tnlb + idx + 0) = p.x(); *(tnlb + idx + 3) = p.x() + t.x();
        *(tnlb + idx + 1) = p.y(); *(tnlb + idx + 4) = p.y() + t.y();
        *(tnlb + idx + 2) = p.z(); *(tnlb + idx + 5) = p.z() + t.z();
    }

    // fill the vertex buffers
    float *va = _vertex_buffer.get(), *na = _normal_buffer.get(), *tna = _tangent_buffer.get(), *ta = _texture_buffer.get();
    for(size_t i=0; i<_vertex_count; ++i) {
        const Vertex& vertex(vertices[i]);

        const size_t vidx = (start * 3) + (i * 3);
        *(va + vidx + 0) = vertex.position.x();
        *(va + vidx + 1) = vertex.position.y();
        *(va + vidx + 2) = vertex.position.z();

        const Vector3& normal(vertex.normal);
        *(na + vidx + 0) = normal.x();
        *(na + vidx + 1) = normal.y();
        *(na + vidx + 2) = normal.z();

        const size_t tnidx = (start * 4) + (i * 4);

        // Mathematics for 3D Game Programming and Computer Graphics, section 7.8.3
        const Vector3& tangent(vertex.tangent);
        const Vector3& bitangent(normal ^ tangent);
        *(tna + tnidx + 0) = tangent.x();
        *(tna + tnidx + 1) = tangent.y();
        *(tna + tnidx + 2) = tangent.z();
        *(tna + tnidx + 3) = bitangent.opposite_direction(vertex.bitangent) ? -1.0f : 1.0f;

        const size_t tidx = (start * 2) + (i * 2);
        *(ta + tidx + 0) = vertex.texture_coords.x();
        *(ta + tidx + 1) = vertex.texture_coords.y();
    }
}

void Geometry::copy_triangles(const Triangle* const triangles, size_t triangle_count, const Vertex* const vertices, size_t vertex_count, size_t start)
{
    std::lock_guard<std::recursive_mutex> guard(_mutex);

    // fill the normal/tangent line buffers (for debugging)
    float *nlb = _normal_line_buffer.get(), *tnlb = _tangent_line_buffer.get();
    for(size_t i=0; i<vertex_count; ++i) {
        const Vertex& vertex(vertices[i]);

        const size_t idx = (start * 2 * 3) + i * 2 * 3;
        const Position& p(vertex.position);
        const Vector3 &n(vertex.normal), &t(vertex.tangent);

        *(nlb + idx + 0) = p.x(); *(nlb + idx + 3) = p.x() + n.x();
        *(nlb + idx + 1) = p.y(); *(nlb + idx + 4) = p.y() + n.y();
        *(nlb + idx + 2) = p.z(); *(nlb + idx + 5) = p.z() + n.z();

        *(tnlb + idx + 0) = p.x(); *(tnlb + idx + 3) = p.x() + t.x();
        *(tnlb + idx + 1) = p.y(); *(tnlb + idx + 4) = p.y() + t.y();
        *(tnlb + idx + 2) = p.z(); *(tnlb + idx + 5) = p.z() + t.z();
    }

    // fill the vertex buffers
    float *vb = _vertex_buffer.get(), *nb = _normal_buffer.get(), *tnb = _tangent_buffer.get(), *tb = _texture_buffer.get();
    for(size_t i=0; i<triangle_count; ++i) {
        const Triangle& triangle(triangles[i]);
        const Vertex &v1(vertices[triangle.v1]), &v2(vertices[triangle.v2]), &v3(vertices[triangle.v3]);

        const size_t idx = (start * 3) + (i * 3 * 3);

        *(vb + idx + 0) = v1.position.x();
        *(vb + idx + 1) = v1.position.y();
        *(vb + idx + 2) = v1.position.z();
        *(vb + idx + 3) = v2.position.x();
        *(vb + idx + 4) = v2.position.y();
        *(vb + idx + 5) = v2.position.z();
        *(vb + idx + 6) = v3.position.x();
        *(vb + idx + 7) = v3.position.y();
        *(vb + idx + 8) = v3.position.z();

        const Vector3 &n1(v1.normal), &n2(v2.normal), &n3(v3.normal);
        *(nb + idx + 0) = n1.x();
        *(nb + idx + 1) = n1.y();
        *(nb + idx + 2) = n1.z();
        *(nb + idx + 3) = n2.x();
        *(nb + idx + 4) = n2.y();
        *(nb + idx + 5) = n2.z();
        *(nb + idx + 6) = n3.x();
        *(nb + idx + 7) = n3.y();
        *(nb + idx + 8) = n3.z();

        const size_t tnidx = (start * 4) + (i * 3 * 4);

        // Mathematics for 3D Game Programming and Computer Graphics, section 7.8.3
        const Vector3 &t1(v1.tangent), &t2(v2.tangent), &t3(v3.tangent);
        const Vector3 b1(n1 ^ t1), b2(n2 ^ t2), b3(n3 ^ t3);
        *(tnb + tnidx + 0)  = t1.x();
        *(tnb + tnidx + 1)  = t1.y();
        *(tnb + tnidx + 2)  = t1.z();
        *(tnb + tnidx + 3)  = b1.opposite_direction(v1.bitangent) ? -1.0f : 1.0f;
        *(tnb + tnidx + 4)  = t2.x();
        *(tnb + tnidx + 5)  = t2.y();
        *(tnb + tnidx + 6)  = t2.z();
        *(tnb + tnidx + 7)  = b2.opposite_direction(v2.bitangent) ? -1.0f : 1.0f;
        *(tnb + tnidx + 8)  = t3.x();
        *(tnb + tnidx + 9)  = t3.y();
        *(tnb + tnidx + 10) = t3.z();
        *(tnb + tnidx + 11) = b3.opposite_direction(v3.bitangent) ? -1.0f : 1.0f;

        const size_t tidx = (start * 2) + (i * 3 * 2);

        *(tb + tidx + 0) = v1.texture_coords.x();
        *(tb + tidx + 1) = v1.texture_coords.y();
        *(tb + tidx + 2) = v2.texture_coords.x();
        *(tb + tidx + 3) = v2.texture_coords.y();
        *(tb + tidx + 4) = v3.texture_coords.x();
        *(tb + tidx + 5) = v3.texture_coords.y();
    }
}

std::string Geometry::str() const
{
    std::stringstream ss;
    ss << "Geometry vertex_count=" << _vertex_count << "\nVertices (" << _vertex_buffer_size << "):\n";
    for(size_t i=0; i<_vertex_buffer_size; i+=3) {
        ss << "(" << _vertex_buffer.get()[i+0]
            << ", " << _vertex_buffer.get()[i+1]
            << ", " << _vertex_buffer.get()[i+2] << "), ";
    }
    return ss.str();
}

}

#if defined WITH_UNIT_TESTS
#include "src/test/UnitTest.h"

class GeometryTest : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(GeometryTest);
        CPPUNIT_TEST(test_allocate_vertex);
        CPPUNIT_TEST(test_vertex_distance);
        CPPUNIT_TEST(test_allocate_triangle);
        CPPUNIT_TEST(test_allocate_weight);
        CPPUNIT_TEST(test_allocate_edge);
        CPPUNIT_TEST(test_compute_tangents);
        CPPUNIT_TEST(test_vertex_create);
        CPPUNIT_TEST(test_vertex_create_copy);
        CPPUNIT_TEST(test_triangle_create);
        CPPUNIT_TEST(test_triangle_create_copy);
    CPPUNIT_TEST_SUITE_END();

public:
    GeometryTest() : CppUnit::TestFixture() {}
    virtual ~GeometryTest() noexcept {}

public:
    void test_allocate_vertex()
    {
        std::shared_ptr<energonsoftware::MemoryAllocator> allocator(energonsoftware::MemoryAllocator::new_allocator(energonsoftware::MemoryAllocator::Type::System, 50 * 1024));
        std::shared_ptr<energonsoftware::Vertex> v1(new(16, *allocator) energonsoftware::Vertex(),
            energonsoftware::MemoryAllocator_delete_aligned<energonsoftware::Vertex, 16>(allocator.get()));
        check_vertex_defaults(*v1);

        v1.reset();

        static const size_t COUNT = 100;
        std::shared_ptr<energonsoftware::Vertex> vertices(energonsoftware::MemoryAllocator_new_aligned<energonsoftware::Vertex, 16>(COUNT, *allocator),
            energonsoftware::MemoryAllocator_delete_aligned<energonsoftware::Vertex[], 16>(COUNT, allocator.get()));
        CPPUNIT_ASSERT(vertices);
        for(size_t i=0; i<COUNT; ++i) {
            check_vertex_defaults(vertices.get()[i]);
        }
        vertices.reset();
    }

    void test_vertex_distance()
    {
    }

    void test_allocate_triangle()
    {
        std::shared_ptr<energonsoftware::MemoryAllocator> allocator(energonsoftware::MemoryAllocator::new_allocator(energonsoftware::MemoryAllocator::Type::System, 50 * 1024));
        std::shared_ptr<energonsoftware::Triangle> t1(new(16, *allocator) energonsoftware::Triangle(),
            energonsoftware::MemoryAllocator_delete_aligned<energonsoftware::Triangle, 16>(allocator.get()));
        check_triangle_defaults(*t1);

        t1.reset();

        static const size_t COUNT = 100;
        std::shared_ptr<energonsoftware::Triangle> triangles(energonsoftware::MemoryAllocator_new_aligned<energonsoftware::Triangle, 16>(COUNT, *allocator),
            energonsoftware::MemoryAllocator_delete_aligned<energonsoftware::Triangle[], 16>(COUNT, allocator.get()));
        CPPUNIT_ASSERT(triangles);
        for(size_t i=0; i<COUNT; ++i) {
            check_triangle_defaults(triangles.get()[i]);
        }
        triangles.reset();
    }

    void test_allocate_weight()
    {
        std::shared_ptr<energonsoftware::MemoryAllocator> allocator(energonsoftware::MemoryAllocator::new_allocator(energonsoftware::MemoryAllocator::Type::System, 50 * 1024));
        std::shared_ptr<energonsoftware::Weight> w1(new(16, *allocator) energonsoftware::Weight(),
            energonsoftware::MemoryAllocator_delete_aligned<energonsoftware::Weight, 16>(allocator.get()));
        check_weight_defaults(*w1);

        w1.reset();

        static const size_t COUNT = 100;
        std::shared_ptr<energonsoftware::Weight> weights(energonsoftware::MemoryAllocator_new_aligned<energonsoftware::Weight, 16>(COUNT, *allocator),
            energonsoftware::MemoryAllocator_delete_aligned<energonsoftware::Weight[], 16>(COUNT, allocator.get()));
        CPPUNIT_ASSERT(weights);
        for(size_t i=0; i<COUNT; ++i) {
            check_weight_defaults(weights.get()[i]);
        }
        weights.reset();
    }

    void test_allocate_edge()
    {
        const energonsoftware::Edge e;
        check_edge_defaults(e);
    }

    void test_compute_tangents()
    {
    }

    void test_vertex_create()
    {
        static const size_t COUNT = 100;
        std::shared_ptr<energonsoftware::MemoryAllocator> allocator(energonsoftware::MemoryAllocator::new_allocator(energonsoftware::MemoryAllocator::Type::System, 50 * 1024));
        std::shared_ptr<energonsoftware::Geometry> g(new(*allocator) energonsoftware::Geometry(COUNT, *allocator),
            energonsoftware::MemoryAllocator_delete<energonsoftware::Geometry>(allocator.get()));

        g.reset();
    }

    void test_vertex_create_copy()
    {
    }

    void test_triangle_create()
    {
        static const size_t COUNT = 100;
        std::shared_ptr<energonsoftware::MemoryAllocator> allocator(energonsoftware::MemoryAllocator::new_allocator(energonsoftware::MemoryAllocator::Type::System, 50 * 1024));
        // vertex_count here is actually ignored
        std::shared_ptr<energonsoftware::Geometry> g(new(*allocator) energonsoftware::Geometry(COUNT, 0, *allocator),
            energonsoftware::MemoryAllocator_delete<energonsoftware::Geometry>(allocator.get()));

        g.reset();
    }

    void test_triangle_create_copy()
    {
    }

private:
    void check_vertex_defaults(const energonsoftware::Vertex& v)
    {
        CPPUNIT_ASSERT(v.index < 0);
        CPPUNIT_ASSERT(v.position.is_zero());
        CPPUNIT_ASSERT(v.normal.is_zero());
        CPPUNIT_ASSERT(v.tangent.is_zero());
        CPPUNIT_ASSERT(v.bitangent.is_zero());
        CPPUNIT_ASSERT(v.texture_coords.is_zero());
        CPPUNIT_ASSERT_EQUAL(0, v.weight_start);
        CPPUNIT_ASSERT_EQUAL(0, v.weight_count);
    }

    void check_triangle_defaults(const energonsoftware::Triangle& t)
    {
        CPPUNIT_ASSERT(t.index < 0);
        CPPUNIT_ASSERT(t.v1 < 0);
        CPPUNIT_ASSERT(t.v2 < 0);
        CPPUNIT_ASSERT(t.v3 < 0);
        CPPUNIT_ASSERT(t.normal.is_zero());
    }

    void check_weight_defaults(const energonsoftware::Weight& w)
    {
        CPPUNIT_ASSERT(w.index < 0);
        CPPUNIT_ASSERT(w.joint < 0);
        CPPUNIT_ASSERT_EQUAL(w.weight, 0.0f);
        CPPUNIT_ASSERT(w.position.is_zero());
        CPPUNIT_ASSERT(w.normal.is_zero());
        CPPUNIT_ASSERT(w.tangent.is_zero());
        CPPUNIT_ASSERT(w.bitangent.is_zero());
    }

    void check_edge_defaults(const energonsoftware::Edge& e)
    {
        CPPUNIT_ASSERT(e.v1 < 0);
        CPPUNIT_ASSERT(e.v2 < 0);
        CPPUNIT_ASSERT(e.t1 < 0);
        CPPUNIT_ASSERT(e.t2 < 0);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(GeometryTest);

#endif
