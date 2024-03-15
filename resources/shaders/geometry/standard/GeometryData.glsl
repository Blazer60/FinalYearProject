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

struct MaskData
{
    float alpha;
    int textureIndex;
};

struct TextureData
{
    uint width;
    uint height;
};