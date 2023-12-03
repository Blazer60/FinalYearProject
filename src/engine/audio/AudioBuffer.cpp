/**
 * @file Audio.cpp
 * @author Ryan Purse
 * @date 03/12/2023
 */


#include "AudioBuffer.h"

#include <al.h>
#include <alc.h>

#include "Logger.h"
#include "LoggerMacros.h"

#include <stb_vorbis.c>

namespace engine
{
    AudioBuffer::AudioBuffer(const std::filesystem::path& path)
    {
        if (path.empty())
            return;

        if (!std::filesystem::exists(path))
        {
            WARN("File % does not exist.\nAborting audio generation");
            return;
        }

        int channels = 0;
        int sampleRate = 0;
        short *output = nullptr;
        const size_t samples = stb_vorbis_decode_filename(path.string().c_str(), &channels, &sampleRate, &output);

        if (samples == -1)
        {
            WARN("Failed to decode file %", path);
            return;
        }

        const ALenum format = channels < 2 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        alGenBuffers(1, &mId);
        alBufferData(mId, format, output, static_cast<ALsizei>(channels * samples * sizeof(short)), sampleRate);

        free(output);
    }

    AudioBuffer::~AudioBuffer()
    {
        if (mId != 0)
            alDeleteBuffers(1, &mId);
    }

    ALuint AudioBuffer::id() const
    {
        return mId;
    }
}
