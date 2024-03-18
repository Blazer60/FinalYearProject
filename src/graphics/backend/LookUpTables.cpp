/**
 * @file LookUpTables.cpp
 * @author Ryan Purse
 * @date 10/03/2024
 */


#include "LookUpTables.h"

#include "FileLoader.h"
#include "GraphicsFunctions.h"
#include "LtcSheenTable.h"

namespace graphics
{
    Lut precalculateLuts(const uint32_t size)
    {
        Lut results;
        results.specularDirectionalAlbedo = generateSpecularDirectionalAlbedoLut(glm::ivec2(size));
        results.specularDirectionalAlbedoAverage = generateSpecularDirectionalAlbedoAverageLut(
            size,
            results.specularDirectionalAlbedo);

        results.specularMissing = generateSpecularMissingLut(
            glm::ivec2(size),
            results.specularDirectionalAlbedo,
            results.specularDirectionalAlbedoAverage);

        results.ltcSheenTable = setupLtcSheenTable();
        results.sheenDirectionalAlbedo = generateSheenLut(
            glm::ivec2(size),
            results.ltcSheenTable);

        results.sheenMissing = generateSheenMissingLut(
            glm::ivec2(size),
            results.sheenDirectionalAlbedo);

        return results;
    }

    TextureBufferObject generateSpecularDirectionalAlbedoLut(const glm::ivec2& size)
    {
        Shader shader {
            { file::shaderPath() / "brdf/GgxDirectionalAlbedo.comp" }
        };

        TextureBufferObject lut(textureFormat::Rg16f);
        lut.resize(size);
        lut.setDebugName("Specular BRDF LUT");

        shader.bind();
        shader.image("lut", lut.getId(), lut.getFormat(), 0, false);

        dispatchCompute(glm::ceil(static_cast<glm::vec2>(size / 8)));

        return lut;
    }

    TextureBufferObject generateSpecularDirectionalAlbedoAverageLut(const uint32_t size, const TextureBufferObject &specularDA)
    {
        Shader shader {
            { file::shaderPath() / "brdf/GgxDirectionalAlbedoAverage.comp" }
        };

        TextureBufferObject lut(textureFormat::Rg16f);
        lut.resize(glm::ivec2(size, 1));
        lut.setDebugName("BRDF Average LUT");

        shader.bind();
        shader.set("brdfLut", specularDA.getId(), 0);
        shader.image("brdfAverageLut", lut.getId(), lut.getFormat(), 0, false, GL_WRITE_ONLY);

        dispatchCompute(glm::ceil(size / 8));

        return lut;
    }

    TextureBufferObject generateSpecularMissingLut(const glm::ivec2& size, const TextureBufferObject &specularDA, const TextureBufferObject &specularDaAverage)
    {
        Shader shader {
            { file::shaderPath() / "brdf/GgxSpecMissing.comp" }
        };

        TextureBufferObject lut(textureFormat::R16f);
        lut.resize(size);
        lut.setDebugName("Specular Missing LUT");

        shader.bind();
        shader.set("directionalAlbedoLut", specularDA.getId(), 0);
        shader.set("directionalAlbedoAverageLut", specularDaAverage.getId(), 1);
        shader.image("specMissing", lut.getId(), lut.getFormat(), 0, false);

        dispatchCompute(glm::ceil(static_cast<glm::vec2>(size / 8)));

        return lut;
    }

    TextureBufferObject setupLtcSheenTable()
    {
        TextureBufferObject table(textureFormat::Rgba16f);
        table.resize(glm::ivec2(sheen::tableSize));
        table.setDebugName("LTC Sheen LUT");

        const auto sheenData = sheen::data();
        table.upload(sheenData.data(), pixelFormat::Rgb);
        return table;
    }

    TextureBufferObject generateSheenLut(const glm::ivec2& size, const TextureBufferObject &sheenTable)
    {
        Shader mIntegrateSheenShader {
            { file::shaderPath() / "brdf/SheenDirectionalAlbedo.comp" }
        };

        TextureBufferObject lut(textureFormat::R16f);

        lut.resize(size);
        lut.setDebugName("Sheen Directional Albedo");

        mIntegrateSheenShader.bind();
        mIntegrateSheenShader.set("sheenTable", sheenTable.getId(), 0);
        mIntegrateSheenShader.image("sheenDirectionalAlbedo", lut.getId(), lut.getFormat(), 0, false);

        dispatchCompute(glm::ceil(static_cast<glm::vec2>(size / 8)));

        return lut;
    }

    TextureBufferObject generateSheenMissingLut(const glm::ivec2& size, const TextureBufferObject& sheenDA)
    {
        Shader mIntegrateSheenMissingShader {
            { file::shaderPath() / "brdf/SheenMissing.comp" }
        };

        TextureBufferObject lut(textureFormat::R16f);

        lut.resize(size);
        lut.setDebugName("Sheen Missing LUT");

        mIntegrateSheenMissingShader.bind();
        mIntegrateSheenMissingShader.set("sheenAlbedoLut", sheenDA.getId(), 0);
        mIntegrateSheenMissingShader.image("sheenMissingLut", lut.getId(), lut.getFormat(), 0, false);

        dispatchCompute(glm::ceil(static_cast<glm::vec2>(size / 8)));

        return lut;
    }
}
