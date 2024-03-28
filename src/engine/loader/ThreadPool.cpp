/**
 * @file ThreadPool.cpp
 * @author Ryan Purse
 * @date 27/03/2024
 */


#include "ThreadPool.h"

#include "Logger.h"
#include "LoggerMacros.h"
#include "ProfileTimer.h"

namespace load
{
    ThreadPool::ThreadPool()
    {
        start();
    }

    ThreadPool::~ThreadPool()
    {
        stop();
    }

    void ThreadPool::start()
    {
        const uint32_t threadCount = std::thread::hardware_concurrency() - 1;
        for (uint32_t i = 0; i < threadCount; ++i)
            mThreads.emplace_back(std::thread(&ThreadPool::threadLoop, this));
        MESSAGE_VERBOSE("Generating thread pool of size %", threadCount);
    }

    void ThreadPool::queueJob(std::unique_ptr<IThreadTask> task)
    {
        ++mJobCount;
        {
            const std::unique_lock lock(mJobsMutex);
            mJobs.push(std::move(task));
        }
        mMutexCondition.notify_one();
    }

    void ThreadPool::stop()
    {
        {
            const std::unique_lock lock(mJobsMutex);
            mShouldTerminate = true;
        }
        mMutexCondition.notify_all();
        for (std::thread &thread : mThreads)
            thread.join();

        mThreads.clear();
    }

    bool ThreadPool::isBusy()
    {
        bool isBusy = false;
        {
            const std::unique_lock lock(mJobsMutex);
            isBusy = !mJobs.empty();
        }
        return isBusy;
    }

    void ThreadPool::resolveFinishedJobs()
    {
        PROFILE_FUNC();
        {
            const std::unique_lock lock(mFinishedJobsMutex);
            while (!mFinishedJobs.empty())
            {
                const std::unique_ptr<IThreadTask> finishedJob = std::move(mFinishedJobs.front());
                mFinishedJobs.pop();

                finishedJob->callback();
                --mJobCount;
            }
        }
    }

    void ThreadPool::threadLoop()
    {
        while (true)
        {
            std::unique_ptr<IThreadTask> job;
            {
                std::unique_lock lock(mJobsMutex);
                mMutexCondition.wait(lock, [this] {
                    return !mJobs.empty() || mShouldTerminate;
                });
                if (mShouldTerminate)
                    return;

                job = std::move(mJobs.front());
                mJobs.pop();
            }
            job->run();
            {
                const std::unique_lock lock(mFinishedJobsMutex);
                mFinishedJobs.push(std::move(job));
            }
        }
    }
} // load
