#include "GL/Objects/Models/Line.hpp"

SSS_GL_BEGIN;

static constexpr glm::vec4 fade = glm::vec4(1, 1, 1, 0);
static constexpr double MINIMUM_FAN_ANGLE = M_PI / 18;
static constexpr double MAX_MITER_ANGLE = M_PI * 0.75;
static constexpr double MINIMUM_BEVEL_ANGLE = M_PI / 18;

std::vector<std::weak_ptr<Polyline>> Polyline::_batch{};
bool Polyline::modified = true;


Polyline::Polyline(Vertex::Vec _path,
    Math::Gradient<float> gradient_thickness,
    Math::Gradient<glm::vec4> gradient_color,
    JointType jopt, TermType topt)
    :   path(_path),
        g_thick(gradient_thickness),
        g_col(gradient_color),
        _jopt(jopt),
        _topt(topt)
{
    //Select the largest element in the gradient to define the antialliasing/feathering width
    _aa_thickness = define_aa_thickness(gradient_thickness.max());
    path_meshing(gradient_thickness, gradient_color, jopt, topt);

    modified = true;
}


Polyline::~Polyline()
{
    modified = true;

    mesh.clear();
    path.clear();
    indices.clear();
}


uint32_t Polyline::update(Math::Gradient<float> gradient_thickness, Math::Gradient<glm::vec4> gradient_color)
{
    path_meshing(gradient_thickness, gradient_color, _jopt, _topt);

    return 0;
}



Polyline::Shared Polyline::Line(Vertex::Vec path,
    Math::Gradient<float> g_thickness, Math::Gradient<glm::vec4> g_color,
    JointType jopt, TermType topt)
{
    Shared line(new Polyline(path, g_thickness, g_color, jopt, topt));
    _batch.emplace_back(line);


    return line;
}

Polyline::Shared Polyline::Line(Vertex::Vec path,
    float thickness, glm::vec4 color,
    JointType jopt, TermType topt)
{
    Math::Gradient<glm::vec4> g_color;
    g_color.push(std::make_pair(0.0f, color));

    Math::Gradient<float> g_thickness;
    g_thickness.push(std::make_pair(0.0f, thickness));

    Shared line(new Polyline(path, g_thickness, g_color, jopt, topt));
    _batch.emplace_back(line);

    return line;
}

Polyline::Shared Polyline::Segment(glm::vec3 a, glm::vec3 b,
    Math::Gradient<float> thickness, Math::Gradient<glm::vec4> color,
    JointType jopt, TermType topt)
{
    Vertex::Vec path;
    path.emplace_back(a);
    path.emplace_back(b);
    

    Shared line(new Polyline(path, thickness, color, jopt, topt));
    _batch.emplace_back(line);
    path.clear();

    return line;
}

Polyline::Shared Polyline::Segment(glm::vec3 a, glm::vec3 b,
    float thickness, glm::vec4 color,
    JointType jopt, TermType topt)
{

    Math::Gradient<float> g_thickness;
    g_thickness.push(std::make_pair(0.0f, thickness));

    Math::Gradient<glm::vec4> g_color;
    g_color.push(std::make_pair(0.0f, color));

    Vertex::Vec path;
    path.emplace_back(a);
    path.emplace_back(b);


    Shared line(new Polyline(path, g_thickness, g_color, jopt, topt));
    _batch.emplace_back(line);
    path.clear();

    return line;
}

Polyline::Shared Polyline::Bezier(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
    Math::Gradient<float> thickness, Math::Gradient<glm::vec4> color,
    JointType jopt, TermType topt)
{
    Vertex::Vec path;
    std::vector<std::pair<float, glm::vec3>> v;
    v.emplace_back(std::make_pair(0.f, a));
    v.emplace_back(std::make_pair(0.5f, Math::bezier_func(0.5f, a, b, c, d)));
    v.emplace_back(std::make_pair(1.f, d));

    Math::bezier_recurs(v, v[0], v[1], a, b, c, d);
    Math::bezier_recurs(v, v[1], v[2], a, b, c, d);

    std::sort(v.begin(), v.end(), Math::sort_pair_vec);
    path.reserve(v.size());

    for (auto p : v) {
        path.emplace_back(p.second);
    }

    Shared line(new Polyline(path, thickness, color, jopt, topt));
    _batch.emplace_back(line);
    path.clear();

    return line;
}

Polyline::Shared Polyline::Bezier(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
    float thickness, glm::vec4 color,
    JointType jopt, TermType topt)
{
    Math::Gradient<float> g_thickness;
    g_thickness.push(std::make_pair(0.0f, thickness));

    Math::Gradient<glm::vec4> g_color;
    g_color.push(std::make_pair(0.0f, color));

    Vertex::Vec path;
    std::vector<std::pair<float, glm::vec3>> v;
    v.emplace_back(std::make_pair(0.f, a));
    v.emplace_back(std::make_pair(0.5f, Math::bezier_func(0.5f, a, b, c, d)));
    v.emplace_back(std::make_pair(1.f, d));

    Math::bezier_recurs(v, v[0], v[1], a, b, c, d);
    Math::bezier_recurs(v, v[1], v[2], a, b, c, d);

    std::sort(v.begin(), v.end(), Math::sort_pair_vec);
    path.reserve(v.size());

    for (auto p : v) {
        path.emplace_back(p.second);
    }

    Shared line(new Polyline(path, g_thickness, g_color, jopt, topt));
    _batch.emplace_back(line);
    path.clear();

    return line;
}

Polyline::Mesh_info::Mesh_info() :
    top(0), btm(1), aa_top(2), aa_btm(3)
{
}

Polyline::Mesh_info::Mesh_info(uint16_t t, uint16_t b, uint16_t aat, uint16_t aab) :
    top(t), btm(b), aa_top(aat), aa_btm(aab)
{
}

void Polyline::Mesh_info::update(uint16_t t, uint16_t b, uint16_t aat, uint16_t aab)
{
    top = t;
    btm = b;
    aa_top = aat;
    aa_btm = aab;
}


float Polyline::define_line_thickness(float thickness)
{
    //Define the thickness used for the line meshing
    if (thickness < 1.f) {
        return 0.f;
    }

    //Add a correction to incorporate the anti-alliasing thickness, usefull for thin lines
    return (thickness - 1.f) / 2.f;
}

float Polyline::define_aa_thickness(float thickness)
{
    //Clamp the anti-alliasing thickness, with a slight correction to look better for thin line
    //Went with a little overboard for larger line, because it looks better
    return std::clamp(thickness / 8.0f, 0.8f, 1.5f);
}

uint8_t Polyline::quad_index(uint32_t lft_top, uint32_t lft_btm, uint32_t rgt_btm, uint32_t rgt_top)
{
    //Use the counter clockwise index convention
    indices.emplace_back(lft_top, lft_btm, rgt_btm);
    indices.emplace_back(rgt_btm, rgt_top, lft_top);

    return 0;
}

uint8_t Polyline::path_meshing(Math::Gradient<float> gradient_thickness, Math::Gradient<glm::vec4> gradient_color, JointType jopt, TermType topt)
{
    if (path.size() < 2) {
        return 1;
    }

    Mesh_info mesh_last;

    glm::vec3 ortho = glm::vec3(0);

    FuncPtr joint_type_func;
    switch (jopt) {
    case JointType::MITER:
        joint_type_func = &Polyline::miter_joint;
        break;
    case JointType::BEVEL:
        joint_type_func = &Polyline::bevel_joint;
        break;
    case JointType::ROUND:
        joint_type_func = &Polyline::fan_joint;
        break;
    default:
        throw std::exception("Unhandled JointType");
    }

    FuncPtr term_type_func;
    switch (topt) {
    case TermType::BUTT:
        term_type_func = &Polyline::butt_ending;
        break; 
    case TermType::SQUARE:
        term_type_func = &Polyline::square_ending;
        break;
    case TermType::ROUND:
        term_type_func = &Polyline::round_ending;
        break;
    case TermType::CONNECT:
        term_type_func = &Polyline::connect_ending;
        break;
    default:
        throw std::exception("Unhandled TermType");
    }

    float m_thickness = 0;
    for (size_t i = 0; i < path.size(); i++) {
        glm::vec4 color = glm::vec4(0, 0, 0, 1);
        //Parameter to define the emplacement in the gradient
        float t = static_cast<float>(i) / static_cast<float>(path.size() - 1);

        if (i == 0 || i + 1 == path.size()) {
            //Line terminaison for first and last point
            (this->*term_type_func)(static_cast<uint32_t>(i), mesh_last, ortho, define_line_thickness(g_thick.evaluate(t)), g_col.evaluate(t));
        }
        else {
            //Meshing along the line
            (this->*joint_type_func)(static_cast<uint32_t>(i), mesh_last, ortho, define_line_thickness(g_thick.evaluate(t)), g_col.evaluate(t));
        }
    }
    return 0;
}

uint8_t Polyline::butt_ending(uint32_t index, Mesh_info& last, 
    glm::vec3& ortho, float thickness, glm::vec4 color)
{
    //Use the present and next point to calculate the direction of the vector
    //Ortho gives the vector that is perpendicular to the direction of the segment
    glm::vec3 direction;

    //Use these vectors to calculate the coordinates of the points with the width and the antialliasing width 

    if (index == 0) {
        direction = Math::direction_vector2D(path[static_cast<size_t>(index) + 1].v_pos, path[index].v_pos);
        ortho = Math::ortho_vector(path[index].v_pos, path[static_cast<size_t>(index) + 1].v_pos);

        //Indexing the first anti_alliasing
        indices.reserve(indices.size() + 6);
        quad_index(last.top, last.aa_top, last.aa_btm, last.btm);
    }
    else {
        //Last point
        direction = Math::direction_vector2D(path[index - 1].v_pos, path[index].v_pos);
        uint32_t et = static_cast<uint32_t>(mesh.size());
        Mesh_info cur(et, et + 1, et + 2, et + 3);

        indices.reserve(indices.size() + 4 * 6);

        //Use indexed renderer method
        quad_index(cur.top, cur.btm, cur.aa_btm, cur.aa_top);
        quad_index(last.top, last.btm, cur.btm, cur.top);
        quad_index(last.aa_top, last.top, cur.top, cur.aa_top);
        quad_index(last.btm, last.aa_btm, cur.aa_btm, cur.btm);
    }

    glm::vec3 p1 = ortho * thickness;
    glm::vec3 p2 = ortho * (thickness + _aa_thickness);
    glm::vec3 p3 = direction * _aa_thickness;

    glm::vec4 aa_color = color * fade;

    mesh.reserve(mesh.size() + 4);
    mesh.emplace_back(path[index].v_pos + p1, color);
    mesh.emplace_back(path[index].v_pos - p1, color);
    mesh.emplace_back(path[index].v_pos + p2 + p3, aa_color);
    mesh.emplace_back(path[index].v_pos - p2 + p3, aa_color);

    return 0;
}

uint8_t Polyline::square_ending(uint32_t index, Mesh_info& last, 
    glm::vec3& ortho, float thickness, glm::vec4 color)
{
    glm::vec3 direction;

    if (index == 0) {
        indices.reserve(indices.size() + 2);
        quad_index(last.top, last.aa_top, last.aa_btm, last.btm);

        direction = Math::direction_vector2D(path[static_cast<size_t>(index) + 1].v_pos, path[index].v_pos);
        ortho = Math::ortho_vector(path[index].v_pos, path[static_cast<size_t>(index) + 1].v_pos);
    }
    else {
        //Last point
        direction = Math::direction_vector2D(path[index - 1].v_pos, path[index].v_pos);
        uint32_t et = static_cast<uint32_t>(mesh.size());
        Mesh_info cur(et, et + 1, et + 2, et + 3);

        //last ibo
        //Anti-alliasing
        indices.reserve(indices.size() + 4 * 2);
        quad_index(cur.top, cur.btm, cur.aa_btm, cur.aa_top);
        quad_index(last.top, last.btm, cur.btm, cur.top);
        quad_index(last.aa_top, last.top, cur.top, cur.aa_top);
        quad_index(last.btm, last.aa_btm, cur.aa_btm, cur.btm);
    }


    mesh.reserve(mesh.size() + 4);

    glm::vec3 p1 = ortho * thickness;
    glm::vec3 p2 = ortho * (thickness + _aa_thickness);
    glm::vec3 p3 = direction * (thickness + _aa_thickness);
    glm::vec3 p4 = direction * thickness;

    glm::vec4 aa_color = color * fade;

    mesh.emplace_back(path[index].v_pos + p1 + p4, color);
    mesh.emplace_back(path[index].v_pos - p1 + p4, color);

    //AA
    mesh.emplace_back(path[index].v_pos + p2 + p3, aa_color);
    mesh.emplace_back(path[index].v_pos - p2 + p3, aa_color);

    return 0;
}

uint8_t Polyline::round_ending(uint32_t index, Mesh_info& last, 
    glm::vec3& ortho, float thickness, glm::vec4 color)
{
    double const count_d = M_PI / MINIMUM_FAN_ANGLE;
    uint32_t const count = static_cast<uint32_t>(count_d);
    glm::vec3 p1, p2, line, direction;
    glm::mat3 rotation;

    mesh.reserve(mesh.size() + 2 * count + 3);
    
    uint32_t pivot = static_cast<uint32_t>(mesh.size());
    mesh.emplace_back(path[index].v_pos, color);
    Mesh_info cur(pivot + 1, pivot + 2 * count + 1, pivot + 2, pivot + 2 * count + 2);


    if (index == 0) {
        indices.reserve(indices.size() + 3 * count);
        ortho = Math::ortho_vector(path[index].v_pos, path[static_cast<size_t>(index) + 1].v_pos);

        p1 = path[index].v_pos + ortho * thickness;
        p2 = path[index].v_pos + ortho * (thickness + _aa_thickness);

        mesh.emplace_back(p1, color);
        mesh.emplace_back(p2, color * fade);

        direction = Math::direction_vector2D(path[index].v_pos, p1);

        for (uint32_t j = 0; j < count; j++) {
            rotation = Math::rotation_matrix2D_ccw(M_PI / count_d * (static_cast<double>(j) + 1.0));

            line = thickness * direction * rotation;
            mesh.emplace_back(path[index].v_pos + line, color);

            line = (thickness + _aa_thickness ) * Math::direction_vector2D(path[index].v_pos, p1) * rotation;
            mesh.emplace_back(path[index].v_pos + line, color * fade);


            indices.emplace_back(pivot, cur.top + 2 * j, cur.top + 2 * j + 2);
            quad_index(cur.top + 2 * j, cur.aa_top + 2 * j, cur.aa_top + 2 * j + 2, cur.top + 2 * j + 2);
        }
        last = cur;
        return 0;

    }
    else {
        indices.reserve(indices.size() + 3 * count + 6);
        ortho = Math::ortho_vector(path[index - 1].v_pos, path[index].v_pos);

        p1 = path[index].v_pos + ortho * thickness;
        p2 = path[index].v_pos + ortho * (thickness + _aa_thickness);

        mesh.emplace_back(p1, color);
        mesh.emplace_back(p2, color * fade);

        direction = Math::direction_vector2D(path[index].v_pos, p1);

        for (uint32_t j = 0; j < count; j++) {
            rotation = Math::rotation_matrix2D_cw(M_PI / count_d * (static_cast<double>(j) + 1.0));

            line = thickness * direction * rotation;
            mesh.emplace_back(path[index].v_pos + line, color);
            line = (thickness + _aa_thickness) * Math::direction_vector2D(path[index].v_pos, p1) * rotation;
            mesh.emplace_back(path[index].v_pos + line, color * fade);

            indices.emplace_back(pivot, cur.top + 2 * j, cur.top + 2 * j + 2);
            quad_index(cur.top + 2 * j, cur.aa_top + 2 * j, cur.aa_top + 2 * j + 2, cur.top + 2 * j + 2);
        }

        quad_index(last.top, last.btm, cur.btm, cur.top);
        quad_index(last.aa_top, last.top, cur.top, cur.aa_top);
        quad_index(last.btm, last.aa_btm, cur.aa_btm, cur.btm);

        return 0;
    }

    return 1;
}

uint8_t Polyline::connect_ending(uint32_t index, Mesh_info& last, 
    glm::vec3& ortho, float thickness, glm::vec4 color)
{
    if (index == 0) {
        ortho = Math::ortho_vector(path[index].v_pos, path[static_cast<size_t>(index) + 1].v_pos);
        path.reserve(path.size() + 2);
        path.emplace_back(path[index].v_pos, color);
        path.emplace_back(path[static_cast<size_t>(index) + 1].v_pos, color);

        mesh.reserve(mesh.size() + 4);
        mesh.emplace_back(path[index].v_pos, color);
        mesh.emplace_back(path[index].v_pos, color);
        mesh.emplace_back(path[index].v_pos, color * fade);
        mesh.emplace_back(path[index].v_pos, color * fade);
    }
    else {
        //update the first 4 mesh points to connect with the 4 last calculated point
        Mesh_info cur;
        mesh[cur.top].v_pos     = mesh[last.top].v_pos;
        mesh[cur.btm].v_pos     = mesh[last.btm].v_pos;
        mesh[cur.aa_top].v_pos  = mesh[last.aa_top].v_pos;
        mesh[cur.aa_btm].v_pos  = mesh[last.aa_btm].v_pos;
    }

    return 0;
}

uint8_t Polyline::miter_joint(uint32_t index, Mesh_info& last, 
    glm::vec3& ortho, float thickness, glm::vec4 color)
{

    indices.reserve(indices.size() + 2 * 3);
    mesh.reserve(mesh.size() + 2);

    //path meshing + aa
    glm::vec3 p1, p2, p3, p4, ortho2, croisement, point;
    uint32_t fp = static_cast<uint32_t>(mesh.size());
    Mesh_info cur(fp, fp + 1, fp + 2, fp + 3);

    //Ortho2 is used to compare the perpendicular vector (ortho) of the first segment with the second segment 
    ortho2 = Math::ortho_vector(path[index].v_pos, path[static_cast<size_t>(index) + 1].v_pos);


    //When the angle between the two segment is too steep use the bevel method as it can cause the point to reach infinity
    if (Math::incidence_angle(ortho, ortho2) > MAX_MITER_ANGLE) {
        bevel_joint(index, last, ortho, thickness, color);
        return 0;
    }


    //Then loop for the first points coordinates and the feathering points (anti alliasing)
    for (uint32_t i = 0; i < 2; i++) {
        //First loop for the filled core, second for AA
        p1 = path[index - 1].v_pos  + ortho * (thickness + i * _aa_thickness);
        p2 = path[index].v_pos      + ortho * (thickness + i * _aa_thickness);
        p3 = path[index].v_pos      + ortho2 * (thickness + i * _aa_thickness);
        p4 = path[index + static_cast<size_t>(1)].v_pos  + ortho2 * (thickness + i * _aa_thickness);

        croisement = Math::intersection_point(p1, p2, p3, p4);
        point = croisement - path[index].v_pos;

        mesh.emplace_back(croisement, color * glm::vec4(1, 1, 1, 1 - i));
        mesh.emplace_back(path[index].v_pos - point, color * glm::vec4(1, 1, 1, 1 - i));
    }

    //IBO
    //Connect the 4 previous defined points to the current 4 new points
    
    quad_index(last.aa_top, last.top, cur.top, cur.aa_top);
    quad_index(last.top, last.btm, cur.btm, cur.top);
    quad_index(last.btm, last.aa_btm, cur.aa_btm, cur.btm);

    //switch ortho with ortho2 to keep track of the current segment
    last = cur;
    ortho = ortho2;

    return 0;
}

uint8_t Polyline::bevel_joint(uint32_t index, Mesh_info& last, 
    glm::vec3& ortho, float thickness, glm::vec4 color)
{
    indices.reserve(indices.size() + 2 * 4 + 1);
    mesh.reserve(mesh.size() + 6);

    Mesh_info cur;
    glm::vec3 p1, p2, p3, p4, bevel_correction;
    float angle = Math::incidence_angle(path[index].v_pos - path[index - 1].v_pos, path[static_cast<size_t>(index) + 1].v_pos - path[index].v_pos);

    glm::vec3 ortho2 = Math::ortho_vector(path[index].v_pos, path[static_cast<size_t>(index) + 1].v_pos);
    
    //Give the relative orientation between the two vectors
    //Use the determinant sign to determine if the next segment is above or below the current segment
    //If determinant is 0, the two segments are colinear
    float det = static_cast<float>(Math::signum(glm::determinant(glm::mat2(path[index].v_pos - path[index - 1].v_pos, path[static_cast<size_t>(index) + 1].v_pos - path[index].v_pos))));


    //When the two segments are near colinear is the miter joint function
    if ((det == 0) || (Math::incidence_angle(ortho2, ortho) < MINIMUM_BEVEL_ANGLE)) {
        miter_joint(index, last, ortho, thickness, color);
        return 0;
    }


    //for first point
    uint32_t fp = static_cast<uint32_t>(mesh.size());
    for (float i = 0; i < 2; i++) {
        //Two bevel points
        bevel_correction = Math::direction_vector2D(path[index - 1].v_pos, path[index].v_pos) * glm::sin(angle * 0.5f) * _aa_thickness;
        p2 = path[index].v_pos - det * ortho * (thickness + i * _aa_thickness) + i * bevel_correction;

        bevel_correction = Math::direction_vector2D(path[index].v_pos, path[static_cast<size_t>(index) + 1].v_pos) * glm::sin(angle * 0.5f) * _aa_thickness;
        p3 = path[index].v_pos - det * ortho2 * (thickness + i * _aa_thickness) - i * bevel_correction;

        mesh.emplace_back(p2, color * glm::vec4(1, 1, 1, 1 - i));
        mesh.emplace_back(p3, color * glm::vec4(1, 1, 1, 1 - i));

        //pivot
        p1 = path[index - 1].v_pos  + det * ortho * (thickness + i * _aa_thickness);
        p2 = path[index].v_pos      + det * ortho * (thickness + i * _aa_thickness);
        p3 = path[index].v_pos      + det * ortho2 * (thickness + i * _aa_thickness);
        p4 = path[static_cast<size_t>(index) + 1].v_pos  + det * ortho2 * (thickness + i * _aa_thickness);

        mesh.emplace_back(Math::intersection_point(p1, p2, p3, p4), color * glm::vec4(1, 1, 1, 1 - i));

    }


    //Bevel triangle and aa
    indices.emplace_back(fp, fp + 1, fp + 2);
    quad_index(fp + 3, fp, fp + 1, fp + 4);

    if (det > 0) {
        cur.update(fp + 2, fp + 1, fp + 5, fp + 4);

        quad_index(last.top, last.btm, fp, cur.top);
        quad_index(last.aa_top, last.top, cur.top, cur.aa_top);
        quad_index(last.btm, last.aa_btm, fp + 3, fp);
    }
    else {
        cur.update(fp + 1, fp + 2, fp + 4, fp + 5);

        quad_index(last.top, last.btm, cur.btm, fp);
        quad_index(last.aa_top, last.top, fp, fp + 3);
        quad_index(last.btm, last.aa_btm, cur.aa_btm, cur.btm);
    }

    ortho = ortho2;
    last = cur;

    return 0;
}



uint8_t Polyline::fan_joint( uint32_t index, Mesh_info& last, 
    glm::vec3& ortho, float thickness, glm::vec4 color)
{
    glm::vec3 ortho2, position, line, direction;
    glm::vec3 p1, p2, p3, p4, pivot;
    glm::mat3 rotation;

    //Give the angle between the previous and next segment
    double angle = Math::incidence_angle(path[index].v_pos - path[index - 1].v_pos, path[index + static_cast<size_t>(1)].v_pos - path[index].v_pos);
    double const count_d = floor((angle) / MINIMUM_FAN_ANGLE);
    uint32_t const count = static_cast<uint32_t>(count_d);
    uint32_t fp;


    ortho2 = Math::ortho_vector(path[index].v_pos, path[static_cast<size_t>(index) + 1].v_pos);

    //Give the relative orientation between the two vectors
    float det = static_cast<float>(Math::signum(glm::determinant(glm::mat2(path[index].v_pos - path[index - 1].v_pos, path[static_cast<size_t>(index) + 1].v_pos - path[index].v_pos))));

    if ((det == 0) || (Math::incidence_angle(ortho2, ortho) < MINIMUM_FAN_ANGLE)) {
        miter_joint(index, last, ortho, thickness, color);
        return 0;
    }

    indices.reserve(indices.size() + 3 * (static_cast<size_t>(count) + 2) + 1);
    mesh.reserve(mesh.size() + 2 * (static_cast<size_t>(count) + 2));

    
    
    Mesh_info cur;

    fp = static_cast<uint32_t>(mesh.size());
    position = path[index].v_pos;

    if (det > 0) {
        //give the indice of the first vertice
        cur.update(fp + 2 * count + 2, fp, fp + 2 * count + 3, fp + 1);

        quad_index(last.aa_top, last.top, cur.top, cur.aa_top);
        quad_index(last.top, last.btm, cur.btm, cur.top);
        quad_index(last.btm, last.aa_btm, cur.aa_btm, cur.btm);

        //IBO
        p1 = path[index].v_pos - det * ortho * thickness;
        p2 = path[index].v_pos - det * ortho * (thickness + _aa_thickness);
        mesh.emplace_back(p1, color);
        mesh.emplace_back(p2, color * fade);

        direction = Math::direction_vector2D(position, p1);

        //fan meshing
        for (uint32_t j = 0; j < count; j++) {
            rotation = Math::rotation_matrix2D_ccw(angle / count_d * (static_cast<double>(j) + 1));

            line = thickness * direction * rotation;
            mesh.emplace_back(position + line, color);
            line = (thickness + _aa_thickness) * direction * rotation;
            mesh.emplace_back(position + line, color * fade);

            indices.emplace_back(cur.top, cur.btm + 2 * j, cur.btm + 2 + 2 * j);
            quad_index(cur.btm + 2 * j, cur.btm + 1 + 2 * j, cur.btm + 2 * j + 3, cur.btm + 2 * j + 2);
        }

        cur.update(fp + 2 * count + 2, fp + 2 * count, fp + 2 * count + 3, fp + 2 * count + 1);
    }
    else {
        cur.update(fp, fp + 2 * count + 2, fp + 1, fp + 2 * count + 3);

        quad_index(last.aa_top, last.top, cur.top, cur.aa_top);
        quad_index(last.top, last.btm, cur.btm, cur.top);
        quad_index(last.btm, last.aa_btm, cur.aa_btm, cur.btm);

        //IBO
        p1 = path[index].v_pos - det * ortho * thickness;
        p2 = path[index].v_pos - det * ortho * (thickness + _aa_thickness);
        mesh.emplace_back(p1, color);
        mesh.emplace_back(p2, color * fade);

        direction = Math::direction_vector2D(position, p1);

        //fan meshing
        for (uint32_t j = 0; j < count; j++) {
            rotation = Math::rotation_matrix2D_cw(angle / count_d * (static_cast<double>(j) + 1));

            line = direction * rotation * thickness;
            mesh.emplace_back(position + line, color);
            line = direction * rotation * (thickness + _aa_thickness);
            mesh.emplace_back(position + line, color * fade);

            indices.emplace_back(cur.btm, cur.top + 2 * j, cur.top + 2 + 2 * j);
            quad_index(cur.top + 2 * j, cur.top + 1 + 2 * j, cur.top + 2 * j + 3, cur.top + 2 * j + 2);
        }

        cur.update(fp + 2 * count, fp + 2 * count + 2, fp + 2 * count + 1, fp + 2 * count + 3);
    }

    //Intersection between the two segments to add the pivot
    for (uint16_t i = 0; i < 2; i++) {
        p1 = path[index - 1].v_pos  + det * ortho * (thickness + i * _aa_thickness);
        p2 = path[index].v_pos      + det * ortho * (thickness + i * _aa_thickness);
        p3 = path[index].v_pos      + det * ortho2 * (thickness + i * _aa_thickness);
        p4 = path[static_cast<size_t>(index) + 1].v_pos  + det * ortho2 * (thickness + i * _aa_thickness);

        pivot = Math::intersection_point(p1, p2, p3, p4);
        mesh.emplace_back(pivot, color * glm::vec4(1, 1, 1, 1 - i));
    }

    last = cur;
    ortho = ortho2;

    return 0;
}

SSS_GL_END;