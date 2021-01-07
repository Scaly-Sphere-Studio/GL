#include "SSS/GL/Model.hpp"

__SSS_GL_BEGIN

Model::Model() try
{
    _vao.reset(new VAO);
    _vbo.reset(new VBO);
    _ibo.reset(new IBO);
}
__CATCH_AND_RETHROW_METHOD_EXC

Model::~Model()
{
}

void Model::scale(glm::vec3 scaling)
{
    _scaling = glm::scale(_scaling, scaling);
    _model_mat4 = _translation * _rotation * _scaling;
}

void Model::rotate(float radians, glm::vec3 axis)
{
    _rotation = glm::rotate(_rotation, radians, axis);
    _model_mat4 = _translation * _rotation * _scaling;
}

void Model::translate(glm::vec3 translation)
{
    _translation = glm::translate(_translation, translation);
    _model_mat4 = _translation * _rotation * _scaling;
}

void Model::resetTransformations(Transformation transformations)
{
    if (!!(transformations & Transformation::Scaling)) {
        _scaling = _og_scaling;
    }
    if (!!(transformations & Transformation::Rotation)) {
        _rotation = _og_rotation;
    }
    if (!!(transformations & Transformation::Translation)) {
        _translation = _og_translation;
    }
    _model_mat4 = _translation * _rotation * _scaling;
}

__SSS_GL_END