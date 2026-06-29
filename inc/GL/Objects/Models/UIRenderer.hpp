#ifndef SSS_GL_UIRENDERER_HPP
#define SSS_GL_UIRENDERER_HPP

#include "Line.hpp"
#include "../Renderer.hpp"
#include "PlaneRenderer.hpp"
#include "Plane.hpp"
#include "../Camera.hpp"
#include "../Shaders.hpp"
#include "../Basic.hpp"

namespace SSS { class Node_UI; }

SSS_GL_BEGIN;


// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

/** Unified ortho renderer for both textured planes (including text) and SDF shapes.
 *  Renders everything in a single pass with orthographic projection.
 */
class SSS_GL_API UIRenderer : public Observer, public Renderer<UIRenderer> {
    friend class SharedClass;
    friend class Window;

private:
    UIRenderer();

public:
    Camera::Shared camera;
    virtual void render() override;

    using SharedClass::create;
    virtual void _subjectUpdate(Subject const& subject, Event const& event) override;
    void updateResolution(const float _w, const float _h);
    void setWindow(Window* pWindow) { _window = pWindow; };

    /** Add UI node with SDF primitives. */
    void push(Node_UI* n);

    /** Register a Node_UI to be rendered in world-space (camera VP). */
    void pushWorld(Node_UI* n);

    /** Set the camera used for the world-space SDF pass. */
    void setWorldCamera(Camera::Shared cam) { _worldCamera = cam; }

    /** Add a plane (textured geometry, including text). */
    void addPlane(std::shared_ptr<PlaneBase> plane);
    /** Remove a plane. */
    void removePlane(std::shared_ptr<PlaneBase> plane);

private:
    GL::Window* _window;

    // For SDF shapes
    Basic::VAO _sdf_vao;
    Basic::VBO _sdf_vbo;
    GLuint _sdf_ssbo = 0;
    std::vector<int> _nodes;

    // For planes (text and textured geometry)
    std::vector<std::shared_ptr<PlaneBase>> _planes;
    Basic::VAO _plane_vao;
    Basic::VBO _plane_static_vbo;
    Basic::IBO _plane_static_ibo;
    Basic::VBO _plane_model_vbo;
    Basic::VBO _plane_alpha_vbo;
    Basic::VBO _plane_tex_offset_vbo;
    bool _planes_update_vbos = true;

    glm::vec2 _resolution;
    glm::mat4 _proj;

    Camera::Shared _worldCamera;
    std::vector<Node_UI*> _worldNodes;

    // Plane rendering helper
    void _setupPlaneVAO();
    void _updatePlaneVBOs();
    void _renderPlanes();
    void _renderSDFShapes();
    void _renderWorldSDFShapes();
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_UIRENDERER_HPP
