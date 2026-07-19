#pragma once

#include <GL/Animations/Animation.hpp>
#include <GL/Objects/Texture.hpp>
#include <FastNoise/FastNoise.h>

SSS_BEGIN

class SSS_GL_API ShakeGenerator : public Track {
public:
    struct ShakeParams {
        float amplitude     = 55.0f;
        float frequency     = 3.0f;   // oscillations per animation cycle (integer → seamless loop)
        float decay         = 15.0f;  // exponential decay rate (ignored in Loop mode)
        int   seed          = 25;
        glm::vec3 direction = glm::vec3{ 1.0f, 0.7f, 0.0f };

        ShakeParams() = default;
        ShakeParams(float amp, float freq, float dec, glm::vec3 dir)
            : amplitude(amp), frequency(freq), decay(dec), direction(dir) {}
    };

    ShakeGenerator(float amp, float freq, float dec, glm::vec3 dir = glm::vec3{ 1.0f, 1.0f, 0.0f })
    {
        _duration = 1s;
        params = ShakeParams{ amp, freq, dec, dir };
        _initNoise();
    }

    ShakeGenerator(const ShakeParams& p = ShakeParams()) : params(p)
    {
        _duration = 1s;
        _initNoise();
    }

    ShakeParams params;
    glm::vec3 shakeOffset = glm::vec3{ 0 };

    enum class Axis { X, Y, Z };

    glm::vec2 shake2D();
    glm::vec3 shake3D();
    GL::Texture::Shared noiseTexture(Axis axis = Axis::X, int width = 256, int height = 128) const;

public:
    FastNoise::SmartNode<FastNoise::Generator> _noiseX;
    FastNoise::SmartNode<FastNoise::Generator> _noiseY;
    FastNoise::SmartNode<FastNoise::Generator> _noiseZ;

    void _initNoise()
    {
        auto make = []() -> FastNoise::SmartNode<FastNoise::Generator> {
            auto n = FastNoise::New<FastNoise::Simplex>();
            n->SetScale(1.0f);
            return n;
        };
        _noiseX = make();
        _noiseY = make();
        _noiseZ = make();
    }

    // Samples simplex noise on a circle in 2D noise space.
    // frequency is rounded to an integer so t=0 and t=1 land on the same point,
    // guaranteeing a seamless loop. The fixed radius keeps the circle large enough
    // to cross zero-regions regardless of seed, preventing one-directional drift.
    float _circleNoise(const FastNoise::SmartNode<FastNoise::Generator>& node,
                       float t, float phase, int seed) const
    {
        static constexpr float NOISE_RADIUS = 4.0f;
        float angle = 2.0f * glm::pi<float>() * std::round(params.frequency) * t + phase;
        float cx = std::cos(angle) * NOISE_RADIUS;
        float cy = std::sin(angle) * NOISE_RADIUS;
        // GenUniformGrid2D uses step-size semantics: position = xStart + i * freq.
        // A 1x1 grid always lands at (xStart, yStart) regardless of cx/cy.
        // Use a 2x2 grid so the [1,1] sample reaches (0 + 1*cx, 0 + 1*cy) = (cx, cy).
        float grid[4];
        node->GenUniformGrid2D(grid, 0, 0, 2, 2, cx, cy, seed);
        return grid[3]; // index [y=1][x=1] in row-major order
    }
};


inline glm::vec2 ShakeGenerator::shake2D()
{
    if (!_playing || _duration == 0s) return glm::vec2{};
    update();

    float t     = static_cast<float>(normalizedTime());
    float decay = (_mode == LectureMode::Loop) ? 1.0f : std::exp(-params.decay * t);
    float phase = glm::pi<float>() * 2.0f / 3.0f;

    shakeOffset.x = _circleNoise(_noiseX, t, 0.0f, params.seed)     * params.amplitude * decay * params.direction.x;
    shakeOffset.y = _circleNoise(_noiseY, t, phase, params.seed + 7) * params.amplitude * decay * params.direction.y;
    shakeOffset.z = 0.0f;

    return glm::vec2{ shakeOffset.x, shakeOffset.y };
}

inline glm::vec3 ShakeGenerator::shake3D()
{
    if (!_playing || _duration == 0s) return glm::vec3{};
    update();

    float t     = static_cast<float>(normalizedTime());
    float decay = (_mode == LectureMode::Loop) ? 1.0f : std::exp(-params.decay * t);
    float phase = glm::pi<float>() * 2.0f / 3.0f;

    shakeOffset.x = _circleNoise(_noiseX, t, 0.0f,        params.seed)      * params.amplitude * decay * params.direction.x;
    shakeOffset.y = _circleNoise(_noiseY, t, phase,        params.seed + 7)  * params.amplitude * decay * params.direction.y;
    shakeOffset.z = _circleNoise(_noiseZ, t, phase * 2.0f, params.seed + 13) * params.amplitude * decay * params.direction.z;

    return shakeOffset;
}


inline GL::Texture::Shared ShakeGenerator::noiseTexture(Axis axis, int width, int height) const
{
    const FastNoise::SmartNode<FastNoise::Generator>* node;
    float phase;
    int   seed;
    switch (axis) {
    case Axis::Y: node = &_noiseY; phase = glm::pi<float>() * 2.0f / 3.0f; seed = params.seed + 7;  break;
    case Axis::Z: node = &_noiseZ; phase = glm::pi<float>() * 4.0f / 3.0f; seed = params.seed + 13; break;
    default:      node = &_noiseX; phase = 0.0f;                             seed = params.seed;      break;
    }

    RGBA32::Vector pixels(static_cast<size_t>(width) * height, RGBA32(20, 20, 20, 255));

    // Center zero line
    int zeroRow = height / 2;
    for (int x = 0; x < width; ++x)
        pixels[zeroRow * width + x] = RGBA32(60, 60, 60, 255);

    for (int x = 0; x < width; ++x) {
        float t   = static_cast<float>(x) / std::max(width - 1, 1);
        float val = _circleNoise(*node, t, phase, seed);  // in [-1, 1]
        int   row = static_cast<int>((1.0f - (val * 0.5f + 0.5f)) * (height - 1));
        row = std::clamp(row, 0, height - 1);
        pixels[row * width + x] = RGBA32(255, 255, 255, 255);
        if (row > 0)
            pixels[(row - 1) * width + x] = RGBA32(160, 160, 160, 255);
        if (row < height - 1)
            pixels[(row + 1) * width + x] = RGBA32(160, 160, 160, 255);
    }

    auto tex = GL::Texture::create();
    tex->editRawPixels(pixels.data(), width, height);
    return tex;
}

SSS_END;
