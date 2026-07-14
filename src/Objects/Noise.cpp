#include "GL/Objects/Noise.hpp"

#include <FastNoise/FastNoise.h>
#include <algorithm>

SSS_GL_BEGIN;

Noise::Noise()
    : Texture()
{
}

Noise::~Noise() = default;

Noise::Shared Noise::create(Type type, int width, int height, float frequency, int seed)
{
    Shared ret(new Noise());
    ret->_noise_type = type;
    ret->_width      = width;
    ret->_height     = height;
    ret->_frequency  = frequency;
    ret->_seed       = seed;
    ret->_generate();
    return ret;
}

void Noise::setNoiseType(Type type)
{
    if (_noise_type != type) {
        _noise_type = type;
        _generate();
    }
}

void Noise::setDimensions(int width, int height)
{
    if (_width != width || _height != height) {
        _width  = width;
        _height = height;
        _generate();
    }
}

void Noise::setFrequency(float frequency)
{
    if (_frequency != frequency) {
        _frequency = frequency;
        _generate();
    }
}

void Noise::setSeed(int seed)
{
    if (_seed != seed) {
        _seed = seed;
        _generate();
    }
}

void Noise::setCellularDistanceFunc(CellularDistanceFunc func)
{
    if (_cell_dist_func != func) {
        _cell_dist_func = func;
        _generate();
    }
}

void Noise::setCellularReturnType(CellularReturnType type)
{
    if (_cell_return_type != type) {
        _cell_return_type = type;
        _generate();
    }
}

void Noise::setCellularGridJitter(float jitter)
{
    if (_cell_grid_jitter != jitter) {
        _cell_grid_jitter = jitter;
        _generate();
    }
}

void Noise::setCellularSizeJitter(float jitter)
{
    if (_cell_size_jitter != jitter) {
        _cell_size_jitter = jitter;
        _generate();
    }
}

void Noise::setCellularValueIndex(int index)
{
    if (_cell_value_index != index) {
        _cell_value_index = index;
        _generate();
    }
}

void Noise::setCellularDistanceIndex0(int index)
{
    if (_cell_dist_index0 != index) {
        _cell_dist_index0 = index;
        _generate();
    }
}

void Noise::setCellularDistanceIndex1(int index)
{
    if (_cell_dist_index1 != index) {
        _cell_dist_index1 = index;
        _generate();
    }
}

void Noise::setDomainWarpType(DomainWarpType type)
{
    if (_warp_type != type) {
        _warp_type = type;
        _generate();
    }
}

void Noise::setDomainWarpAmplitude(float amplitude)
{
    if (_warp_amplitude != amplitude) {
        _warp_amplitude = amplitude;
        _generate();
    }
}

void Noise::setDomainWarpFrequency(float frequency)
{
    if (_warp_frequency != frequency) {
        _warp_frequency = frequency;
        _generate();
    }
}

void Noise::setFractalType(FractalType type)
{
    if (_fractal_type != type) {
        _fractal_type = type;
        _generate();
    }
}

void Noise::setFractalOctaves(int octaves)
{
    if (_fractal_octaves != octaves) {
        _fractal_octaves = octaves;
        _generate();
    }
}

void Noise::setFractalLacunarity(float lacunarity)
{
    if (_fractal_lacunarity != lacunarity) {
        _fractal_lacunarity = lacunarity;
        _generate();
    }
}

void Noise::setFractalGain(float gain)
{
    if (_fractal_gain != gain) {
        _fractal_gain = gain;
        _generate();
    }
}

void Noise::regenerate()
{
    _generate();
}

namespace {

FastNoise::DistanceFunction toFNDistFunc(Noise::CellularDistanceFunc f)
{
    switch (f) {
    case Noise::CellularDistanceFunc::EuclideanSq: return FastNoise::DistanceFunction::EuclideanSquared;
    case Noise::CellularDistanceFunc::Manhattan:   return FastNoise::DistanceFunction::Manhattan;
    case Noise::CellularDistanceFunc::Hybrid:      return FastNoise::DistanceFunction::Hybrid;
    case Noise::CellularDistanceFunc::MaxAxis:     return FastNoise::DistanceFunction::MaxAxis;
    default:                                        return FastNoise::DistanceFunction::Euclidean;
    }
}

FastNoise::CellularDistance::ReturnType toFNReturnType(Noise::CellularReturnType r)
{
    switch (r) {
    case Noise::CellularReturnType::Index0Add1: return FastNoise::CellularDistance::ReturnType::Index0Add1;
    case Noise::CellularReturnType::Index0Sub1: return FastNoise::CellularDistance::ReturnType::Index0Sub1;
    case Noise::CellularReturnType::Index0Mul1: return FastNoise::CellularDistance::ReturnType::Index0Mul1;
    case Noise::CellularReturnType::Index0Div1: return FastNoise::CellularDistance::ReturnType::Index0Div1;
    default:                                    return FastNoise::CellularDistance::ReturnType::Index0;
    }
}

} // namespace

void Noise::_generate()
{
    if (_width <= 0 || _height <= 0)
        return;

    // Build the base generator node.
    // SetScale(1.0f) is called on each concrete type to neutralize the default
    // Scale=100 so that _frequency (via GenUniformGrid2D step sizes) is the
    // sole frequency control.
    FastNoise::SmartNode<FastNoise::Generator> base;
    switch (_noise_type) {
    case Type::OpenSimplex2: {
        auto n = FastNoise::New<FastNoise::Simplex>();
        n->SetScale(1.0f);
        base = n;
        break;
    }
    case Type::OpenSimplex2S: {
        auto n = FastNoise::New<FastNoise::SuperSimplex>();
        n->SetScale(1.0f);
        base = n;
        break;
    }
    case Type::Perlin: {
        auto n = FastNoise::New<FastNoise::Perlin>();
        n->SetScale(1.0f);
        base = n;
        break;
    }
    case Type::Value: {
        auto n = FastNoise::New<FastNoise::Value>();
        n->SetScale(1.0f);
        base = n;
        break;
    }
    case Type::White: {
        auto n = FastNoise::New<FastNoise::White>();
        base = n;
        break;
    }
    case Type::CellularValue: {
        auto n = FastNoise::New<FastNoise::CellularValue>();
        n->SetScale(1.0f);
        n->SetDistanceFunction(toFNDistFunc(_cell_dist_func));
        n->SetGridJitter(_cell_grid_jitter);
        n->SetSizeJitter(_cell_size_jitter);
        n->SetValueIndex(_cell_value_index);
        base = n;
        break;
    }
    case Type::CellularDistance: {
        auto n = FastNoise::New<FastNoise::CellularDistance>();
        n->SetScale(1.0f);
        n->SetDistanceFunction(toFNDistFunc(_cell_dist_func));
        n->SetReturnType(toFNReturnType(_cell_return_type));
        n->SetGridJitter(_cell_grid_jitter);
        n->SetSizeJitter(_cell_size_jitter);
        n->SetDistanceIndex0(_cell_dist_index0);
        n->SetDistanceIndex1(_cell_dist_index1);
        base = n;
        break;
    }
    }

    // Optionally wrap in a fractal node
    FastNoise::SmartNode<FastNoise::Generator> root = base;
    switch (_fractal_type) {
    case FractalType::FBm: {
        auto frac = FastNoise::New<FastNoise::FractalFBm>();
        frac->SetSource(base);
        frac->SetOctaveCount(_fractal_octaves);
        frac->SetLacunarity(_fractal_lacunarity);
        frac->SetGain(_fractal_gain);
        root = frac;
        break;
    }
    case FractalType::Ridged: {
        auto frac = FastNoise::New<FastNoise::FractalRidged>();
        frac->SetSource(base);
        frac->SetOctaveCount(_fractal_octaves);
        frac->SetLacunarity(_fractal_lacunarity);
        frac->SetGain(_fractal_gain);
        root = frac;
        break;
    }
    default:
        break;
    }

    // Optionally apply domain warp: warps input coordinates before the cellular lookup,
    // giving cells organic fluid shapes while preserving crisp Voronoi edges.
    if (_warp_type != DomainWarpType::None) {
        auto warpNode = FastNoise::New<FastNoise::DomainWarpGradient>();
        warpNode->SetSource(root);
        warpNode->SetWarpAmplitude(_warp_amplitude);
        warpNode->SetScale(_warp_frequency);
        root = warpNode;
    }

    std::vector<float> noise(static_cast<size_t>(_width) * static_cast<size_t>(_height));
    root->GenUniformGrid2D(noise.data(), 0, 0, _width, _height, _frequency, _frequency, _seed);

    RGBA32::Vector pixels(noise.size());
    for (size_t i = 0; i < noise.size(); ++i) {
        uint8_t const v = static_cast<uint8_t>(
            std::clamp((noise[i] * 0.5f + 0.5f) * 255.f, 0.f, 255.f));
        pixels[i] = RGBA32(v, v, v, 255);
    }
    editRawPixels(pixels.data(), _width, _height);
}

SSS_GL_END;
