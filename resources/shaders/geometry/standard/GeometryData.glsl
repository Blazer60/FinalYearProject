#version 460 core

struct LayerData
{
    vec4 diffuseColour;
    vec4 specularColour;
    vec4 sheenColour;
    float roughness;
    float sheenRoughness;
    int diffuseTextureIndex;
    int specularTextureIndex;

    int normalTextureIndex;
    int roughnessTextureIndex;
    int sheenTextureIndex;
    int sheenRoughnessTextureIndex;

    int metallicTextureIndex;
    int _padding01;
    int _padding02;
    int _padding03;
};

#define PASSTHROUGH_FLAG_DIFFUSE 1
#define PASSTHROUGH_FLAG_SPECULAR 2
#define PASSTHROUGH_FLAG_NORMAL 4
#define PASSTHROUGH_FLAG_ROUGHNESS 8
#define PASSTHROUGH_FLAG_SHEEN 16
#define PASSTHROUGH_FLAG_SHEEN_ROUGHNESS 32

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