#version 460 core

#if !defined(FRAGMENT_OUTPUT)
    #define FRAGMENT_OUTPUT 0
#endif

#define BYTES 8
#define UINT_SIZE 4
#define DATA_STREAM_SIZE 25
#define GBUFFER_LAYER_COUNT 3
#define GBUFFER_UINT_COUNT 12
#define STREAM_HEADER_BYTE_COUNT 1

#include "../Maths.glsl"
#include "../interfaces/GBufferFlags.h"

layout(binding = 0, rgba32ui) uniform uimage2DArray storageGBuffer;

// Requires a depth prepass and early depth testing.
//layout(binding = 0, std430) buffer StorageGBufferSsbo
//{
//    uint uintOffset;  // uint to avoid race-conditions.
//    uint byteStream[];
//} storageGBufferSsbo;

#if FRAGMENT_OUTPUT > 0
layout(location = 0) out uvec4 oGBuffer0;
layout(location = 1) out uvec4 oGBuffer1;
layout(location = 2) out uvec4 oGBuffer2;
#endif

struct GBuffer
{
    uint flags;
    vec3 emissive;
    vec3 diffuse;
    vec3 specular;
    vec3 normal;
    float roughness;
    vec3 fuzzColour;
    float fuzzRoughness;
    uint byteCount;
};

struct Stream
{
    // Do not touch data. Use the helper functions to get information.
    uint data[DATA_STREAM_SIZE];
    int byteOffset;
};

uint streamRecordUintCount(in out Stream stream)
{
    const uint uintCount = uint(ceil(stream.byteOffset / UINT_SIZE));
    stream.data[0] = bitfieldInsert(stream.data[0], uintCount, 0 * BYTES, 1 * BYTES);
    return uintCount;
}

// Unused until SSBO is in use.
void streamRecordSsboIndex(in out Stream stream, uint index)
{
    stream.data[0] = bitfieldInsert(stream.data[0], index, 1 * BYTES, 3 * BYTES);
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

// Unused until SSBO is in use.
//uint requestAdditionalStorage(uint uintCount)
//{
//    return atomicAdd(storageGBufferSsbo.uintOffset, uintCount);
//}

void streamPushToStorageGBuffer(Stream stream, ivec2 coord)
{
    streamRecordUintCount(stream);
    const int uintCount = int(ceil(stream.byteOffset / UINT_SIZE));

#if FRAGMENT_OUTPUT > 0
    oGBuffer0.xyzw = uvec4(stream.data[0], stream.data[1], stream.data[2], stream.data[3]);
    oGBuffer1.xyzw = uvec4(stream.data[4], stream.data[5], stream.data[6], stream.data[7]);
    oGBuffer2.xyzw = uvec4(stream.data[8], stream.data[9], stream.data[10], stream.data[11]);
#else
    int imageIndex = 0;
    for (int streamIndex = 0; streamIndex < uintCount; streamIndex += 4)
    {
        imageStore(storageGBuffer, ivec3(coord, imageIndex++),
            uvec4(
                stream.data[streamIndex + 0],
                stream.data[streamIndex + 1],
                stream.data[streamIndex + 2],
                stream.data[streamIndex + 3]));
    }
#endif
}

Stream streamPullFromStorageGBuffer(ivec2 coord, out uint uintCount)
{
    Stream stream;
    stream.byteOffset = 0;

    const uvec4 slice = imageLoad(storageGBuffer, ivec3(coord, 0));
    stream.data[0] = slice.x;
    stream.data[1] = slice.y;
    stream.data[2] = slice.z;
    stream.data[3] = slice.w;

    uintCount = streamExtractBytes(stream, 1);

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

// Special case for tile-classification since we only need the first few bytes.
uint pullFlagsFromStorageGBuffer(ivec2 coord)
{
    const uvec4 slice = imageLoad(storageGBuffer, ivec3(coord, 0));
    return bitfieldExtract(slice.x, STREAM_HEADER_BYTE_COUNT * BYTES, GBUFFER_FLAG_BYTE_COUNT * BYTES);
}

GBuffer gBufferCreate()
{
    GBuffer gBuffer;

    gBuffer.flags = 0;
    gBuffer.emissive = vec3(0);
    gBuffer.diffuse = vec3(0);
    gBuffer.specular = vec3(0);
    gBuffer.normal = vec3(0);
    gBuffer.roughness = 0;
    gBuffer.fuzzColour = vec3(0);
    gBuffer.fuzzRoughness = 0;
    gBuffer.byteCount = 0;

    return gBuffer;
}

void gBufferSetFlag(in out GBuffer gBuffer, int flag, int value)
{
    gBuffer.flags = bitfieldInsert(gBuffer.flags, value, flag, 1);
}

int gBufferHasFlag(GBuffer gBuffer, int flag)
{
    return int(bitfieldExtract(gBuffer.flags, flag, 1));
}

int gBufferIsValid(GBuffer gBuffer)
{
    return int(gBuffer.byteCount != 0);
}

void pushToStorageGBuffer(GBuffer gBuffer, ivec2 coord)
{
    // So that later on we can tell if something is a sky pixel or not.
    gBufferSetFlag(gBuffer, GBUFFER_FLAG_MATERIAL_BIT, 1);

    Stream stream;
    stream.byteOffset = STREAM_HEADER_BYTE_COUNT;

    streamInsertBytes(stream, gBuffer.flags, GBUFFER_FLAG_BYTE_COUNT);

    streamPackNormal(stream, gBuffer.normal);
    streamPackUnorm4x8(stream, vec4(gBuffer.roughness, 0.f, 0.f, 0.f), 1);
    streamPackUnorm4x8(stream, vec4(gBuffer.diffuse, 0.f), 3);
    streamPackUnorm4x8(stream, vec4(gBuffer.specular, 0.f), 3);
    streamPackUnorm4x8(stream, vec4(gBuffer.emissive, 0.f), 3);

    if (gBufferHasFlag(gBuffer, GBUFFER_FLAG_FUZZ_BIT) == 1)
    {
        streamPackUnorm4x8(stream, vec4(gBuffer.fuzzColour, 0.f), 3);
        streamPackUnorm4x8(stream, vec4(gBuffer.fuzzRoughness, 0.f, 0.f, 0.f), 1);
    }

    streamPushToStorageGBuffer(stream, coord);
}

GBuffer pullFromStorageGBuffer(ivec2 coord)
{
    uint uintCount;
    Stream stream = streamPullFromStorageGBuffer(coord, uintCount);

    if (uintCount == 0)
    {
        return gBufferCreate();  // I.e.: A sky pixel.
    }

    GBuffer gBuffer = gBufferCreate();
    gBuffer.flags         = streamExtractBytes(stream, GBUFFER_FLAG_BYTE_COUNT);

    gBuffer.normal        = streamUnpackNormal(stream);
    gBuffer.roughness     = streamUnpackUnorm4x8(stream, 1).x;
    gBuffer.diffuse       = streamUnpackUnorm4x8(stream, 3).xyz;
    gBuffer.specular      = streamUnpackUnorm4x8(stream, 3).xyz;
    gBuffer.emissive      = streamUnpackUnorm4x8(stream, 3).xyz;

    if (gBufferHasFlag(gBuffer, GBUFFER_FLAG_FUZZ_BIT) == 1)
    {
        gBuffer.fuzzColour    = streamUnpackUnorm4x8(stream, 3).xyz;
        gBuffer.fuzzRoughness = streamUnpackUnorm4x8(stream, 1).x;
    }

    gBuffer.byteCount   = stream.byteOffset;  // This is the amount of bytes read. Not the actual amount submitted to the buffer.

    return gBuffer;
}
