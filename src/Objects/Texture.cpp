#include "GL/Objects/Texture.hpp"
#include "GL/Objects/Models/Plane.hpp"
#include "GL/Objects/Shaders.hpp"
#include "GL/Window.hpp"

#include <FastNoise/FastNoise.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning(suppress : 4996)
#include <stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include <stb_image.h>

SSS_GL_BEGIN;

std::string Texture::_resource_folder;


void Texture::_register()
{
    REGISTER_EVENT("SSS_TEXTURE_RESIZE"); 
    REGISTER_EVENT("SSS_TEXTURE_CONTENT"); 
    REGISTER_EVENT("SSS_TEXTURE_LOADED"); 
}

Texture::Texture() try
    : _raw_texture(GL_TEXTURE_2D_ARRAY)
{
    _raw_texture.bind();
    _raw_texture.parameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    _raw_texture.parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Ensure frames vector has a default single frame (previously initialized inline)
    _frames.resize(1);
    _frames.total_time = std::chrono::nanoseconds(0);
    _frames.w = 0;
    _frames.h = 0;

    _observe(_loading_thread);

    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().life_state)) {
        LOG_GL_MSG("Texture -> created");
    }
}
CATCH_AND_RETHROW_METHOD_EXC;

Texture::~Texture()
{
    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().life_state)) {
        LOG_GL_MSG("Texture -> deleted");
    }
}

Texture::Shared Texture::create(std::string const& filepath)
{
    Shared ret = create();
    ret->loadImage(filepath);
    return ret;
}

Texture::Shared Texture::create(std::filesystem::path const& filepath)
{
    Shared ret = create();
    ret->loadImage(filepath.string());
    return ret;
}

Texture::Shared Texture::create(TR::Area::Shared area)
{
    Shared ret = create();
    ret->setTextArea(area);
    return ret;
}

Texture::Shared Texture::createCellularNoise(int width, int height, float frequency, int seed)
{
    auto fnGenerator = FastNoise::New<FastNoise::CellularValue>();
    // CellularValue defaults to Scale=100 (Frequency=0.01), which would stack
    // with the xStepSize/yStepSize below and make `frequency` ~100x weaker
    // than expected. Neutralize it so `frequency` is the only frequency knob.
    fnGenerator->SetScale(1.0f);
    std::vector<float> noise(static_cast<size_t>(width) * static_cast<size_t>(height));
    fnGenerator->GenUniformGrid2D(noise.data(), 0, 0, width, height, frequency, frequency, seed);

    RGBA32::Vector pixels(noise.size());
    for (size_t i = 0; i < noise.size(); ++i) {
        uint8_t const v = static_cast<uint8_t>(std::clamp((noise[i] * 0.5f + 0.5f) * 255.f, 0.f, 255.f));
        pixels[i] = RGBA32(v, v, v, 255);
    }

    Shared ret = create();
    ret->editRawPixels(pixels.data(), width, height);
    return ret;
}

Texture::Shared Texture::createSDF(std::vector<UIPrimitive> prims, int width, int height)
{
    Shared ret = create();
    ret->setSDF(std::move(prims), width, height);
    return ret;
}

void Texture::setType(Type type) noexcept
{
    if (_type != type)
        _internalEdit(type);
}

void Texture::setUVMode(UVMode mode) noexcept
{
    if (_uv_mode == mode) {
        return;
    }
    _uv_mode = mode;
    _updateWrapParams();
}

void Texture::setRepeat(bool repeat) noexcept
{
    if (_repeat == repeat) {
        return;
    }
    _repeat = repeat;
    _updateWrapParams();
}

void Texture::_updateWrapParams() noexcept
{
    if (_uv_mode == UVMode::Polar) {
        // Angle (S) always wraps around the 0/1 seam to avoid a visible
        // discontinuity. Radius (T) follows _repeat: clamping (default) avoids
        // ring artifacts from polar mapping naturally pushing radius past 1.0
        // in the UV square's corners; repeating lets an animated radius
        // offset (radiate effect) tile seamlessly instead of clamping to the
        // edge pixel once offset + radius exceeds 1.0 (which otherwise freezes
        // most of the visible disk and warps the remainder toward the center).
        _raw_texture.parameteri(GL_TEXTURE_WRAP_S, GL_REPEAT);
        _raw_texture.parameteri(GL_TEXTURE_WRAP_T, _repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    }
    else {
        GLint const wrap = _repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
        _raw_texture.parameteri(GL_TEXTURE_WRAP_S, wrap);
        _raw_texture.parameteri(GL_TEXTURE_WRAP_T, wrap);
    }
}

void Texture::loadImage(std::string const& filepath)
{
    _loading_thread.run(_resource_folder, filepath);
    _filepath = filepath;
}

void Texture::editRawPixels(void const* pixels, int width, int height) try
{
    if (_frames.size() != 1) {
        _frames.resize(1);
    }
    _frames.w = width;
    _frames.h = height;
    // Replace previous pixel storage
    uint32_t const* ptr = reinterpret_cast<uint32_t const*>(pixels);
    _frames[0].pixels = RGBA32::Vector(ptr, ptr + (width * height));

    // Update plane type and scaling
    _internalEdit(Type::Raw);

    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().edit)) {
        LOG_GL_MSG("Texture -> edit");
    }
}
CATCH_AND_RETHROW_METHOD_EXC;

void Texture::setColor(RGBA32 color)
{
    editRawPixels(&color, 1, 1);
}

void Texture::setSDF(std::vector<UIPrimitive> prims, int width, int height) try
{
    _sdf_prims = std::move(prims);
    _frames.resize(1);
    _frames[0].pixels.clear();
    _frames.total_time = std::chrono::nanoseconds(0);
    _frames.w = width;
    _frames.h = height;
    _internalEdit(Type::SDF);
}
CATCH_AND_RETHROW_METHOD_EXC;

void Texture::_renderSDF()
{
    int const w = _frames.w, h = _frames.h;
    if (w <= 0 || h <= 0 || _sdf_prims.empty())
        return;
    auto shader = Window::getPresetShaders(static_cast<uint32_t>(Shaders::Preset::PlaneSDF));
    if (!shader)
        return;

    // Save the state changed below
    GLint prev_fbo = 0, prev_viewport[4]{};
    GLfloat prev_clear[4]{};
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prev_fbo);
    glGetIntegerv(GL_VIEWPORT, prev_viewport);
    glGetFloatv(GL_COLOR_CLEAR_VALUE, prev_clear);
    GLboolean const blend = glIsEnabled(GL_BLEND);
    GLboolean const depth = glIsEnabled(GL_DEPTH_TEST);

    // Render target: layer 0 of the texture array
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _raw_texture.id, 0, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        // Unit quad, same layout as the planes' (pos xyz, uv)
        constexpr float vertices[] = {
            -0.5f,  0.5f, 0.0f,   0.f, 1.f,
            -0.5f, -0.5f, 0.0f,   0.f, 0.f,
             0.5f, -0.5f, 0.0f,   1.f, 0.f,
             0.5f,  0.5f, 0.0f,   1.f, 1.f,
        };
        constexpr unsigned indices[] = { 0, 1, 2, 2, 3, 0 };
        GLuint vao = 0, buffers[3]{};
        glGenVertexArrays(1, &vao);
        glGenBuffers(3, buffers);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[2]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, _sdf_prims.size() * sizeof(UIPrimitive),
            _sdf_prims.data(), GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffers[2]);

        // The shader composites every primitive itself and outputs straight
        // alpha: store it as is, without blending with the cleared target.
        glViewport(0, 0, w, h);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Quad y = -0.5 lands on texel row 0, which planes sample as their
        // top edge: scaling local coordinates by the pixel size gives Y-down
        // pixel coordinates centered on the texture.
        shader->use();
        shader->setUniform("u_VP", glm::scale(glm::mat4(1.f), glm::vec3(2.f, 2.f, 1.f)));
        shader->setUniform("u_Model", glm::mat4(1.f));
        shader->setUniform("u_LocalScale", glm::vec2(w, h));
        shader->setUniform("u_SDFMode", 1);
        shader->setUniform("u_PrimSize", static_cast<int>(_sdf_prims.size()));
        shader->setUniform("u_Alpha", 1.f);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(3, buffers);
    }
    else {
        LOG_GL_MSG("Texture -> SDF render target is incomplete");
    }

    // Restore state
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prev_fbo));
    glDeleteFramebuffers(1, &fbo);
    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
    glClearColor(prev_clear[0], prev_clear[1], prev_clear[2], prev_clear[3]);
    if (blend) glEnable(GL_BLEND);
    if (depth) glEnable(GL_DEPTH_TEST);
}

void Texture::setTextArea(TR::Area::Shared area)
{
    _set(_area, area);
    _internalEdit(Type::Text);
}

void Texture::getCurrentDimensions(int& w, int& h) const noexcept
{
    if (_type == Type::Raw || _type == Type::SDF) {
        w = _frames.w;
        h = _frames.h;
    }
    else if (_type == Type::Text) {
        if (_area)
            _area->pixelsGetDimensions(w, h);
        else {
            w = 0;
            h = 0;
        }
    }
}

std::tuple<int, int> Texture::getCurrentDimensions() const noexcept
{
    int w, h;
    getCurrentDimensions(w, h);
    return std::make_tuple(w, h);
}

void Texture::savePNG() const
{
    int w, h;
    getCurrentDimensions(w, h);
    std::vector<uint8_t> pixels(4 * w * h);

    glGetTextureImage(_raw_texture.id, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.size(), &pixels[0]);

    std::string time = std::format("{:%Y-%m-%d_%H-%M-%S}", std::chrono::system_clock::now());
    size_t const index = time.find('.');
    if (index < time.size()) {
        time.resize(index);
    }

    std::string const name = time + "_" + THIS_ADDR + ".png";
    stbi_write_png(name.c_str(), w, h, 4, &pixels[0], 0);
}

void Texture::_subjectUpdate(Subject const& subject, Event const& event)
{
    if (subject.is<_AsyncLoading>()) {
        _frames = std::move(_loading_thread._frames);
        _internalEdit(Type::Raw);
        EMIT_EVENT("SSS_TEXTURE_LOADED");
    }
    else if (subject.is<TR::Area>()) {
        _internalEdit(Type::Text);
    }
}

void Texture::_AsyncLoading::_asyncFunction(std::string folder, std::string filepath)
{
    _frames.clear();
    _frames.total_time = std::chrono::nanoseconds(0);
    _frames.w = 0;
    _frames.h = 0;

    std::string path;
    if (path = folder + filepath; folder.empty() || !pathIsFile(path)) {
        if (path = pathWhich(filepath); !pathIsFile(path)) {
            throw_exc(CONTEXT_MSG("Found no file for given arguments", filepath));
        }
    }

    // Check if filepath ends with ".png"
    // If file is a PNG or APNG, use load_apng (which works for simple PNG files)
    // Else, use stbi functions
    static const std::string png(".png");
    // Ends with ".png"
    if (filepath.compare(filepath.length() - png.length(), png.length(), png) == 0) {
        // Load frames
        std::vector<_internal::APNGFrame> apng_frames;
        if (load_apng(path.c_str(), apng_frames) < 0) {
            SSS::throw_exc(CONTEXT_MSG("load_apng error", filepath));
        }
        // Copy data
        if (!apng_frames.empty()) {
            _frames.w = apng_frames[0].w;
            _frames.h = apng_frames[0].h;
            _frames.reserve(apng_frames.size());
            // Copy each frame one by one
            for (auto& apng_frame : apng_frames) {
                if (_beingCanceled()) return;
                // Copy pixels and compute delay
                auto& frame = _frames.emplace_back();
                uint32_t const* p = reinterpret_cast<uint32_t const*>(apng_frame.vec.data());
                frame.pixels = RGBA32::Vector(p, p + (_frames.w * _frames.h));
                if (apng_frame.delay_den > 0) {
                    frame.delay = std::chrono::nanoseconds(static_cast<int64_t>(1e9
                        * static_cast<double>(apng_frame.delay_num)
                        / static_cast<double>(apng_frame.delay_den)
                        ));
                }
                else
                    frame.delay = std::chrono::milliseconds(16);
                _frames.total_time += frame.delay;
                // Free raw pixels early
                apng_frame.vec.clear();
                apng_frame.rows.clear();
            }
        }
    }
    // Doesn't end with ".png"
    else {
        // Load image
        SSS::C_Ptr <uint32_t, void(*)(void*), stbi_image_free>
            raw_pixels(reinterpret_cast<uint32_t*>(stbi_load(
                path.c_str(),   // Filepath to picture
                &_frames.w,     // Width, to query
                &_frames.h,     // Height, to query
                nullptr,        // Byte composition, to query if not requested
                4               // Byte composition, to request (here RGBA32)
            )));
        // Throw if error
        if (raw_pixels == nullptr) {
            SSS::throw_exc(CONTEXT_MSG(stbi_failure_reason(), filepath));
        }
        // Fill vector
        if (_beingCanceled()) return;
        _frames.emplace_back().pixels =
            RGBA32::Vector(raw_pixels.get(), raw_pixels.get() + (_frames.w * _frames.h));
    }
}

void Texture::_internalEdit(Type type)
{
    _type = type;
    if (_type == Type::Raw) {
        if (_raw_texture.editSettings(_frames.w, _frames.h, static_cast<int>(_frames.size())))
        {
            EMIT_EVENT("SSS_TEXTURE_RESIZE");
        }
        for (uint32_t i = 0; i < _frames.size(); ++i) {
            _raw_texture.editPixels(_frames[i].pixels.data(), i);
        }
    }
    else if (_type == Type::Text) {
        int w = 0, h = 0;
        if (_area)
            _area->pixelsGetDimensions(w, h);
        if (_raw_texture.editSettings(w, h))
        {
            EMIT_EVENT("SSS_TEXTURE_RESIZE");
        }
        if (_area)
            _raw_texture.editPixels(_area->pixelsGet());
    }
    else if (_type == Type::SDF) {
        if (_raw_texture.editSettings(_frames.w, _frames.h))
        {
            EMIT_EVENT("SSS_TEXTURE_RESIZE");
        }
        _renderSDF();
    }

    if (_callback_f)
        _callback_f(*this);

    EMIT_EVENT("SSS_TEXTURE_CONTENT"); 
    // Log
    if (Log::GL::Texture::query(Log::GL::Texture::get().edit)) {
        LOG_GL_MSG("Texture -> edit");
    }
}


SSS_GL_END;