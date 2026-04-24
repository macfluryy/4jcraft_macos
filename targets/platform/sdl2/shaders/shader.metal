R"MSL(
#include <metal_stdlib>
using namespace metal;

struct Uniforms {
    float4x4 uMVP;
    float4x4 uMV;
    float4x4 uTexMat0;
    float4   uNormalRow0;
    float4   uNormalRow1;
    float4   uNormalRow2;

    float4   uBaseColor;
    float4   uFogColor;
    float4   uLMTransform;

    float4   uLight0Dir;
    float4   uLight1Dir;
    float4   uLightDiffuse;
    float4   uLightAmbient;
    float4   uChunkOffset;
    float4   uScalars;
    float4   uScalars2;
    int4     uFlags;
    int4     uFlags2;
};

struct VertexIn {
    float3   position  [[attribute(0)]];
    float2   uv0       [[attribute(1)]];
    uchar4   color     [[attribute(2)]];
    char3    normal    [[attribute(3)]];
    short2   lmRaw     [[attribute(4)]];
};

struct VertexOut {
    float4 position [[position]];
    float2 vUV0;
    float2 vUV1;
    float4 vColor;
    float  vFogFactor;
};

vertex VertexOut vertexMain(VertexIn in [[stage_in]],
                            constant Uniforms& u [[buffer(1)]]) {
    VertexOut out;

    // uScalars = (uFogStart, uFogEnd, uFogDensity, uAlphaRef)
    // uScalars2 = (uInvGamma, uNormalSign, 0, 0)
    // uFlags = (uLighting, uFogMode, uFogEnable, uUseTexture)
    // uFlags2 = (uUseLightmap, uSrgbOutput, 0, 0)
    // uChunkOffset.xyz holds offset
    // uLight*Dir.xyz are directions
    // uNormalRow0/1/2 are rows of 3x3 matrix (w ignored)

    float4 pos4 = float4(in.position + u.uChunkOffset.xyz, 1.0);
    float4 eyePos = u.uMV * pos4;
    out.position  = u.uMVP * pos4;

    out.vUV0 = (u.uTexMat0 * float4(in.uv0, 0.0, 1.0)).xy;

    float2 lm = (in.lmRaw.x <= -500) ? u.uScalars2.zw : float2(in.lmRaw);
    out.vUV1 = (lm / 256.0) * u.uLMTransform.xy + u.uLMTransform.zw;

    float4 vertColor = float4(in.color) / 255.0;
    bool sentinel = all(vertColor == float4(0.0));
    float4 col = sentinel ? u.uBaseColor : vertColor.abgr;

    if (u.uFlags.x == 1) {
        float3 nIn = float3(in.normal) / 127.0;
        float3 n = float3(dot(u.uNormalRow0.xyz, nIn),
                          dot(u.uNormalRow1.xyz, nIn),
                          dot(u.uNormalRow2.xyz, nIn));
        n = normalize(n) * u.uScalars2.y;
        float d0 = max(dot(n, u.uLight0Dir.xyz), 0.0);
        float d1 = max(dot(n, u.uLight1Dir.xyz), 0.0);
        out.vColor = float4(col.rgb * (u.uLightAmbient.xyz + u.uLightDiffuse.xyz * clamp(d0 + d1, 0.0, 1.0)), col.a);
    } else {
        out.vColor = col;
    }

    float eDist = length(eyePos.xyz);
    int fogMode = u.uFlags.y;
    if      (fogMode == 1) out.vFogFactor = clamp((u.uScalars.y - eDist) / max(u.uScalars.y - u.uScalars.x, 1e-4), 0.0, 1.0);
    else if (fogMode == 2) out.vFogFactor = clamp(exp(-u.uScalars.z * eDist), 0.0, 1.0);
    else if (fogMode == 3) { float d = u.uScalars.z * eDist; out.vFogFactor = clamp(exp(-d*d), 0.0, 1.0); }
    else                   out.vFogFactor = 1.0;

    return out;
}

fragment float4 fragmentMain(VertexOut in [[stage_in]],
                             constant Uniforms& u [[buffer(1)]],
                             texture2d<float> tex0 [[texture(0)]],
                             texture2d<float> tex1 [[texture(1)]],
                             sampler samp0 [[sampler(0)]],
                             sampler samp1 [[sampler(1)]]) {
    float4 texColor = (u.uFlags.w != 0) ? tex0.sample(samp0, in.vUV0) : float4(1.0);
    float4 c = texColor * in.vColor;

    if (c.a < u.uScalars.w) discard_fragment();

    if (u.uFlags2.x != 0) c.rgb *= tex1.sample(samp1, in.vUV1).rgb;
    if (u.uFlags.z != 0)  c.rgb = mix(u.uFogColor.rgb, c.rgb, in.vFogFactor);

    if (u.uFlags2.y == 0)
        c.rgb = pow(c.rgb, float3(u.uScalars2.x));

    return c;
}
)MSL";