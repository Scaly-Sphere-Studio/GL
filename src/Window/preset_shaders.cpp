#include "GL/Window.hpp"
#include "GL/Objects/Shaders.hpp"

SSS_GL_BEGIN;

static void _planeShadersData(std::string& vertex, std::string& fragment)
{
    vertex = R"(
#version 330 core
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec2 a_UV;

layout(location = 2) in mat4 a_Model;

layout(location = 6) in float a_Alpha;
layout(location = 7) in uint a_TextureOffset;

uniform mat4 u_VP;

out vec3 UVW;
out float Alpha;
flat out int instanceID;

void main()
{
    gl_Position = u_VP * a_Model * vec4(a_Pos, 1);
    UVW = vec3(a_UV, a_TextureOffset);
    Alpha = a_Alpha;
    instanceID = gl_InstanceID;
}
)";

    fragment = R"(
#version 330 core
out vec4 FragColor;

in vec3 UVW;
in float Alpha;
flat in int instanceID;

uniform sampler2DArray u_Textures[gl_MaxTextureImageUnits];
// Per-Texture UV mapping mode: 0 = Cartesian, 1 = Polar (matches Texture::UVMode)
uniform int u_UVModes[gl_MaxTextureImageUnits];
// Per-Texture UV offset. Cartesian: added directly to UV (pan). Polar: x = angle offset (turns), y = radius offset
uniform vec2 u_UVOffsets[gl_MaxTextureImageUnits];

#define _TWO_PI 6.28318530718

void main()
{
    vec2 uv = UVW.xy;
    if (u_UVModes[instanceID] == 1) {
        vec2 c = uv - vec2(0.5);
        float angle = atan(c.y, c.x) / _TWO_PI + 0.5 + u_UVOffsets[instanceID].x;
        float radius = length(c) * 2.0 + u_UVOffsets[instanceID].y;
        uv = vec2(angle, radius);
    } else {
        uv += u_UVOffsets[instanceID];
    }
    FragColor = texture(u_Textures[instanceID], vec3(uv, UVW.z));
    FragColor.w *= Alpha;
}
)";
}

static void _planeSDFShadersData(std::string& vertex, std::string& fragment)
{
    vertex = R"(
#version 430 core

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec2 a_UV;

uniform mat4 u_VP;
uniform mat4 u_Model;

out vec2 v_LocalXY;   // [-0.5, 0.5] local plane space -> SDF input
out vec2 v_UV;        // [0, 1] texture UV -> mask mode

void main()
{
    gl_Position = u_VP * u_Model * vec4(a_Pos, 1.0);
    v_LocalXY = a_Pos.xy;
    v_UV      = a_UV;
}
)";

    fragment = R"(
#version 430 core

#define _PI 3.14159265359

uniform int   u_SDFMode;     // matches PlaneBase::SDFMode: 0=None (unused here), 1=Shape, 2=Mask
uniform int   u_PrimSize;
uniform float u_Alpha;
uniform float u_Progress;
uniform int   u_TexOffset;         // APNG frame index, Mask mode only
uniform sampler2DArray u_Texture;  // Mask mode only
uniform int   u_UVMode;            // Texture::UVMode, Mask mode only: 0 = Cartesian, 1 = Polar
uniform vec2  u_UVOffset;          // Mask mode only. Cartesian: added directly to UV (pan). Polar: x = angle offset (turns), y = radius offset

in vec2 v_LocalXY;
in vec2 v_UV;

out vec4 FragColor;

// Enum SDF_Shapes copy declared in Shapes.hpp
#define SDCIRCLE        0
#define SDORIENTEDBOX   1
#define SDROUNDEDDBOX   2
#define SDSEGMENT       3
#define SDPIE           4
#define SDRING          5
#define SDARC           6
#define SDTRIANGLE      7
#define SDROUNEDX       8
#define SDCROSS         9
#define SDPENTAGON      10
#define SDHEXAGON       11
#define SDBEZIER_CUBIC  12
#define SDELLIPSE       13

// SDF Blend Flags
#define GROUP           1
#define SUBTRACT        2
#define INTERSECTION    4

// Color blending Flags
#define GRAYSCALE       1
#define GRADIENT        2
#define COLORMAP        4

struct UIPrimitive {
    vec2 pos;
    vec2 size;

    vec4 color;
    vec4 color2;

    vec4 border;
    float borderWidth;
    float cornerRadius;
    int   shapeId;
    int   blendMode;
    float innerRadius;  // 4 bytes
    float progress;     // 4 bytes
    vec2   pos2;
    vec2   pos3;        // 8 bytes
    vec2   pos4;        // 8 bytes

    float rotation;     // 4 bytes
    float scale;        // 4 bytes
    int   blendColor;   // 4 bytes
    int   gradientID;   // 4 bytes
};

layout(binding = 0, std430) readonly buffer Primitives {
    UIPrimitive primitives[]; // unsized array
};

// Transforms
vec2 rotate(const in vec2 samplePosition, const in float rotation){
    float angle     = rotation * _PI  * -1/180;
    float sine      = sin(angle);
    float cosine    = cos(angle);
    return vec2(cosine * samplePosition.x + sine * samplePosition.y, cosine * samplePosition.y - sine * samplePosition.x);
}

vec2 scale(const vec2 samplePosition, float scale){
    return samplePosition / scale;
}

vec2 translate(vec2 samplePosition, vec2 offset){
    return samplePosition - offset;
}

// Bezier
// iteratively improve the result using the newton method
// use 0 for the pure form of the approximation (and be surprised by how good it is!)
#define ITERATIONS 2

// implement all sorts of complex functions to compose the final analytic function,
// which is by it's very nature a conformal map. Meaning it will be a good approximation.
// complex exponential
vec2 cexp(vec2 c) {
    return exp(c.x)*vec2(cos(c.y), sin(c.y));
}
// complex logarithm
vec2 cln(vec2 c) {
    return vec2(log(dot(c,c))*.5, atan(c.y, c.x));
}
// complex multiplication
vec2 cmul(vec2 a, vec2 b) {
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}
// complex conjugation
vec2 conj(vec2 c) { return vec2(c.x, -c.y); }
// complex division
vec2 cdiv(vec2 a, vec2 b) {
    return cmul(a, conj(b)) / dot(b, b);
}
// complex sqrt
vec2 csqrt(vec2 a) {
    float r = length(a);
    if ((a.y + a.x) - a.x == 0.0) {
        return a.x >= 0.0 ? vec2(sqrt(r), 0.0) : vec2(0.0, sqrt(r));
    }
    vec2 h = a / r + vec2(1.0, 0.0);
    return h * sqrt(r / dot(h, h));
}
// complex cuberoot
vec2 ccbrt(vec2 a) {
    return cexp(cln(a)/3.0);
}

void cubic_roots(vec2 a, vec2 b, vec2 c, vec2 d, out vec2 x0, out vec2 x1, out vec2 x2) {
    // Cardano's formula for complex coefficients
    vec2 ac = cmul(a, c);
    vec2 bb = cmul(b, b);
    vec2 aa = cmul(a, a);
    vec2 d0 = bb - 3.0 * ac;
    vec2 d1 = 2.0 * cmul(b, bb) - 9.0 * cmul(ac, b) + 27.0 * cmul(aa, d);
    vec2 s = csqrt(cmul(d1, d1) - 4.0 * cmul(cmul(d0, d0), d0));
    vec2 opta = d1 - s;
    vec2 optb = d1 + s;
    vec2 opt = dot(opta,opta) < dot(optb,optb) ? optb : opta;
    vec2 cb = ccbrt(opt * 0.5);
    x0 = cdiv(b + cb + cdiv(d0, cb), -3.0 * a);
    vec2 root = vec2(-0.5, 0.866025403784439);
    cb = cmul(cb, root);
    x1 = cdiv(b + cb + cdiv(d0, cb), -3.0 * a);
    cb = cmul(cb, root);
    x2 = cdiv(b + cb + cdiv(d0, cb), -3.0 * a);
}
vec2 cubic(vec2 a, vec2 b, vec2 c, vec2 d, vec2 x) {
    return cmul(cmul(cmul(a, x) + b, x) + c, x) + d;
}
vec2 quadratic(vec2 a, vec2 b, vec2 c, vec2 x) {
    return cmul(cmul(a, x) + b, x) + c;
}
vec2 selectx(vec2 x0, vec2 x1) {
    return abs(x0.y) < abs(x1.y) ? x0 : x1;
}
float newton_quintic(float a, float b, float c, float d, float e, float f, float x0) {
    float v = ((((a * x0 + b) * x0 + c) * x0 + d) * x0 + e) * x0 + f;
    float dv = (((5.0 * a * x0 + 4.0 * b) * x0 + 3.0 * c) * x0 + 2.0 * d) * x0 + e;
    float ddv = ((20.0 * a * x0 + 12.0 * b) * x0 + 6.0 * c) * x0 + 2.0 * d;
    float p = dv / ddv;
    float q = v / ddv * 2.0;
    float dx = p - sqrt(max(p * p - q, 0.0)) * sign(p);
    return x0 - dx;
}
float newton_bezier(float a, float b, float c, float d, float e, float f, float x0) {
    x0 = clamp(x0, 0.0, 1.0);
    for (int i = 0; i < ITERATIONS; i++) {
        x0 = clamp(newton_quintic(a, b, c, d, e, f, x0), 0.0, 1.0);
    }
    return x0;
}

// Signed Distance Functions

float sdOrientedBox( const in vec2 p, const in vec2 a, const in vec2 b, const in float th )
{
    float l = length(b-a);
    vec2  d = (b-a)/l;
    vec2  q = (p-(a+b)*0.5);
          q = mat2(d.x,-d.y,d.y,d.x)*q;
          q = abs(q)-vec2(l,th)*0.5;
    return length(max(q,0.0)) + min(max(q.x,q.y),0.0);
}

float sdRoundedBox( in vec2 p, in vec2 b, in vec2 upper, in vec2 lower )
{
    vec4 r = vec4(upper, lower);
    r.xy = (p.x<0.0)?r.xz : r.yw;
    r.x  = (p.y>0.0)?r.x  : r.y;
    vec2 q = abs(p)-b+r.x;
    return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - r.x;
}

float sdCircle(const in vec2 p, const in float r )
{
    return length(p) - r;
}

// Exact ellipse SDF (Inigo Quilez). ab = semi-axes; falls back to sdCircle
// when they match to avoid the l=0 division in the general solve.
float sdEllipse( in vec2 p, in vec2 ab )
{
    if (abs(ab.x - ab.y) < 1e-4) return length(p) - ab.x;

    p = abs(p);
    if( p.x > p.y ){ p=p.yx; ab=ab.yx; }
    float l = ab.y*ab.y - ab.x*ab.x;
    float m = ab.x*p.x/l;      float m2 = m*m;
    float n = ab.y*p.y/l;      float n2 = n*n;
    float c = (m2+n2-1.0)/3.0; float c3 = c*c*c;
    float q = c3 + m2*n2*2.0;
    float d = c3 + m2*n2;
    float g = m + m*n2;
    float co;
    if( d<0.0 )
    {
        float h = acos(q/c3)/3.0;
        float s = cos(h);
        float t = sin(h)*sqrt(3.0);
        float rx = sqrt( -c*(s + t + 2.0) + m2 );
        float ry = sqrt( -c*(s - t + 2.0) + m2 );
        co = (ry+sign(l)*rx+abs(g)/(rx*ry)-m)/2.0;
    }
    else
    {
        float h = 2.0*m*n*sqrt( d );
        float s = sign(q+h)*pow(abs(q+h), 1.0/3.0);
        float u = sign(q-h)*pow(abs(q-h), 1.0/3.0);
        float rx = -s - u - c*4.0 + 2.0*m2;
        float ry = (s - u)*sqrt(3.0);
        float rm = sqrt( rx*rx + ry*ry );
        co = (ry/sqrt(rm-rx)+2.0*g/rm-m)/2.0;
    }
    vec2 r = ab * vec2(co, sqrt(1.0-co*co));
    return length(r-p) * sign(p.y-r.y);
}

float sdSegment(const in vec2 p, const in vec2 a, const in vec2 b, const in float th )
{
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h ) - th/2.0;
}

float sdPie(vec2 p, in float t, in float rot,  const in float r )
{
    // -0.5 full, 0.5 empty, mapping to aperture 0->1
    p = rotate(p, 360*t / 2);
    float o = ((1 - clamp(t, 0,1)) - 0.5) * _PI;
    vec2 c = vec2( cos(o), sin(o));

    p.x = abs(p.x);
    float l = length(p) - r;
    float m = length(p-c*clamp(dot(p,c),0.0,r)); // c=sin/cos of aperture
    return max(l,m*sign(c.y*p.x-c.x*p.y));
}

// Ring
// p  : pixel
// n  : normal
// r  : radius
// th : thickness
float sdRing( in vec2 p, in vec2 n, in float r, float th )
{
    p.x = abs(p.x);
    p = mat2x2(n.x,n.y,-n.y,n.x)*p;
    return max( abs(length(p)-r)-th*0.5,
                length(vec2(p.x,max(0.0,abs(r-p.y)-th*0.5)))*sign(p.x) );
}

float sdParamRing( in vec2 p, in float t, in float rot, in vec2 size )
{
    p = rotate(p, (360*t / 2));
    return sdRing(p, vec2(cos(t*_PI), sin(t*_PI)),  size.r, size.g );
}

float sdArc( vec2 p, float t, float rot,  in vec2 r )
{
    p = rotate(p, (360*t / 2));
    float o = ((1 - clamp(t, 0,1)) - 0.5) * _PI;
    vec2 sc = vec2( cos(o), sin(o));

    p.x = abs(p.x);
    return ((sc.y*p.x>sc.x*p.y) ? length(p-sc*r.x) :
                                  abs(length(p)-r.x)) - r.y;
}

float sdRoundedX( in vec2 p, in float w, in float r )
{
    p = abs(p);
    return length(p-min(p.x+p.y,w)*0.5) - r;
}

// Ring
// p  : pixel
// b  : (branch length, branche thickness)
// r  : rounding factor
float sdCross( in vec2 p, in vec2 b, float r )
{
    p = abs(p); p = (p.y>p.x) ? p.yx : p.xy;
    vec2  q = p - b;
    float k = max(q.y,q.x);
    vec2  w = (k>0.0) ? q : vec2(b.y-p.x,-k);
    return sign(k)*length(max(w,0.0)) + r;
}

float sdPentagon( in vec2 p, in float r )
{
    const vec3 k = vec3(0.809016994,0.587785252,0.726542528);
    p.x = abs(p.x);
    p -= 2.0*min(dot(vec2(-k.x,k.y),p),0.0)*vec2(-k.x,k.y);
    p -= 2.0*min(dot(vec2( k.x,k.y),p),0.0)*vec2( k.x,k.y);
    p -= vec2(clamp(p.x,-r*k.z,r*k.z),r);
    return length(p)*sign(p.y);
}

float sdHexagon( in vec2 p, in float r )
{
    const vec3 k = vec3(-0.866025404,0.5,0.577350269);
    p = abs(p);
    p -= 2.0*min(dot(k.xy,p),0.0)*k.xy;
    p -= vec2(clamp(p.x, -k.z*r, k.z*r), r);
    return length(p)*sign(p.y);
}

// Triangle
// p  : pixel
// p0, p1, p2 : Position triangle corners
float sdTriangle( in vec2 p, in vec2 p0, in vec2 p1, in vec2 p2 )
{
    vec2 e0 = p1-p0, e1 = p2-p1, e2 = p0-p2;
    vec2 v0 = p -p0, v1 = p -p1, v2 = p -p2;
    vec2 pq0 = v0 - e0*clamp( dot(v0,e0)/dot(e0,e0), 0.0, 1.0 );
    vec2 pq1 = v1 - e1*clamp( dot(v1,e1)/dot(e1,e1), 0.0, 1.0 );
    vec2 pq2 = v2 - e2*clamp( dot(v2,e2)/dot(e2,e2), 0.0, 1.0 );
    float s = sign( e0.x*e2.y - e0.y*e2.x );
    vec2 d = min(min(vec2(dot(pq0,pq0), s*(v0.x*e0.y-v0.y*e0.x)),
                     vec2(dot(pq1,pq1), s*(v1.x*e1.y-v1.y*e1.x))),
                     vec2(dot(pq2,pq2), s*(v2.x*e2.y-v2.y*e2.x)));
    return -sqrt(d.x)*sign(d.y);
}

float sdBezierCubic(in vec2 p, inout float t, UIPrimitive e)
{
    // test cubic roots
    vec2 a = e.pos4 - e.pos + 3.0 * (e.pos2 - e.pos3);
    vec2 b = 3.0 * e.pos - 6.0 * e.pos2 + 3.0 * e.pos3;
    vec2 c = -3.0 * e.pos + 3.0 * e.pos2;
    vec2 d = e.pos - p;
    float qa = 3.0 * dot(a, a), qb = 5.0 * dot(a, b), qc = 2.0 * dot(b, b) + 4.0 * dot(a, c), qd = 3.0 * dot(c, b) + 3.0 * dot(a, d), qe = 2.0 * dot(b, d) + dot(c, c), qf = dot(c, d);
    vec2 x0, x1, x2;
    cubic_roots(a, b, c, d, x0, x1, x2);
    // limit to range 0-1
    vec2 x = vec2(0.0, length(e.pos - p));
    x0.x = newton_bezier(qa, qb, qc, qd, qe, qf, x0.x);
    x0.y = length(cubic(a, b, c, d, vec2(x0.x, 0.0)));
    x = selectx(x, x0);
    x1.x = newton_bezier(qa, qb, qc, qd, qe, qf, x1.x);
    x1.y = length(cubic(a, b, c, d, vec2(x1.x, 0.0)));
    x = selectx(x, x1);
    x2.x = newton_bezier(qa, qb, qc, qd, qe, qf, x2.x);
    x2.y = length(cubic(a, b, c, d, vec2(x2.x, 0.0)));
    x = selectx(x, x2);
    float dp3 = length(e.pos4 - p);
    if (x.y > dp3) {
        x.x = 1.0;
        x.y = dp3;
    }

    t = x.x;
    vec2 pos = cubic(a, b, c, d, vec2(t, 0.0));
    vec2 tangent = quadratic(3.0 * a, 2.0 * b, c, vec2(t, 0.0));
    float dist = length(pos);
    float sgn0 = sign(cmul(conj(tangent), pos).y);
    vec2 pSign = -inverse(mat2(a, b)) * c;
    float q = p.y*p.y - p.x;
    float sgn = sgn0 * ((q > 0.0) && (q-pSign.y+1.0 > 0.0) ? sign(t*t - p.y * t + q) : 1.0);
    float d0 = abs(dist);

    return d0 - e.size.x/2;
}

// Blend modes
float intersect(const in float shape1, const in float shape2){
    return max(shape1, shape2);
}

float subtract(const in float base, const  in float subtraction){
    return intersect(base, -subtraction);
}

float add(in float base, in float shape){
    return min(base, shape);
}

float composeSdf(in int blend, const in float d, const in float shape)
{
    if((blend & SUBTRACT) == SUBTRACT)
        return subtract(d, shape);

    if((blend & INTERSECTION) == INTERSECTION)
        return intersect(d, shape);

    return  add(d, shape);
}

vec4 over(vec4 dst, vec4 src) {
    float outA = src.a + dst.a * (1.0 - src.a);
    vec3 outRGB = (src.rgb * src.a + dst.rgb * dst.a * (1.0 - src.a)) / max(outA, 1e-5);
    return vec4(outRGB, outA);
}

float borderCoverage(float dist, float thickness)
{
    float halfT = thickness * 0.5;
    float w = max(0, fwidth(dist));               // AA width
    float inner = smoothstep(-halfT - w, -halfT + w, dist);
    float outer = smoothstep(halfT - w, halfT + w, dist);

    return -clamp((inner - outer), 0.0, 1.0) +1 ;
}

// gradient texture (256 wide x N gradients tall, stored as a 2D array with 1 layer)
uniform sampler2DArray uGradientTexture;
// The active gradient ID (0-255), selects the row in the texture
uniform int uGradientID;

void main()
{
    float u_blur = 2.0;

    vec2 p = v_LocalXY;

    // Background color
    vec4 col = vec4(0.0);

    int loop = 0;

    float d  = 1;
    float t = 0;

    for(int i = 0; i < u_PrimSize; i++)
    {
        UIPrimitive e = primitives[i];

        if((e.blendMode & GROUP) == GROUP)
            loop ^= 1; //switch group mode

        p = (e.rotation > 0.0) ? rotate(p, e.rotation): p;

        if(e.shapeId == SDCIRCLE      ){d = composeSdf(e.blendMode, d, sdCircle(p- e.pos, e.size.r));}
        if(e.shapeId == SDORIENTEDBOX ){d = composeSdf(e.blendMode, d, sdOrientedBox  (p, e.pos, e.pos2, e.size.r));}
        if(e.shapeId == SDROUNDEDDBOX ){d = composeSdf(e.blendMode, d, sdRoundedBox  (p- e.pos, e.size, e.pos2, e.pos3));}
        if(e.shapeId == SDSEGMENT     ){d = composeSdf(e.blendMode, d, sdSegment      (p, e.pos, e.pos2, e.size.r));}
        if(e.shapeId == SDPIE         ){d = composeSdf(e.blendMode, d, sdPie(p, e.progress, e.rotation, e.size.r));}
        if(e.shapeId == SDRING        ){d = composeSdf(e.blendMode, d, sdParamRing(p, e.progress, e.rotation, e.size));}
        if(e.shapeId == SDARC         ){d = composeSdf(e.blendMode, d, sdArc(p, u_Progress, e.rotation, e.size));}
        if(e.shapeId == SDTRIANGLE    ){d = composeSdf(e.blendMode, d, sdTriangle(p, e.pos, e.pos2, e.pos3));}
        if(e.shapeId == SDROUNEDX     ){d = composeSdf(e.blendMode, d, sdRoundedX(p, e.size.r, e.cornerRadius));}
        if(e.shapeId == SDCROSS       ){d = composeSdf(e.blendMode, d, sdCross(p, e.size, e.cornerRadius));}
        if(e.shapeId == SDPENTAGON    ){d = composeSdf(e.blendMode, d,sdPentagon(p, e.size.r));}
        if(e.shapeId == SDHEXAGON     ){d = composeSdf(e.blendMode, d,sdHexagon(p, e.size.r));}
        if(e.shapeId == SDBEZIER_CUBIC){d = composeSdf(e.blendMode, d,sdBezierCubic(p, t, e));}
        if(e.shapeId == SDELLIPSE     ){d = composeSdf(e.blendMode, d, sdEllipse(p- e.pos, e.size));}

        if(loop!=0) continue;

        d = (e.cornerRadius > 0) ?  d - e.cornerRadius : d;
        d = (e.innerRadius> 0) ? d = abs(d) - e.innerRadius : d;

        // Anti Aliasing
        float w = u_blur*fwidth(d);
        float h = smoothstep(-w/2.0, w/2.0, -d); // smooth centered blur AA

        // Alpha blending
        float alpha = e.color.a * h;

        vec4 src = vec4(e.color.xyz, alpha);

        if((e.blendColor & GRADIENT) == GRADIENT)
        {
            src = vec4(mix(e.color.xyz, e.color2.xyz, t), alpha);
        }

        if((e.blendColor & GRAYSCALE) == GRAYSCALE)
        {
            float Clinear = 0.2126 * src.x + 0.7152 * src.y + 0.0722 * src.z;
            Clinear = (Clinear < 0.0031308) ? Clinear * 12.92 : pow(Clinear,1.0/2.4) * 1.055 - 0.055;
            src =  vec4(Clinear,Clinear,Clinear, alpha);
        }

        if((e.blendColor & COLORMAP) == COLORMAP)
        {
            float u = src.r;
            vec4 gradientColor = texture(uGradientTexture, vec3(u, float(uGradientID/255.0), 0));
            src = vec4(gradientColor.xyz, alpha);
        }

        col = over(col, src);

        // Border with AA
        if(e.borderWidth > 0) {
            d = borderCoverage(d, e.borderWidth);
            col = mix( vec4(e.border.xyz, 1.0), col, d);
        }

        d  = 1;
    }

    if (u_SDFMode == 2) {
        vec2 uv = v_UV;
        if (u_UVMode == 1) {
            vec2 c = uv - vec2(0.5);
            float angle = atan(c.y, c.x) / (2.0 * _PI) + 0.5 + u_UVOffset.x;
            float radius = length(c) * 2.0 + u_UVOffset.y;
            uv = vec2(angle, radius);
        } else {
            uv += u_UVOffset;
        }
        vec4 texColor = texture(u_Texture, vec3(uv, float(u_TexOffset)));
        FragColor = vec4(texColor.rgb, texColor.a * col.a * u_Alpha);
    } else {
        FragColor = vec4(col.rgb, col.a * u_Alpha);
    }
}
)";
}

static void _lineShadersData(std::string& vertex, std::string& fragment)
{
    vertex = R"(
#version 440 core

//Coordinates and colors data input
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec4 model_colors;

//Color output for the fragment shader
out vec4 fragmentColor;

// Projection matrix
uniform mat4 u_MVP;


void main(){
    //Transform the vertex position using the ortho projection matrix
    gl_Position =  u_MVP * vec4(vertexPosition_modelspace, 1);

    //Color output for the fragment shader
    fragmentColor = model_colors;
}
)";

    fragment = R"(
#version 440 core

//Color input from the vertex shader
in vec4 fragmentColor;

//Color output using vertices data
out vec4 l_Color;


void main(){
  l_Color = fragmentColor;
}
)";
}

static void _uiShapeShadersData(std::string& vertex, std::string& fragment)
{
    vertex = R"(
#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 uProj;

out vec2 vWorldPos;

uniform vec2 uSize;
uniform vec3 uPos;

void main() {
    vec3 wPos = uPos + aPos * vec3(uSize, 1.0);
    gl_Position = uProj * vec4(wPos, 1.0);

    vWorldPos = wPos.xy;
}
)";

    fragment = R"(
#version 430 core

#define _PI 3.14159265359
uniform int     uPrimSize;
uniform vec2    uFrameRes;
uniform float   uProgress;

in vec2 vWorldPos;

#define SDCIRCLE        0
#define SDORIENTEDBOX   1
#define SDROUNDEDDBOX   2
#define SDSEGMENT       3
#define SDPIE           4
#define SDRING          5
#define SDARC           6
#define SDTRIANGLE      7
#define SDROUNEDX       8
#define SDCROSS         9
#define SDPENTAGON      10
#define SDHEXAGON       11
#define SDBEZIER_CUBIC  12
#define SDELLIPSE       13

#define GROUP           1
#define SUBTRACT        2
#define INTERSECTION    4

#define GRAYSCALE       1
#define GRADIENT        2
#define COLORMAP        4

out vec4 FragColor;

struct UIPrimitive {
    vec2 pos;
    vec2 size;
    vec4 color;
    vec4 color2;
    vec4 border;
    float borderWidth;
    float cornerRadius;
    int   shapeId;
    int   blendMode;
    float innerRadius;
    float progress;
    vec2   pos2;
    vec2   pos3;
    vec2   pos4;
    float rotation;
    float scale;
    int   blendColor;
    int   gradientID;
};

layout(binding = 0, std430) readonly buffer Primitives {
    UIPrimitive primitives[];
};

void transformPixelSpace(inout UIPrimitive prim)
{
    float rmin  = min(uFrameRes.x, uFrameRes.y);
    vec2 factor = uFrameRes / rmin;
    prim.pos            /= uFrameRes   * 0.5 / factor;
    prim.pos2           /= uFrameRes   * 0.5 / factor;
    prim.pos3           /= uFrameRes   * 0.5 / factor;
    prim.pos4           /= uFrameRes   * 0.5 / factor;
    prim.size           /= uFrameRes   * 0.5 / factor;
    prim.borderWidth    /= uFrameRes.x * 0.5;
    prim.cornerRadius   /= uFrameRes.x * 0.5;
    prim.innerRadius    /= uFrameRes.x * 0.5;
}

vec2 rotate(const in vec2 samplePosition, const in float rotation){
    float angle   = rotation * _PI * -1.0/180.0;
    float sine    = sin(angle);
    float cosine  = cos(angle);
    return vec2(cosine * samplePosition.x + sine * samplePosition.y, cosine * samplePosition.y - sine * samplePosition.x);
}

#define ITERATIONS 2
vec2 cexp(vec2 c) { return exp(c.x)*vec2(cos(c.y), sin(c.y)); }
vec2 cln(vec2 c)  { return vec2(log(dot(c,c))*.5, atan(c.y, c.x)); }
vec2 cmul(vec2 a, vec2 b) { return vec2(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x); }
vec2 conj(vec2 c) { return vec2(c.x, -c.y); }
vec2 cdiv(vec2 a, vec2 b) { return cmul(a, conj(b)) / dot(b, b); }
vec2 csqrt(vec2 a) {
    float r = length(a);
    if ((a.y + a.x) - a.x == 0.0) return a.x >= 0.0 ? vec2(sqrt(r), 0.0) : vec2(0.0, sqrt(r));
    vec2 h = a / r + vec2(1.0, 0.0);
    return h * sqrt(r / dot(h, h));
}
vec2 ccbrt(vec2 a) { return cexp(cln(a)/3.0); }
void cubic_roots(vec2 a, vec2 b, vec2 c, vec2 d, out vec2 x0, out vec2 x1, out vec2 x2) {
    vec2 ac = cmul(a,c), bb = cmul(b,b), aa = cmul(a,a);
    vec2 d0 = bb - 3.0*ac;
    vec2 d1 = 2.0*cmul(b,bb) - 9.0*cmul(ac,b) + 27.0*cmul(aa,d);
    vec2 s  = csqrt(cmul(d1,d1) - 4.0*cmul(cmul(d0,d0),d0));
    vec2 opta = d1 - s, optb = d1 + s;
    vec2 opt = dot(opta,opta) < dot(optb,optb) ? optb : opta;
    vec2 cb = ccbrt(opt * 0.5);
    x0 = cdiv(b + cb + cdiv(d0, cb), -3.0*a);
    vec2 root = vec2(-0.5, 0.866025403784439);
    cb = cmul(cb, root); x1 = cdiv(b + cb + cdiv(d0, cb), -3.0*a);
    cb = cmul(cb, root); x2 = cdiv(b + cb + cdiv(d0, cb), -3.0*a);
}
vec2 cubic(vec2 a, vec2 b, vec2 c, vec2 d, vec2 x)     { return cmul(cmul(cmul(a,x)+b,x)+c,x)+d; }
vec2 quadratic(vec2 a, vec2 b, vec2 c, vec2 x)         { return cmul(cmul(a,x)+b,x)+c; }
vec2 selectx(vec2 x0, vec2 x1)                         { return abs(x0.y) < abs(x1.y) ? x0 : x1; }
float newton_quintic(float a, float b, float c, float d, float e, float f, float x0) {
    float v  = ((((a*x0+b)*x0+c)*x0+d)*x0+e)*x0+f;
    float dv = (((5.0*a*x0+4.0*b)*x0+3.0*c)*x0+2.0*d)*x0+e;
    float ddv= ((20.0*a*x0+12.0*b)*x0+6.0*c)*x0+2.0*d;
    float p  = dv/ddv;
    float q  = v/ddv*2.0;
    float dx = p - sqrt(max(p*p - q, 0.0))*sign(p);
    return x0 - dx;
}
float newton_bezier(float a, float b, float c, float d, float e, float f, float x0) {
    x0 = clamp(x0, 0.0, 1.0);
    for (int i = 0; i < ITERATIONS; i++) x0 = clamp(newton_quintic(a,b,c,d,e,f,x0), 0.0, 1.0);
    return x0;
}

float sdOrientedBox(const in vec2 p, const in vec2 a, const in vec2 b, const in float th) {
    float l = length(b-a); vec2 d = (b-a)/l;
    vec2 q = (p-(a+b)*0.5); q = mat2(d.x,-d.y,d.y,d.x)*q; q = abs(q)-vec2(l,th)*0.5;
    return length(max(q,0.0)) + min(max(q.x,q.y),0.0);
}
float sdRoundedBox(in vec2 p, in vec2 b, in vec2 upper, in vec2 lower) {
    vec4 r = vec4(upper, lower);
    r.xy = (p.x<0.0) ? r.xz : r.yw; r.x = (p.y>0.0) ? r.x : r.y;
    vec2 q = abs(p)-b+r.x;
    return min(max(q.x,q.y),0.0) + length(max(q,0.0)) - r.x;
}
float sdCircle(const in vec2 p, const in float r) { return length(p) - r; }
float sdEllipse(in vec2 p, in vec2 ab) {
    if (abs(ab.x - ab.y) < 1e-4) return length(p) - ab.x;
    p = abs(p); if (p.x > p.y) { p=p.yx; ab=ab.yx; }
    float l = ab.y*ab.y - ab.x*ab.x;
    float m = ab.x*p.x/l, m2 = m*m, n = ab.y*p.y/l, n2 = n*n;
    float c = (m2+n2-1.0)/3.0, c3 = c*c*c, q = c3 + m2*n2*2.0, d = c3 + m2*n2, g = m + m*n2;
    float co;
    if (d < 0.0) {
        float h = acos(q/c3)/3.0, s = cos(h), t = sin(h)*sqrt(3.0);
        float rx = sqrt(-c*(s+t+2.0)+m2), ry = sqrt(-c*(s-t+2.0)+m2);
        co = (ry+sign(l)*rx+abs(g)/(rx*ry)-m)/2.0;
    } else {
        float h = 2.0*m*n*sqrt(d);
        float s = sign(q+h)*pow(abs(q+h),1.0/3.0), u = sign(q-h)*pow(abs(q-h),1.0/3.0);
        float rx = -s-u-c*4.0+2.0*m2, ry = (s-u)*sqrt(3.0), rm = sqrt(rx*rx+ry*ry);
        co = (ry/sqrt(rm-rx)+2.0*g/rm-m)/2.0;
    }
    vec2 r = ab * vec2(co, sqrt(1.0-co*co));
    return length(r-p) * sign(p.y-r.y);
}
float sdSegment(const in vec2 p, const in vec2 a, const in vec2 b, const in float th) {
    vec2 pa = p-a, ba = b-a;
    return length(pa - ba*clamp(dot(pa,ba)/dot(ba,ba), 0.0, 1.0)) - th/2.0;
}
float sdPie(vec2 p, in float t, in float rot, const in float r) {
    p = rotate(p, 360.0*t/2.0);
    float o = ((1.0 - clamp(t,0.0,1.0)) - 0.5)*_PI;
    vec2 c = vec2(cos(o), sin(o)); p.x = abs(p.x);
    return max(length(p)-r, length(p-c*clamp(dot(p,c),0.0,r))*sign(c.y*p.x-c.x*p.y));
}
float sdRing(in vec2 p, in vec2 n, in float r, float th) {
    p.x = abs(p.x); p = mat2x2(n.x,n.y,-n.y,n.x)*p;
    return max(abs(length(p)-r)-th*0.5, length(vec2(p.x,max(0.0,abs(r-p.y)-th*0.5)))*sign(p.x));
}
float sdParamRing(in vec2 p, in float t, in float rot, in vec2 size) {
    p = rotate(p, 360.0*t/2.0);
    return sdRing(p, vec2(cos(t*_PI), sin(t*_PI)), size.r, size.g);
}
float sdArc(vec2 p, float t, float rot, in vec2 r) {
    p = rotate(p, 360.0*t/2.0);
    float o = ((1.0 - clamp(t,0.0,1.0)) - 0.5)*_PI;
    vec2 sc = vec2(cos(o), sin(o)); p.x = abs(p.x);
    return ((sc.y*p.x>sc.x*p.y) ? length(p-sc*r.x) : abs(length(p)-r.x)) - r.y;
}
float sdRoundedX(in vec2 p, in float w, in float r) {
    p = abs(p); return length(p - min(p.x+p.y,w)*0.5) - r;
}
float sdCross(in vec2 p, in vec2 b, float r) {
    p = abs(p); p = (p.y>p.x) ? p.yx : p.xy;
    vec2 q = p - b; float k = max(q.y,q.x);
    vec2 w = (k>0.0) ? q : vec2(b.y-p.x,-k);
    return sign(k)*length(max(w,0.0)) + r;
}
float sdPentagon(in vec2 p, in float r) {
    const vec3 k = vec3(0.809016994,0.587785252,0.726542528);
    p.x = abs(p.x);
    p -= 2.0*min(dot(vec2(-k.x,k.y),p),0.0)*vec2(-k.x,k.y);
    p -= 2.0*min(dot(vec2( k.x,k.y),p),0.0)*vec2( k.x,k.y);
    p -= vec2(clamp(p.x,-r*k.z,r*k.z),r);
    return length(p)*sign(p.y);
}
float sdHexagon(in vec2 p, in float r) {
    const vec3 k = vec3(-0.866025404,0.5,0.577350269);
    p = abs(p); p -= 2.0*min(dot(k.xy,p),0.0)*k.xy;
    p -= vec2(clamp(p.x,-k.z*r,k.z*r),r);
    return length(p)*sign(p.y);
}
float sdTriangle(in vec2 p, in vec2 p0, in vec2 p1, in vec2 p2) {
    vec2 e0=p1-p0,e1=p2-p1,e2=p0-p2,v0=p-p0,v1=p-p1,v2=p-p2;
    vec2 pq0=v0-e0*clamp(dot(v0,e0)/dot(e0,e0),0.0,1.0);
    vec2 pq1=v1-e1*clamp(dot(v1,e1)/dot(e1,e1),0.0,1.0);
    vec2 pq2=v2-e2*clamp(dot(v2,e2)/dot(e2,e2),0.0,1.0);
    float s = sign(e0.x*e2.y-e0.y*e2.x);
    vec2 d = min(min(vec2(dot(pq0,pq0),s*(v0.x*e0.y-v0.y*e0.x)),
                     vec2(dot(pq1,pq1),s*(v1.x*e1.y-v1.y*e1.x))),
                     vec2(dot(pq2,pq2),s*(v2.x*e2.y-v2.y*e2.x)));
    return -sqrt(d.x)*sign(d.y);
}
float sdBezierCubic(in vec2 p, inout float t, UIPrimitive e) {
    vec2 a = e.pos4 - e.pos + 3.0*(e.pos2 - e.pos3);
    vec2 b = 3.0*e.pos - 6.0*e.pos2 + 3.0*e.pos3;
    vec2 c = -3.0*e.pos + 3.0*e.pos2;
    vec2 d = e.pos - p;
    float qa=3.0*dot(a,a), qb=5.0*dot(a,b), qc=2.0*dot(b,b)+4.0*dot(a,c);
    float qd=3.0*dot(c,b)+3.0*dot(a,d), qe=2.0*dot(b,d)+dot(c,c), qf=dot(c,d);
    vec2 x0, x1, x2;
    cubic_roots(a,b,c,d,x0,x1,x2);
    vec2 x = vec2(0.0, length(e.pos - p));
    x0.x = newton_bezier(qa,qb,qc,qd,qe,qf,x0.x); x0.y = length(cubic(a,b,c,d,vec2(x0.x,0.0))); x = selectx(x,x0);
    x1.x = newton_bezier(qa,qb,qc,qd,qe,qf,x1.x); x1.y = length(cubic(a,b,c,d,vec2(x1.x,0.0))); x = selectx(x,x1);
    x2.x = newton_bezier(qa,qb,qc,qd,qe,qf,x2.x); x2.y = length(cubic(a,b,c,d,vec2(x2.x,0.0))); x = selectx(x,x2);
    float dp3 = length(e.pos4 - p);
    if (x.y > dp3) { x.x = 1.0; x.y = dp3; }
    t = x.x;
    vec2 pos = cubic(a,b,c,d,vec2(t,0.0));
    vec2 tangent = quadratic(3.0*a,2.0*b,c,vec2(t,0.0));
    float dist = length(pos);
    float sgn0 = sign(cmul(conj(tangent),pos).y);
    vec2 pSign = -inverse(mat2(a,b))*c;
    float q2 = p.y*p.y - p.x;
    float sgn = sgn0 * ((q2>0.0) && (q2-pSign.y+1.0>0.0) ? sign(t*t - p.y*t + q2) : 1.0);
    return abs(dist) - e.size.x/2.0;
}

float intersect(const in float s1, const in float s2) { return max(s1, s2); }
float subtract(const in float base, const in float sub) { return intersect(base, -sub); }
float add(in float base, in float shape) { return min(base, shape); }
float composeSdf(in int blend, const in float d, const in float shape) {
    if ((blend & SUBTRACT)     == SUBTRACT)     return subtract(d, shape);
    if ((blend & INTERSECTION) == INTERSECTION) return intersect(d, shape);
    return add(d, shape);
}

vec4 over(vec4 dst, vec4 src) {
    float outA = src.a + dst.a*(1.0 - src.a);
    vec3  outRGB = (src.rgb*src.a + dst.rgb*dst.a*(1.0 - src.a)) / max(outA, 1e-5);
    return vec4(outRGB, outA);
}
float borderCoverage(float dist, float thickness) {
    float halfT = thickness*0.5, w = max(0.0, fwidth(dist));
    return -clamp((smoothstep(-halfT-w,-halfT+w,dist) - smoothstep(halfT-w,halfT+w,dist)), 0.0, 1.0) + 1.0;
}

uniform sampler2DArray uGradientTexture;
uniform int uGradientID;

void main()
{
    float rmin = min(uFrameRes.x, uFrameRes.y);
    float u_blur = 2.0;
    vec2 Position = 2.0 * vWorldPos / rmin;
    vec2 p = Position;
    vec4 col = vec4(0.0);
    int loop = 0;
    float d = 1.0, t = 0.0;

    for (int i = 0; i < uPrimSize; i++) {
        UIPrimitive e = primitives[i];
        transformPixelSpace(e);
        if ((e.blendMode & GROUP) == GROUP) loop ^= 1;
        p = (e.rotation > 0.0) ? rotate(p, e.rotation) : p;
        if (e.shapeId == SDCIRCLE      ) d = composeSdf(e.blendMode, d, sdCircle(p-e.pos, e.size.r));
        if (e.shapeId == SDORIENTEDBOX ) d = composeSdf(e.blendMode, d, sdOrientedBox(p, e.pos, e.pos2, e.size.r));
        if (e.shapeId == SDROUNDEDDBOX ) d = composeSdf(e.blendMode, d, sdRoundedBox(p-e.pos, e.size, e.pos2, e.pos3));
        if (e.shapeId == SDSEGMENT     ) d = composeSdf(e.blendMode, d, sdSegment(p, e.pos, e.pos2, e.size.r));
        if (e.shapeId == SDPIE         ) d = composeSdf(e.blendMode, d, sdPie(p, e.progress, e.rotation, e.size.r));
        if (e.shapeId == SDRING        ) d = composeSdf(e.blendMode, d, sdParamRing(p, e.progress, e.rotation, e.size));
        if (e.shapeId == SDARC         ) d = composeSdf(e.blendMode, d, sdArc(p, uProgress, e.rotation, e.size));
        if (e.shapeId == SDTRIANGLE    ) d = composeSdf(e.blendMode, d, sdTriangle(p, e.pos, e.pos2, e.pos3));
        if (e.shapeId == SDROUNEDX     ) d = composeSdf(e.blendMode, d, sdRoundedX(p, e.size.r, e.cornerRadius));
        if (e.shapeId == SDCROSS       ) d = composeSdf(e.blendMode, d, sdCross(p, e.size, e.cornerRadius));
        if (e.shapeId == SDPENTAGON    ) d = composeSdf(e.blendMode, d, sdPentagon(p, e.size.r));
        if (e.shapeId == SDHEXAGON     ) d = composeSdf(e.blendMode, d, sdHexagon(p, e.size.r));
        if (e.shapeId == SDBEZIER_CUBIC) d = composeSdf(e.blendMode, d, sdBezierCubic(p, t, e));
        if (e.shapeId == SDELLIPSE     ) d = composeSdf(e.blendMode, d, sdEllipse(p-e.pos, e.size));
        if (loop != 0) continue;
        d = (e.cornerRadius > 0.0) ? d - e.cornerRadius : d;
        d = (e.innerRadius  > 0.0) ? abs(d) - e.innerRadius : d;
        float w = u_blur * fwidth(d);
        float h = smoothstep(-w/2.0, w/2.0, -d);
        float alpha = e.color.a * h;
        vec4 src = vec4(e.color.xyz, alpha);
        if ((e.blendColor & GRADIENT)  == GRADIENT)  src = vec4(mix(e.color.xyz, e.color2.xyz, t), alpha);
        if ((e.blendColor & GRAYSCALE) == GRAYSCALE) {
            float L = 0.2126*src.x + 0.7152*src.y + 0.0722*src.z;
            L = (L < 0.0031308) ? L*12.92 : pow(L,1.0/2.4)*1.055 - 0.055;
            src = vec4(L, L, L, alpha);
        }
        if ((e.blendColor & COLORMAP) == COLORMAP) {
            vec4 gradientColor = texture(uGradientTexture, vec3(src.r, float(uGradientID)/255.0, 0));
            src = vec4(gradientColor.xyz, alpha);
        }
        col = over(col, src);
        if (e.borderWidth > 0.0) {
            d = borderCoverage(d, e.borderWidth);
            col = mix(vec4(e.border.xyz, 1.0), col, d);
        }
        d = 1.0;
    }
    FragColor = col;
}
)";
}

void Window::_loadPresetShaders() try
{
    std::string vertex_data, fragment_data;

    // Plane shader
    {
        uint32_t const id = static_cast<uint32_t>(Shaders::Preset::Plane);
        // Retrieve shader pointer
        auto& shader = _main._preset_shaders[id];
        // Allocate new shader
        shader = Shaders::create();
        // Retrieve shader data
        _planeShadersData(vertex_data, fragment_data);
        // Load shader
        shader->loadFromStrings(vertex_data, fragment_data);
    }

    // Line shader
    {
        uint32_t const id = static_cast<uint32_t>(Shaders::Preset::Line);
        // Retrieve shader pointer
        auto& shader = _main._preset_shaders[id];
        // Allocate new shader
        shader = Shaders::create();
        // Retrieve shader data
        _lineShadersData(vertex_data, fragment_data);
        // Load shader
        shader->loadFromStrings(vertex_data, fragment_data);
    }

    // Plane SDF shader
    {
        uint32_t const id = static_cast<uint32_t>(Shaders::Preset::PlaneSDF);
        // Retrieve shader pointer
        auto& shader = _main._preset_shaders[id];
        // Allocate new shader
        shader = Shaders::create();
        // Retrieve shader data
        _planeSDFShadersData(vertex_data, fragment_data);
        // Load shader
        shader->loadFromStrings(vertex_data, fragment_data);
    }

    // UI SDF shape shader
    {
        uint32_t const id = static_cast<uint32_t>(Shaders::Preset::UIShape);
        auto& shader = _main._preset_shaders[id];
        shader = Shaders::create();
        _uiShapeShadersData(vertex_data, fragment_data);
        shader->loadFromStrings(vertex_data, fragment_data);
    }

}
CATCH_AND_RETHROW_FUNC_EXC;

SSS_GL_END;