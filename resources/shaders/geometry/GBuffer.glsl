#version 460 core

layout(binding = 0, rgba32ui) uniform uimage2DArray storageGBuffer;

#include "../Maths.glsl"

#define BYTES 8
#define UINT_SIZE 4
#define DATA_STREAM_SIZE 12
#define GBUFFER_LAYER_COUNT 3

struct GBuffer
{
    vec3 emissive;
    vec3 diffuse;
    vec3 specular;
    vec3 normal;
    float roughness;
    uint byteCount;
};

struct Stream
{
    // Do not touch data. Use the helper functions to get information.
    uint data[DATA_STREAM_SIZE];
    int byteOffset;  // Should start at 1.
};

void streamRecordUintCount(in out Stream stream)
{
    const uint uintCount = uint(ceil(stream.byteOffset / UINT_SIZE));
    stream.data[0] = bitfieldInsert(stream.data[0], uintCount, 0 * BYTES, 1 * BYTES);
}

void streamInsertBytes(in out Stream stream, uint bytes, int byteCount)
{
    const int index = int(floor(stream.byteOffset / UINT_SIZE));
    const int localOffset = stream.byteOffset % UINT_SIZE;
    const int localSpaceRemaining = UINT_SIZE - localOffset;
    if (localSpaceRemaining > byteCount)
    {
        stream.data[index] = bitfieldInsert(stream.data[index], bytes, localOffset * BYTES, byteCount * BYTES);
    }
    else
    {
        const int leftSize = localSpaceRemaining;
        const int rightSize = byteCount - localSpaceRemaining;

        const uint leftPart  = bitfieldExtract(bytes, 0 * BYTES,        leftSize * BYTES);
        const uint rightPart = bitfieldExtract(bytes, leftSize * BYTES, rightSize * BYTES);

        stream.data[index]     = bitfieldInsert(stream.data[index],      leftPart,  localOffset * BYTES, leftSize * BYTES);
        stream.data[index + 1] = bitfieldInsert(stream.data[index + 1],  rightPart, 0 * BYTES,           rightSize * BYTES);
    }

    stream.byteOffset += byteCount;
}

uint streamExtractBytes(in out Stream stream, int byteCount)
{
    const int index = int(floor(stream.byteOffset / UINT_SIZE));
    const int localOffset = stream.byteOffset % UINT_SIZE;
    const int localSpaceRemaining = UINT_SIZE - localOffset;

    stream.byteOffset += byteCount;

    if (localSpaceRemaining > byteCount)
    {
        return bitfieldExtract(stream.data[index], localOffset * BYTES, byteCount * BYTES);
    }
    else
    {
        const int leftSize = localSpaceRemaining;
        const int rightSize = byteCount - localSpaceRemaining;

        const uint leftPart   = bitfieldExtract(stream.data[index], localOffset * BYTES, leftSize * BYTES);
        const uint rightPart  = bitfieldExtract(stream.data[index + 1], 0 * BYTES, rightSize * BYTES);

        uint result = bitfieldInsert(0, leftPart, 0 * BYTES, leftSize * BYTES);
        result      = bitfieldInsert(result, rightPart, leftSize * BYTES, rightSize * BYTES);
        return result;
    }
}

void streamPackUnorm4x8(in out Stream stream, vec4 vector, int elementCount)
{
    streamInsertBytes(stream, packUnorm4x8(vector), elementCount);
}

vec4 streamUnpackUnorm4x8(in out Stream stream, int elementCount)
{
    return unpackUnorm4x8(streamExtractBytes(stream, elementCount));
}

void streamPackSnorm2x16(in out Stream stream, vec2 vector)
{
    streamInsertBytes(stream, packSnorm2x16(vector), UINT_SIZE);
}

vec2 streamUnpackSnorm2x16(in out Stream stream)
{
    return unpackSnorm2x16(streamExtractBytes(stream, UINT_SIZE));
}

// (Cigolle, Z. H, et al., 2014) A Survey of efficient representations for Independent Unit Vectors.
void streamPackNormal(in out Stream stream, vec3 normal)
{
    const vec2 p = normal.xy * (1.f / (abs(normal.x) + abs(normal.y) + abs(normal.z)));
    streamPackSnorm2x16(stream, (normal.z <= 0.f) ? ((1.f - abs(p.yx)) * signNotZero(p)) : p);
}

vec3 streamUnpackNormal(in out Stream stream)
{
    const vec2 e = streamUnpackSnorm2x16(stream);
    vec3 v = vec3(e.xy, 1.f - abs(e.x) - abs(e.y));
    if (v.z < 0.f)
        v.xy = (1.f - abs(v.yx)) * signNotZero(v.xy);
    return normalize(v);
}
// end.

void streamPushToStorageGBuffer(Stream stream, ivec2 coord)
{
    streamRecordUintCount(stream);
    const int uintCount = int(ceil(stream.byteOffset / UINT_SIZE));

    for (int index = 0; index < uintCount; index += 4)
    {
        imageStore(storageGBuffer, ivec3(coord, int(floor(index / 4))), uvec4(stream.data[index], stream.data[index + 1], stream.data[index + 2], stream.data[index + 3]));
    }
}

Stream streamPullFromStorageGBuffer(ivec2 coord)
{
    Stream stream;
    stream.byteOffset = 0;

    const uvec4 slice = imageLoad(storageGBuffer, ivec3(coord, 0));
    stream.data[0] = slice.x;
    stream.data[1] = slice.y;
    stream.data[2] = slice.z;
    stream.data[3] = slice.w;
    const uint uintCount = streamExtractBytes(stream, 1);

    int i = 1;
    for (int index = 4; index < uintCount * 4; index += 4)
    {
        const uvec4 slice = imageLoad(storageGBuffer, ivec3(coord, i++));
        stream.data[index + 0] = slice.x;
        stream.data[index + 1] = slice.y;
        stream.data[index + 2] = slice.z;
        stream.data[index + 3] = slice.w;
    }

    return stream;
}

void pushToStorageGBuffer(GBuffer gBuffer, ivec2 coord)
{
    Stream stream;
    stream.byteOffset = 1;

    streamPackNormal(stream, gBuffer.normal);
    streamPackUnorm4x8(stream, vec4(gBuffer.roughness, 0.f, 0.f, 0.f), 1);
    streamPackUnorm4x8(stream, vec4(gBuffer.diffuse, 0.f), 3);
    streamPackUnorm4x8(stream, vec4(gBuffer.specular, 0.f), 3);
    streamPackUnorm4x8(stream, vec4(gBuffer.emissive, 0.f), 3);

    streamPushToStorageGBuffer(stream, coord);
}

GBuffer pullFromStorageGBuffer(ivec2 coord)
{
    Stream stream = streamPullFromStorageGBuffer(coord);

    GBuffer gBuffer;
    gBuffer.normal      = streamUnpackNormal(stream);
    gBuffer.roughness   = streamUnpackUnorm4x8(stream, 1).x;
    gBuffer.diffuse     = streamUnpackUnorm4x8(stream, 3).xyz;
    gBuffer.specular    = streamUnpackUnorm4x8(stream, 3).xyz;
    gBuffer.emissive    = streamUnpackUnorm4x8(stream, 3).xyz;
    gBuffer.byteCount   = stream.byteOffset;  // This is the amount of bytes read. Not the actual amount submitted to the buffer.

    return gBuffer;
}
