/**
 * @file UberLayer.h
 * @author Ryan Purse
 * @date 11/03/2024
 */


#pragma once

#include "Callback.h"
#include "Drawable.h"
#include "FileLoader.h"
#include "MaterialData.h"
#include "Pch.h"
#include "Texture.h"
#include "TexturePool.h"

namespace load
{
    std::shared_ptr<Texture> texture(const std::filesystem::path&);
}

namespace engine
{
    /**
     * @author Ryan Purse
     * @date 11/03/2024
     */
    class UberLayer
        : public ui::Drawable
    {
        friend class UberMaterial;
        typedef std::function<void(graphics::TexturePool&, graphics::LayerData&)> UpdateFunc;
    public:
        explicit UberLayer(const std::filesystem::path &path);
        ~UberLayer() override;
        std::string name() const { return mName; }
        std::filesystem::path path() const { return mPath; }
        void onDrawUi() override;
        // Saving is handled by the resource pool once the last instance has gone out of scope.
        void saveToDisk() const;

        void lookForTextureChange(const std::shared_ptr<Texture> &texture);

        std::vector<UpdateFunc> layerUpdates;

        static constexpr float minThickness = 0.f;
        static constexpr float maxThickness = 0.2f;
        static float remapThickness(float topThickness);
    protected:
        void loadFromDisk();

        std::string mName;
        std::filesystem::path mPath;

        std::shared_ptr<Texture> mDiffuseTexture            = load::texture("");
        std::shared_ptr<Texture> mSpecularTexture           = load::texture("");
        std::shared_ptr<Texture> mNormalTexture             = load::texture("");
        std::shared_ptr<Texture> mRoughnessTexture          = load::texture("");
        std::shared_ptr<Texture> mSheenTexture              = load::texture("");
        std::shared_ptr<Texture> mSheenRoughnessTexture     = load::texture("");
        std::shared_ptr<Texture> mMetallicTexture           = load::texture("");
        std::shared_ptr<Texture> mTopSpecularTexture        = load::texture("");
        std::shared_ptr<Texture> mTopNormalTexture          = load::texture("");
        std::shared_ptr<Texture> mTransmittanceTexture      = load::texture("");
        std::shared_ptr<Texture> mTopRoughnessTexture       = load::texture("");
        std::shared_ptr<Texture> mTopThicknessTexture       = load::texture("");
        std::shared_ptr<Texture> mTopCoverageTexture        = load::texture("");

        glm::vec3 mDiffuseColour = glm::vec3(1.f);
        glm::vec3 mSpecularColour = glm::vec3(0.f);
        float     mRoughness = 1.f;
        glm::vec3 mSheenColour = glm::vec3(0.f);
        float     mSheenRoughness = 0.5f;

        glm::vec3 mTopSpecularColour = glm::vec3(0.f);
        float     mTopRoughness = 0.2f;
        glm::vec3 mTransmittanceColour = glm::vec3(0.f);
        float     mTopThickness = 0.002f;
        float     mTopCoverage = 0.f;


        graphics::WrapOp mDiffuseWrapOp         = graphics::WrapOp::Repeat;
        graphics::WrapOp mSpecularWrapOp        = graphics::WrapOp::Repeat;
        graphics::WrapOp mNormalWrapOp          = graphics::WrapOp::Repeat;
        graphics::WrapOp mRoughnessWrapOp       = graphics::WrapOp::Repeat;
        graphics::WrapOp mSheenWrapOp           = graphics::WrapOp::Repeat;
        graphics::WrapOp mSheenRoughnessWrapOp  = graphics::WrapOp::Repeat;
        graphics::WrapOp mMetallicWrapOp        = graphics::WrapOp::Repeat;
        graphics::WrapOp mTopSpecularWrapOp     = graphics::WrapOp::Repeat;
        graphics::WrapOp mTopNormalWrapOp       = graphics::WrapOp::Repeat;
        graphics::WrapOp mTransmittanceWrapOp   = graphics::WrapOp::Repeat;
        graphics::WrapOp mTopRoughnessWrapOp    = graphics::WrapOp::Repeat;
        graphics::WrapOp mTopThicknessWrapOp    = graphics::WrapOp::Repeat;
        graphics::WrapOp mTopCoverageWrapOp     = graphics::WrapOp::Repeat;


        uint32_t mCallbackToken = 0;
    };

} // engine
