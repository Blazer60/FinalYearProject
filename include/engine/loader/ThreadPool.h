/**
 * @file ThreadPool.h
 * @author Ryan Purse
 * @date 27/03/2024
 */


#pragma once

#include <mutex>
#include <queue>

#include "LoadingTask.h"
#include "Pch.h"

namespace load
{
    /**
     * @author Ryan Purse
     * @date 27/03/2024
     */
    class ThreadPool 
    {
    public:
        ThreadPool();
        ~ThreadPool();
        void start();
        void queueJob(std::unique_ptr<IThreadTask> task);
        void stop();
        bool isBusy();
        void resolveFinishedJobs();
        uint32_t getJobCount() const { return mJobCount; }

    protected:
        void threadLoop();

        bool mShouldTerminate = false;
        std::vector<std::thread> mThreads;

        std::queue<std::unique_ptr<IThreadTask>> mJobs;
        std::condition_variable mMutexCondition;
        std::mutex mJobsMutex;

        std::queue<std::unique_ptr<IThreadTask>> mFinishedJobs;
        std::mutex mFinishedJobsMutex;

        uint32_t mJobCount = 0;  // Only the main thread can touch this.
    };
} // load
