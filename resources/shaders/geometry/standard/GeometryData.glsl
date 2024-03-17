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
};

#define PASSTHROUGH_FLAG_DIFFUSE 1
#define PASSTHROUGH_FLAG_SPECULAR 2
#define PASSTHROUGH_FLAG_NORMAL 4
#define PASSTHROUGH_FLAG_ROUGHNESS 8
#define PASSTHROUGH_FLAG_SHEEN 16
#define PASSTHROUGH_FLAG_SHEEN_ROUGHNESS 32

#define OPERATION_LERP 0
#define OPERATION_THRESHOLD 1

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
};