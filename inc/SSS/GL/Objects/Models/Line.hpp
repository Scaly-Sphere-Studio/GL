#ifndef SSS_GL_LINE_HPP
#define SSS_GL_LINE_HPP

#include <SSS/Math.hpp>
#include "SSS/GL/Objects/Basic.hpp"
#include <unordered_map>

SSS_GL_BEGIN;

// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

class SSS_GL_API Polyline {
    friend class LineRenderer;

public:
    enum struct JointType {
        /** Create a fan from the first border to the second*/
        ROUND,
        /** Angle are cut off at the edge of the corner*/
        BEVEL,
        /** Joint is the intersection of the extended lines*/
        MITER,
    };

    enum struct TermType {
        /** Terminaisons are rounded and extended by half the line thickness*/
        ROUND,
        /** Line cut at the anchor points*/
        BUTT,
        /** Expend the terminaison by half the line thickness*/
        SQUARE,
        /** Connect the first and last point according to joint style*/
        CONNECT
    };

    struct Vertex {
        Vertex(glm::vec3 position = glm::vec3(0.f),
            glm::vec4 color = glm::vec4(1.f, 0.f, 0.f, 1.f))
            : v_pos(position), v_color(color) {};

        glm::vec3 v_pos;
        glm::vec4 v_color;

        using Vec = std::vector<Vertex>;

        friend std::ostream& operator<<(std::ostream& os, const Vertex& v)
        {
            os << "[" << v.v_pos.x << "," << v.v_pos.y << "," << v.v_pos.z << "]" << std::endl;
            return os;
        }
    };

private:
    Polyline(Vertex::Vec path,
        Math::Gradient<float> thickness, Math::Gradient<glm::vec4> color, 
        JointType jopt, TermType topt);
public:
    ~Polyline();

    using Shared = std::shared_ptr<Polyline>;

    static Shared Line(Vertex::Vec path,
        Math::Gradient<float> thickness, Math::Gradient<glm::vec4> color,
        JointType jopt = JointType::BEVEL, TermType topt = TermType::BUTT);

    static Shared Line(Vertex::Vec path,
        float thickness = 10.0f, glm::vec4 color = glm::vec4(0,0,0,1),
        JointType jopt = JointType::BEVEL, TermType topt = TermType::BUTT);

    static Shared Segment(glm::vec3 a, glm::vec3 b,
        Math::Gradient<float> thickness, Math::Gradient<glm::vec4> color,
        JointType jopt = JointType::BEVEL, TermType topt = TermType::BUTT);

    static Shared Segment(glm::vec3 a, glm::vec3 b,
        float thickness = 10.0f, glm::vec4 color = glm::vec4(0,0,0,1),
        JointType jopt = JointType::BEVEL, TermType topt = TermType::BUTT);

    static Shared Bezier(
        glm::vec3 a, glm::vec3 b,
        glm::vec3 c, glm::vec3 d,
        Math::Gradient<float> thickness, Math::Gradient<glm::vec4> color,
        JointType jopt = JointType::BEVEL, TermType topt = TermType::BUTT);

    static Shared Bezier(
        glm::vec3 a, glm::vec3 b,
        glm::vec3 c, glm::vec3 d,
        float thickness = 10.0f, glm::vec4 color = glm::vec4(0,0,0,1),
        JointType jopt = JointType::BEVEL, TermType topt = TermType::BUTT);


    uint32_t update(Math::Gradient<float> gradient_thickness, Math::Gradient<glm::vec4> gradient_color);

    static bool sort(std::weak_ptr<Polyline>& f, std::weak_ptr<Polyline>& s)
    {
        if (f.lock() && s.lock()) {
            return f.lock()->mesh[0].v_pos.z > s.lock()->mesh[0].v_pos.z;
        }
        return false;
    };

private:

    static std::vector<std::weak_ptr<Polyline>> _batch;
    // State of the batch, information about when the buffer has been
    // modified and should be regenerated
    static bool modified;

    struct Mesh_info {
        Mesh_info();
        Mesh_info(uint16_t t, uint16_t b, uint16_t aat, uint16_t aab);

        void update(uint16_t t, uint16_t b, uint16_t aat, uint16_t aab);
        uint16_t top, btm, aa_top, aa_btm;
    };

    struct Indices {
        Indices(uint16_t uc = 0, uint16_t bc = 0, uint16_t sc = 0)
            : _uc(uc), _bc(bc), _sc(sc) {};

        uint32_t _uc, _bc, _sc;

        using Vec = std::vector<Indices>;

        friend std::ostream& operator<<(std::ostream& os, const Indices& t)
        {
            os << "[" << t._uc << "," << t._bc << "," << t._sc << "]" << std::endl;
            return os;
        }
    };

    TermType _topt;
    JointType _jopt;
    float _aa_thickness;
    Math::Gradient<float> g_thick;
    Math::Gradient<glm::vec4> g_col;

    Vertex::Vec path;
    Vertex::Vec mesh;
    Indices::Vec indices;

    float define_line_thickness(float thickness);
    float define_aa_thickness(float thickness);
    uint8_t quad_index(uint32_t lft_top, uint32_t lft_btm, uint32_t rgt_btm, uint32_t rgt_top);

    //Create the mesh of the line using the path data
    uint8_t path_meshing(Math::Gradient<float> gradient_thickness, Math::Gradient<glm::vec4> gradient_color, JointType jopt, TermType topt);


    //terminaisons functions
    uint8_t butt_ending(uint32_t index, Mesh_info& last, glm::vec3& ortho, float thickness, glm::vec4 color);
    uint8_t square_ending(uint32_t index, Mesh_info& last, glm::vec3& ortho, float thickness, glm::vec4 color);
    uint8_t round_ending(uint32_t index, Mesh_info& last, glm::vec3& ortho, float thickness, glm::vec4 color);
    uint8_t connect_ending(uint32_t index, Mesh_info& last, glm::vec3& ortho, float thickness, glm::vec4 color);

    //joint functions
    uint8_t miter_joint(uint32_t index, Mesh_info& last, glm::vec3& ortho, float thickness, glm::vec4 color);
    uint8_t bevel_joint(uint32_t index, Mesh_info& last, glm::vec3& ortho, float thickness, glm::vec4 color);
    uint8_t fan_joint(uint32_t index, Mesh_info& last, glm::vec3& ortho, float thickness, glm::vec4 color);
    
    //termination / joint function ptr
    using FuncPtr = uint8_t(Polyline::*)(uint32_t index, Mesh_info&, glm::vec3& pos, float thickness, glm::vec4 color);
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_LINE_HPP