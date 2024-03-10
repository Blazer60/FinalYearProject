/**
 * @file Skybox.cpp
 * @author Ryan Purse
 * @date 10/03/2024
 */


#include "Skybox.h"

#include "FramebufferObject.h"
#include "Mesh.h"
#include "Primitives.h"

namespace graphics
{
    void Skybox::generate(const std::string_view path, const glm::ivec2 size)
    {
        HdrTexture hdrImage(path);
        hdrImage.setDebugName("HDR Skybox Image");
        createCubemapFromHdrTexture(hdrImage, size);
        generateIrradianceMap(size / 8);
        generatePrefilterMap(size / 4);
        isValid = true;
    }

    void Skybox::createCubemapFromHdrTexture(const HdrTexture& hdrImage, glm::ivec2 size)
    {
        hdrSkybox.resize(size);
        hdrSkybox.setDebugName("Skybox Cubemap");

        FramebufferObject auxiliaryFrameBuffer(GL_ONE, GL_ONE, GL_ALWAYS);

        const glm::mat4 views[] = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        auxiliaryFrameBuffer.bind();
        mHdrToCubemapShader.bind();
        mHdrToCubemapShader.set("u_texture", hdrImage.getId(), 0);

        glViewport(0, 0, size.x, size.y);

        const SubMesh fullscreenTriangle = primitives::fullscreenTriangle();
        glBindVertexArray(fullscreenTriangle.vao());

        for (int i = 0; i < 6; ++i)
        {
            mHdrToCubemapShader.set("u_view_matrix", views[i]);
            auxiliaryFrameBuffer.attach(&hdrSkybox, 0, i);
            auxiliaryFrameBuffer.clear(glm::vec4(glm::vec3(0.f), 1.f));

            glDrawElements(GL_TRIANGLES, fullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);

            auxiliaryFrameBuffer.detach(0);
        }
    }

    void Skybox::generateIrradianceMap(const glm::ivec2 size)
    {
        irradianceMap.resize(size);
        irradianceMap.setDebugName("Irradiance Map");

        FramebufferObject auxiliaryFrameBuffer(GL_ONE, GL_ONE, GL_ALWAYS);

        const glm::mat4 views[] = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        auxiliaryFrameBuffer.bind();
        mCubemapToIrradianceShader.bind();
        mCubemapToIrradianceShader.set("u_environment_texture", hdrSkybox.getId(), 0);

        glViewport(0, 0, size.x, size.y);

        const auto fullscreenTriangle = primitives::fullscreenTriangle();
        glBindVertexArray(fullscreenTriangle.vao());

        for (int i = 0; i < 6; ++i)
        {
            mCubemapToIrradianceShader.set("u_view_matrix", views[i]);
            auxiliaryFrameBuffer.attach(&irradianceMap, 0, i);
            auxiliaryFrameBuffer.clear(glm::vec4(glm::vec3(0.f), 1.f));

            glDrawElements(GL_TRIANGLES, fullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);

            auxiliaryFrameBuffer.detach(0);
        }
    }

    void Skybox::generatePrefilterMap(const glm::ivec2 size)
    {
        prefilterMap.resize(size);
        prefilterMap.setDebugName("Prefilter Map");

        FramebufferObject auxiliaryFrameBuffer(GL_ONE, GL_ONE, GL_ALWAYS);

        const glm::mat4 views[] = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        auxiliaryFrameBuffer.bind();
        mPreFilterShader.bind();
        mPreFilterShader.set("u_environment_texture", hdrSkybox.getId(), 0);

        const auto fullscreenTriangle = primitives::fullscreenTriangle();
        glBindVertexArray(fullscreenTriangle.vao());

        for (int mip = 0; mip < prefilterMap.getMipLevels(); ++mip)
        {
            const glm::ivec2 mipSize = size >> mip;
            glViewport(0, 0, mipSize.x, mipSize.y);

            const float roughness = static_cast<float>(mip) / (static_cast<float>(prefilterMap.getMipLevels()) - 1.f);
            mPreFilterShader.set("u_roughness", roughness);

            for (int i = 0; i < 6; ++i)
            {
                mPreFilterShader.set("u_view_matrix", views[i]);
                auxiliaryFrameBuffer.attach(&prefilterMap, 0, i, mip);
                auxiliaryFrameBuffer.clear(glm::vec4(glm::vec3(0.f), 1.f));

                glDrawElements(GL_TRIANGLES, fullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);

                auxiliaryFrameBuffer.detach(0);
            }
        }
    }
} // graphics
