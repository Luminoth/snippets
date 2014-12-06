#if !defined __GEOMETRY_H__
#define __GEOMETRY_H__

#include "src/core/math/Vector.h"

namespace energonsoftware {

struct Vertex
{
    static void destroy(Vertex* const vertex, MemoryAllocator* const allocator);
    static Vertex* create_array(size_t count, MemoryAllocator& allocator);
    static void destroy_array(Vertex* const vertices, size_t count, MemoryAllocator* const allocator);

    int index;
    Position position;
    Vector3 normal, tangent, bitangent;
    Vector2 texture_coords;
    int weight_start, weight_count;

    Vertex();
    virtual ~Vertex() noexcept {}

    std::string str() const;

    float distance_squared(const Vertex& other) const { return position.distance_squared(other.position); }
};

struct Triangle
{
    static void destroy(Triangle* const triangle, MemoryAllocator* const allocator);
    static Triangle* create_array(size_t count, MemoryAllocator& allocator);
    static void destroy_array(Triangle* const triangles, size_t count, MemoryAllocator* const allocator);

    int index;
    int v1, v2, v3;
    Vector3 normal;

    Triangle();
    virtual ~Triangle() noexcept {}
};

struct Weight
{
    static void destroy(Weight* const weight, MemoryAllocator* const allocator);
    static Weight* create_array(size_t count, MemoryAllocator& allocator);
    static void destroy_array(Weight* const weight, size_t count, MemoryAllocator* const allocator);

    int index;
    int joint;
    float weight;
    Position position;
    Vector3 normal, tangent, bitangent;

    Weight();
    virtual ~Weight() noexcept {}
};

struct Edge
{
    /*static void destroy(Edge* const edge, MemoryAllocator* const allocator);
    static Edge* create_array(size_t count, MemoryAllocator& allocator);
    static void destroy_array(Edge* const edge, size_t count, MemoryAllocator* const allocator);*/

    int v1, v2;
    int t1, t2;

    Edge();
    virtual ~Edge() noexcept {}

    std::string str() const;
};

void compute_tangents(Triangle* const triangles, size_t triange_count, Vertex* const vertices, size_t vertex_count, MemoryAllocator& allocator, bool smooth=false);

class Geometry
{
public:
    static void destroy(Geometry* const weight, MemoryAllocator* const allocator);

public:
    explicit Geometry(size_t vertex_count, MemoryAllocator& allocator);
    Geometry(const Vertex* const vertices, size_t vertex_count, MemoryAllocator& allocator);

    Geometry(size_t triangle_count, size_t vertex_count, MemoryAllocator& allocator);
    Geometry(const Triangle* const triangles, size_t triangle_count, const Vertex* const vertices, size_t vertex_count, MemoryAllocator& allocator);

    virtual ~Geometry() noexcept;

public:
    boost::recursive_mutex& mutex() { return _mutex; }

    size_t vertex_count() const { return _vertex_count; }

    // NOTE: must lock before using these!
    size_t vertex_buffer_size() const { return _vertex_buffer_size; }
    const float* const vertex_buffer() const { return _vertex_buffer.get(); }

    size_t normal_buffer_size() const { return _normal_buffer_size; }
    const float* const normal_buffer() const { return _normal_buffer.get(); }

    size_t tangent_buffer_size() const { return _tangent_buffer_size; }
    const float* const tangent_buffer() const { return _tangent_buffer.get(); }

    size_t texture_buffer_size() const { return _texture_buffer_size; }
    const float* const texture_buffer() const { return _texture_buffer.get(); }

    // for debugging
    const float* const normal_line_buffer() const { return _normal_line_buffer.get(); }
    const float* const tangent_line_buffer() const { return _tangent_line_buffer.get(); }

    void copy_vertices(const Vertex* const vertices, size_t vertex_count, size_t start=0);
    void copy_triangles(const Triangle* const triangles, size_t triangle_count, const Vertex* const vertices, size_t vertex_count, size_t start=0);

    std::string str() const;

private:
    void allocate_buffers(size_t vertex_count, MemoryAllocator& allocator);
    void allocate_buffers(size_t triangle_count, size_t vertex_count, MemoryAllocator& allocator);

private:
    boost::recursive_mutex _mutex;

    size_t _vertex_count;

    size_t _vertex_buffer_size;
    boost::shared_array<float> _vertex_buffer;

    size_t _normal_buffer_size;
    boost::shared_array<float> _normal_buffer;

    size_t _tangent_buffer_size;
    boost::shared_array<float> _tangent_buffer;

    size_t _texture_buffer_size;
    boost::shared_array<float> _texture_buffer;

    // debugging stuffs
    boost::shared_array<float> _normal_line_buffer;
    boost::shared_array<float> _tangent_line_buffer;

private:
    Geometry();
    DISALLOW_COPY_AND_ASSIGN(Geometry);
};

}

#endif
