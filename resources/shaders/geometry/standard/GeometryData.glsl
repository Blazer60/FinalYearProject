#version 460 core

struct LayerData
{
    vec4 diffuseColour;
    vec4 specularColour;
    vec4 sheenColour;
    vec4 topSpecularColour;
    vec4 transmittanceColour;
    float roughness;
    float sheenRoughness;
    float topRoughness;
    float topThickness;
    float topCoverage;

    int diffuseTextureIndex;
    int specularTextureIndex;
    int normalTextureIndex;
    int roughnessTextureIndex;
    int sheenTextureIndex;
    int sheenRoughnessTextureIndex;
    int metallicTextureIndex;
    int topSpecularColourTextureIndex;
    int topNormalTextureIndex;
    int transmittanceColourTextureIndex;
    int topRoughnessTextureIndex;
    int topThicknessTextureIndex;
    int topCoverageTextureIndex;
    int _padding01;
    int _padding02;
};

#define PASSTHROUGH_FLAG_DIFFUSE 1
#define PASSTHROUGH_FLAG_SPECULAR 2
#define PASSTHROUGH_FLAG_NORMAL 4
#define PASSTHROUGH_FLAG_ROUGHNESS 8
#define PASSTHROUGH_FLAG_SHEEN 16
#define PASSTHROUGH_FLAG_SHEEN_ROUGHNESS 32
#define PASSTHROUGH_FLAG_TOP_SPECULAR 64
#define PASSTHROUGH_FLAG_TRANSMITTANCE 128
#define PASSTHROUGH_FLAG_TOP_ROUGHNESS 256
#define PASSTHROUGH_FLAG_TOP_THICKNESS 512
#define PASSTHROUGH_FLAG_TOP_COVERAGE 1024
#define PASSTHROUGH_FLAG_TOP_NORMAL 2048

#define OPERATION_LERP 0
#define OPERATION_THRESHOLD 1

#define WRAP_REPEAT 0
#define WRAP_CLAMP_TO_EDGE 1

struct MaskData
{
    float alpha;
    int textureIndex;
    uint passthroughFlags;
    uint operation;
};

struct TextureData
{
    uint width;
    uint height;
    uint wrapOp;
};