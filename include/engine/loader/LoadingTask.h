/**
 * @file LoadingTask.h
 * @author Ryan Purse
 * @date 27/03/2024
 */


#pragma once

#include <future>

#include "Disk.h"
#include "Pch.h"

namespace load
{
    class IThreadTask
    {
    public:
        virtual ~IThreadTask() = default;

        virtual void run() = 0;
        virtual void callback() = 0;
    };

    template<typename T>
    using TTaskRun = std::function<T()>;

    template<typename T>
    using TTaskCallback = std::function<void(T&)>;

    template<typename T>
    class ThreadTask : public IThreadTask
    {
    public:
        ThreadTask(const TTaskRun<T> &run, const TTaskCallback<T> &callback)
            : mRun(run), mCallback(callback)
        {

        }

        void run() override
        {
            mResult = mRun();
        }

        void callback() override
        {
            mCallback(mResult);
        }

    protected:
        T mResult;
        TTaskRun<T>   mRun;
        TTaskCallback<T> mCallback;
    };

    template<typename T>
    std::unique_ptr<IThreadTask> makeJob(const TTaskRun<T> &run, const TTaskCallback<T> &callback)
    {
        return std::make_unique<ThreadTask<T>>(run, callback);
    }
}
