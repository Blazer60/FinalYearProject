/**
 * @file Renderer.cpp
 * @author Ryan Purse
 * @date 14/07/2023
 */


#include "Renderer.h"
#include "WindowHelpers.h"
#include "Primitives.h"
#include "GraphicsFunctions.h"
#include "Shader.h"
#include "ProfileTimer.h"
#include "LtcSheenTable.h"
#include "backend/RendererBackend.h"
#include "shader/ShaderCompilation.h"

Renderer::Renderer() :
    mFullscreenTriangle(primitives::fullscreenTriangle()),
    mRendererBackend(new graphics::RendererBackend())
{
    // Blending texture data / enabling lerping.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

bool Renderer::debugMessageCallback(GLDEBUGPROC callback)
{
    int flags { 0 };  // Check to see if OpenGL debug context was set up correctly.
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    
    if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(callback, nullptr);
    }
    
    return (flags & GL_CONTEXT_FLAG_DEBUG_BIT) != 0;
}

std::string Renderer::getVersion()
{
    return (reinterpret_cast<const char*>(glGetString(GL_VERSION)));
}

Renderer::~Renderer()
{
    delete mRendererBackend;
}

void Renderer::drawMesh(
    const uint32_t vao, int32_t indiciesCount, const glm::mat4& matrix, const graphics::MaterialData& material)
{
    if (material.layers.empty())
        CRASH("No material layers results in undefined behaviour");

    if (!material.masks.empty() && material.layers.size() > 1)
    {
        mMultiMaterialGeometryQueue.emplace_back(vao, indiciesCount, matrix);
        mMultiMaterialQueue.push_back(material);
    }
    else
    {
        mSingleMaterialGeometryQueue.emplace_back(vao, indiciesCount, matrix);
        mSingleMaterialQueue.push_back(material);
    }
}

void Renderer::drawMesh(const SubMesh& surface, const glm::mat4& matrix, const graphics::MaterialData& material)
{
    drawMesh(surface.vao(), surface.indicesCount(), matrix, material);
}

void Renderer::drawDebugMesh(const uint32_t vao, const int32_t indicesCount, const glm::mat4& matrix, const glm::vec3& colour)
{
    mDebugQueue.emplace_back(graphics::DebugQueueObject { vao, indicesCount, matrix, colour });
}

void Renderer::drawDebugMesh(const SubMesh& subMesh, const glm::mat4& matrix, const glm::vec3& colour)
{
    drawDebugMesh(subMesh.vao(), subMesh.indicesCount(), matrix, colour);
}

void Renderer::drawDebugMesh(const SharedMesh& mesh, const glm::mat4& matrix, const glm::vec3& colour)
{
    for (int i = 0; i < mesh->size(); ++i)
    {
        SubMesh &subMesh = *(*mesh)[i];
        drawDebugMesh(subMesh, matrix, colour);
    }
}

void Renderer::drawDebugLine(const glm::vec3& startPosition, const glm::vec3& endPosition, const glm::vec3& colour)
{
    mLineQueue.emplace_back(graphics::LineQueueObject { startPosition, endPosition, colour });
}

void Renderer::submit(const CameraSettings &cameraSettings)
{
    mCameraQueue.emplace_back(cameraSettings);
}

void Renderer::submit(const graphics::DirectionalLight &directionalLight)
{
    mDirectionalLightQueue.emplace_back(directionalLight);
}

void Renderer::submit(const graphics::PointLight &pointLight)
{
    mPointLightQueue.emplace_back(pointLight);
}

void Renderer::submit(const graphics::Spotlight &spotLight)
{
    mSpotlightQueue.emplace_back(spotLight);
}

void Renderer::render() const
{
    PROFILE_FUNC();
    if (window::bufferSize().x <= 0 || window::bufferSize().y <= 0)
        return;

    mRendererBackend->copyQueues({
        mCameraQueue,
        mDirectionalLightQueue,
        mPointLightQueue,
        mSpotlightQueue,
        mDebugQueue,
        mLineQueue,
        mMultiMaterialGeometryQueue,
        mMultiMaterialQueue,
        mSingleMaterialGeometryQueue,
        mSingleMaterialQueue,
    });
    mRendererBackend->execute();
}

void Renderer::clear()
{
    mCameraQueue.clear();
    mDirectionalLightQueue.clear();
    mPointLightQueue.clear();
    mSpotlightQueue.clear();
    mDebugQueue.clear();
    mLineQueue.clear();

    mMultiMaterialGeometryQueue.clear();
    mMultiMaterialQueue.clear();

    mSingleMaterialGeometryQueue.clear();
    mSingleMaterialQueue.clear();
}

void Renderer::generateSkybox(const std::string_view path, const glm::ivec2 desiredSize) const
{
    mRendererBackend->generateSkybox(path, desiredSize);
}

void Renderer::drawFullscreenTriangleNow() const
{
    glBindVertexArray(mFullscreenTriangle.vao());
    glDrawElements(GL_TRIANGLES, mFullscreenTriangle.indicesCount(), GL_UNSIGNED_INT, nullptr);
}

const TextureBufferObject &Renderer::getPrimaryBuffer() const
{
    return mRendererBackend->getContext().backBuffer;
}

const TextureBufferObject &Renderer::getLightBuffer() const
{
    return mRendererBackend->getContext().lightBuffer;
}

const TextureBufferObject &Renderer::getDepthBuffer() const
{
    return mRendererBackend->getContext().depthBuffer;
}

const TextureBufferObject& Renderer::getDebugBuffer() const
{
    return mRendererBackend->getDebugBuffer();
}

const TextureBufferObject& Renderer::getFromGBuffer(const graphics::gbuffer type, const bool gammaCorrect, const glm::vec4 &defaultValue) const
{
    return mRendererBackend->queryGbuffer(type, gammaCorrect, defaultValue);
}

const TextureBufferObject &Renderer::whiteFurnaceTest() const
{
    return mRendererBackend->whtieFurnacetest();
}

const TextureBufferObject& Renderer::drawTileClassification() const
{
    return mRendererBackend->tileOverlay();
}

void Renderer::setIblMultiplier(const float multiplier) const
{
    mRendererBackend->setIblMultiplier(glm::abs(multiplier));
}

void Renderer::setUseUberVariant(const bool useUber) const
{
    mRendererBackend->setUseUberVariant(useUber);
}

void Renderer::rendererGuiNewFrame()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

