#ifndef SSS_GL_NOISE_HPP
#define SSS_GL_NOISE_HPP

#include "Texture.hpp"

/** @file
 *  Defines class SSS::GL::Noise.
 */

SSS_GL_BEGIN;

// Ignore warning about STL exports as they're private members
#pragma warning(push, 2)
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

/** Procedurally-generated texture driven by a noise algorithm.
 *  Wraps Texture with type/dimension/frequency/seed parameters
 *  and regenerates the underlying pixel data whenever they change.
 *  @sa Noise::create()
 */
class SSS_GL_API Noise : public Texture {
public:
    using Shared = std::shared_ptr<Noise>;

    ~Noise();

    /** Noise generation algorithm. */
    enum class Type {
        OpenSimplex2,     ///< Gradient noise, good general-purpose choice.
        OpenSimplex2S,    ///< Smoother variant of OpenSimplex2.
        Perlin,           ///< Classic Perlin gradient noise.
        Value,            ///< Value noise (blocky, fast).
        White,            ///< Pure white (random) noise.
        CellularValue,    ///< Voronoi cells colored by hashed cell value.
        CellularDistance, ///< Voronoi cells colored by distance to feature points.
    };

    /** Distance metric used by Cellular noise. */
    enum class CellularDistanceFunc {
        Euclidean,    ///< Standard straight-line distance.
        EuclideanSq,  ///< Squared Euclidean — faster, slightly different look.
        Manhattan,    ///< Taxicab / diamond-shaped cells.
        Hybrid,       ///< Mix of Euclidean and Manhattan.
        MaxAxis,      ///< Chebyshev / square-shaped cells.
    };

    /** Value returned per cell by CellularDistance noise. */
    enum class CellularReturnType {
        Index0,       ///< Distance to the closest feature point.
        Index0Add1,   ///< Closest + second-closest distance (smooth ridges).
        Index0Sub1,   ///< Closest - second-closest distance.
        Index0Mul1,   ///< Closest * second-closest distance.
        Index0Div1,   ///< Closest / second-closest distance.
    };

    /** Domain warp applied to input coordinates before sampling the base noise. */
    enum class DomainWarpType {
        None,     ///< No domain warp — coordinates sampled directly.
        Gradient, ///< Gradient domain warp — creates organic, fluid cell shapes.
    };

    /** Fractal layering applied on top of the base noise. */
    enum class FractalType {
        None,   ///< No fractal — single octave only.
        FBm,    ///< Fractional Brownian Motion — smooth, natural layering.
        Ridged, ///< Ridged multifractal — sharp ridges, good for mountains.
    };

    /** Creates a Noise texture with the given parameters and immediately generates it. */
    static Shared create(Type type = Type::Perlin, int width = 256, int height = 256,
                         float frequency = 0.02f, int seed = 1337);

    // --- Core parameters ---

    /** Sets the noise algorithm and regenerates. */
    void setNoiseType(Type type);
    Type getNoiseType() const noexcept { return _noise_type; }

    /** Resizes the texture and regenerates. */
    void setDimensions(int width, int height);
    int getWidth()  const noexcept { return _width; }
    int getHeight() const noexcept { return _height; }

    /** Sets the noise frequency and regenerates. Smaller values = larger features. */
    void setFrequency(float frequency);
    float getFrequency() const noexcept { return _frequency; }

    /** Sets the noise seed and regenerates. */
    void setSeed(int seed);
    int getSeed() const noexcept { return _seed; }

    // --- Cellular options (active when type is CellularValue or CellularDistance) ---

    /** Sets the cellular distance metric and regenerates. */
    void setCellularDistanceFunc(CellularDistanceFunc func);
    CellularDistanceFunc getCellularDistanceFunc() const noexcept { return _cell_dist_func; }

    /** Sets the cellular return value type (CellularDistance only) and regenerates. */
    void setCellularReturnType(CellularReturnType type);
    CellularReturnType getCellularReturnType() const noexcept { return _cell_return_type; }

    /** Sets the jitter displacing feature points from their grid positions and regenerates. */
    void setCellularGridJitter(float jitter);
    float getCellularGridJitter() const noexcept { return _cell_grid_jitter; }

    /** Sets the jitter that varies cell sizes and regenerates. */
    void setCellularSizeJitter(float jitter);
    float getCellularSizeJitter() const noexcept { return _cell_size_jitter; }

    /** Sets which neighbor cell's value to use (CellularValue only) and regenerates. */
    void setCellularValueIndex(int index);
    int getCellularValueIndex() const noexcept { return _cell_value_index; }

    /** Sets the primary neighbor index for return-type combinations (CellularDistance only) and regenerates. */
    void setCellularDistanceIndex0(int index);
    int getCellularDistanceIndex0() const noexcept { return _cell_dist_index0; }

    /** Sets the secondary neighbor index for return-type combinations (CellularDistance only) and regenerates. */
    void setCellularDistanceIndex1(int index);
    int getCellularDistanceIndex1() const noexcept { return _cell_dist_index1; }

    // --- Domain warp options ---

    /** Sets the domain warp algorithm and regenerates. */
    void setDomainWarpType(DomainWarpType type);
    DomainWarpType getDomainWarpType() const noexcept { return _warp_type; }

    /** Sets the domain warp amplitude and regenerates. Larger = more distortion. */
    void setDomainWarpAmplitude(float amplitude);
    float getDomainWarpAmplitude() const noexcept { return _warp_amplitude; }

    /** Sets the domain warp frequency and regenerates. */
    void setDomainWarpFrequency(float frequency);
    float getDomainWarpFrequency() const noexcept { return _warp_frequency; }

    // --- Fractal options ---

    /** Sets the fractal layering algorithm and regenerates. */
    void setFractalType(FractalType type);
    FractalType getFractalType() const noexcept { return _fractal_type; }

    /** Sets the number of fractal octaves [1-8] and regenerates. */
    void setFractalOctaves(int octaves);
    int getFractalOctaves() const noexcept { return _fractal_octaves; }

    /** Sets the frequency multiplier per fractal octave and regenerates. */
    void setFractalLacunarity(float lacunarity);
    float getFractalLacunarity() const noexcept { return _fractal_lacunarity; }

    /** Sets the amplitude multiplier per fractal octave and regenerates. */
    void setFractalGain(float gain);
    float getFractalGain() const noexcept { return _fractal_gain; }

    /** Unconditionally regenerates the noise texture with the current parameters. */
    void regenerate();

private:
    Noise();

    Type                 _noise_type       { Type::Perlin };
    int                  _width            { 256 };
    int                  _height           { 256 };
    float                _frequency        { 0.02f };
    int                  _seed             { 1337 };

    CellularDistanceFunc _cell_dist_func   { CellularDistanceFunc::Euclidean };
    CellularReturnType   _cell_return_type { CellularReturnType::Index0 };
    float                _cell_grid_jitter { 1.0f };
    float                _cell_size_jitter { 1.0f };
    int                  _cell_value_index { 0 };
    int                  _cell_dist_index0 { 0 };
    int                  _cell_dist_index1 { 1 };

    DomainWarpType       _warp_type        { DomainWarpType::None };
    float                _warp_amplitude   { 1.0f };
    float                _warp_frequency   { 0.5f };

    FractalType          _fractal_type     { FractalType::None };
    int                  _fractal_octaves  { 3 };
    float                _fractal_lacunarity{ 2.0f };
    float                _fractal_gain     { 0.5f };

    void _generate();
};

#pragma warning(pop)

SSS_GL_END;

#endif // SSS_GL_NOISE_HPP
