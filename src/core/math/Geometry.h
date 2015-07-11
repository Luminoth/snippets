#if !defined __GEOMETRY_H__
#define __GEOMETRY_H__

#include "src/core/math/Vector.h"

namespace energonsoftware {

struct Vertex
{
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
    int index;
    int v1, v2, v3;
    Vector3 normal;

    Triangle();
    virtual ~Triangle() noexcept {}
};

struct Weight
{
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
    explicit Geometry(size_t vertex_count, MemoryAllocator& allocator);
    Geometry(const Vertex* const vertices, size_t vertex_count, MemoryAllocator& allocator);

    Geometry(size_t triangle_count, size_t vertex_count, MemoryAllocator& allocator);
    Geometry(const Triangle* const triangles, size_t triangle_count, const Vertex* const vertices, size_t vertex_count, MemoryAllocator& allocator);

    virtual ~Geometry() noexcept;

public:
    std::recursive_mutex& mutex() { return _mutex; }

    size_t vertex_count() const { return _vertex_count; }

    // NOTE: must lock before using these!
    size_t vertex_buffer_size() const { return _vertex_buffer_size; }
    const float* vertex_buffer() const { return _vertex_buffer.get(); }

    size_t normal_buffer_size() const { return _normal_buffer_size; }
    const float* normal_buffer() const { return _normal_buffer.get(); }

    size_t tangent_buffer_size() const { return _tangent_buffer_size; }
    const float* tangent_buffer() const { return _tangent_buffer.get(); }

    size_t texture_buffer_size() const { return _texture_buffer_size; }
    const float* texture_buffer() const { return _texture_buffer.get(); }

    // for debugging
    const float* normal_line_buffer() const { return _normal_line_buffer.get(); }
    const float* tangent_line_buffer() const { return _tangent_line_buffer.get(); }

    void copy_vertices(const Vertex* const vertices, size_t vertex_count, size_t start=0);
    void copy_triangles(const Triangle* const triangles, size_t triangle_count, const Vertex* const vertices, size_t vertex_count, size_t start=0);

    std::string str() const;

private:
    void allocate_buffers(size_t vertex_count, MemoryAllocator& allocator);
    void allocate_buffers(size_t triangle_count, size_t vertex_count, MemoryAllocator& allocator);

private:
    std::recursive_mutex _mutex;

    size_t _vertex_count;

    size_t _vertex_buffer_size;
    std::shared_ptr<float> _vertex_buffer;

    size_t _normal_buffer_size;
    std::shared_ptr<float> _normal_buffer;

    size_t _tangent_buffer_size;
    std::shared_ptr<float> _tangent_buffer;

    size_t _texture_buffer_size;
    std::shared_ptr<float> _texture_buffer;

    // debugging stuffs
    std::shared_ptr<float> _normal_line_buffer;
    std::shared_ptr<float> _tangent_line_buffer;

private:
    Geometry() = delete;
    DISALLOW_COPY_AND_ASSIGN(Geometry);
};

}

#endif
