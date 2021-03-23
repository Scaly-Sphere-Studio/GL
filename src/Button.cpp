#include "SSS/GL/Button.hpp"

__SSS_GL_BEGIN

Button::Button()
    : Plane::Plane()
{
}

Button::Button(std::string const& filepath)
    : Plane::Plane(filepath)
{
}

bool Button::_WasItPressed(double x, double y)
{
    glm::vec2 u = getModelMat4() * glm::vec4(-0.5, -0.5, 0, 1);
    glm::vec2 v = getModelMat4() * glm::vec4(0.5, 0.5, 0, 1);
    //__LOG_MSG(SSS::toString(u[0]) + " ; " + SSS::toString(u[1]) + " ... " + SSS::toString(v[0]) + " ; " + SSS::toString(v[1]))
    return (x > u[0] && x < v[0] && y > u[1] && y < v[1]);
}

__SSS_GL_END