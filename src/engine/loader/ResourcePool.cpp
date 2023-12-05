/**
 * @file ResourcePool.cpp
 * @author Ryan Purse
 * @date 01/11/2023
 */


#include "ResourcePool.h"
#include <Statistics.h>
#include <FileLoader.h>

namespace engine
{
    template<typename T>
    void internalClean(std::unordered_map<std::string, std::shared_ptr<T>> &map)
    {
        std::vector<std::string> toDelete;
        for (auto &[hashName, value] : map)
        {
            if (value.use_count() <= 1)
                toDelete.push_back(hashName);
        }
        for (const std::string &hashName : toDelete)
        {
            MESSAGE("Cleaning up %", hashName);
            map.erase(hashName);
        }
    }
    
    void ResourcePool::clean()
    {
        internalClean(mShaders);
        internalClean(mModels);
        internalClean(mTextures);
        internalClean(mAudioBuffers);
    }
    
    std::shared_ptr<Shader> ResourcePool::loadShader(
        const std::filesystem::path &vertexPath,
        const std::filesystem::path &fragmentPath)
    {
        const std::string hashName = vertexPath.string() + fragmentPath.string();
        if (auto it = mShaders.find(hashName); it != mShaders.end())
            return it->second;
        
        // Currently no error handling for incorrect path.
        auto resource = std::make_shared<Shader>(vertexPath, fragmentPath);
        mShaders[hashName] = resource;
        return resource;
    }
    
    std::shared_ptr<Texture> ResourcePool::loadTexture(const std::filesystem::path &path)
    {
        const std::string hashName = path.string();
        if (auto it = mTextures.find(hashName); it != mTextures.end())
            return it->second;
        
        auto resource = std::make_shared<Texture>(path);
        mTextures[hashName] = resource;
        return resource;
    }

    std::shared_ptr<AudioBuffer> ResourcePool::loadAudioBuffer(const std::filesystem::path& path)
    {
        const std::string hashName = path.string();
        if (auto it = mAudioBuffers.find(hashName); it != mAudioBuffers.end())
            return it->second;

        auto resource = std::make_shared<AudioBuffer>(path);
        mAudioBuffers[hashName] = resource;
        return resource;
    }
}
