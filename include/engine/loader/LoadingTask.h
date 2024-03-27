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
    class ITask
    {
    public:
        virtual ~ITask() = default;
        virtual bool checkAndPerform() = 0;
    };

    template<typename T>
    using TTaskFuture = std::future<T>;

    template<typename T>
    using TTaskCallback = std::function<void(T)>;

    template<typename T>
    class Task : public ITask
    {
    public:
        Task(TTaskFuture<T> future, const TTaskCallback<T> &callback)
            : mFutureResult(std::move(future)), mCallback(callback)
        {

        }

        bool checkAndPerform() override
        {
            // I don't think there's any other way of doing this. But it means that the main
            // thread is put to sleep for each item that is loading.
            if (const auto status = mFutureResult.wait_for(std::chrono::nanoseconds(0)); status == std::future_status::ready)
            {
                // Possible bug: There's no memory fence to force the other thread to fetch the new result.
                // This could result in errors.
                mCallback(mFutureResult.get());
                return true;
            }
            return false;
        }

    protected:
        TTaskFuture<T>   mFutureResult;
        TTaskCallback<T> mCallback;
    };

    template<typename T>
    std::unique_ptr<ITask> makeTask(TTaskFuture<T> future, const TTaskCallback<T&> &callback)
    {
        return std::make_unique<Task<T>>(std::move(future), callback);
    }
}
