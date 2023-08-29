/**
 * @file EngineMemory.h
 * @author Ryan Purse
 * @date 28/08/2023
 */

#pragma once

#include <cstdint>
#include <type_traits>
#include <typeinfo>
#include <iostream>

class InvalidReference;

class ControlBlock
{
public:
    ControlBlock(ControlBlock &rhs) = delete;
    ControlBlock& operator=(ControlBlock &rhs) = delete;
protected:
    ControlBlock() = default;
    virtual ~ControlBlock() = default;
    
    void incrementReference() { ++mRefCount; }
    void decrementReference()
    {
        if (--mRefCount == 0 && !isValid())
            delete this;
    }
    
    [[nodiscard]] virtual bool isValid() const = 0;
    [[nodiscard]] virtual uint64_t typeHash() const = 0;
    virtual void deleteResource() = 0;
    
    uint32_t mRefCount { 0 };

private:
    template<typename> friend class Resource;
    template<typename> friend class Ref;
};

template<typename T>
class HeapResource
    : public ControlBlock
{
protected:
    template<typename ...TArgs>
    explicit HeapResource(TArgs &&... args)
        : mData(new T(std::forward<TArgs>(args)...))
    {
    
    }
    
    ~HeapResource() override = default;
    
    [[nodiscard]] bool isValid() const override { return mData != nullptr; }
    [[nodiscard]] uint64_t typeHash() const override { return typeid(T).hash_code(); };
    
    void deleteResource() override
    {
        delete mData;
        mData = nullptr;
        if (mRefCount == 0)
            delete this;
    }
    
    T *mData { nullptr };

private:
    template<typename> friend class Resource;
};

/**
 * @brief A heap allocated resource that has one owner.
 * Behaves like a std::shared_ptr\<T> but cannot be copied (like std::unique_ptr\<T>).
 * Use Ref<T> if you need a non-owning pointer to this resource.
 * Resources may be up-casted with the original derived class being stored. This is
 * useful when Resources are stored in containers.
 * @tparam T - The underlying type that you want to store.
 * @see Ref\<T>
 * @see std::unique_ptr\<T>
 */
template<typename T>
class Resource
{
public:
    friend void swap(Resource &lhs, Resource &rhs);
    
    // Default constructor.
    explicit Resource() = default;
    
    // Default constructor
    template<typename ...TArgs>
    explicit Resource(TArgs && ...args)
    {
        auto *cb = new HeapResource<T>(std::forward<TArgs>(args)...);
        mControlBlock = cb;
        mPtr = cb->mData;
    }
    
    // Copy-constructor. Not allowed.
    Resource(Resource<T> &rhs) = delete;
    
    // Move-constructor
    Resource(Resource &&other) noexcept
        : Resource()
    {
        swap(*this, other);
    }
    
    template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, bool> = true>
    Resource(Resource<U> &&other) noexcept
        : Resource()
    {
        partialSwap(*this, other);
    }
    
    // Destructor
    ~Resource()
    {
        if (mControlBlock)
            mControlBlock->deleteResource();
        mPtr = nullptr;
        mControlBlock = nullptr;
    }
    
    // Copy-assignment. Not allowed.
    Resource& operator=(Resource &resource) = delete;
    
    // Move-assignment
    Resource& operator=(Resource &&resource) noexcept
    {
        swap(*this, resource);
        return *this;
    }
    
    friend void swap(Resource &lhs, Resource &rhs)
    {
        using std::swap;
        
        swap(lhs.mPtr, rhs.mPtr);
        swap(lhs.mControlBlock, rhs.mControlBlock);
    }
    
    template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, bool> = true>
    void partialSwap(Resource &lhs, Resource<U> &rhs)
    {
        using std::swap;
        
        lhs.mPtr = static_cast<T*>(rhs.mPtr);
        rhs.mPtr = nullptr;  // We can't guarantee a successful down-cast.
        swap(lhs.mControlBlock, rhs.mControlBlock);
    }
    
    T* operator->() { return mPtr; }
    const T* operator->() const { return mPtr; }
    [[nodiscard]] T* get() { return mPtr; }
    [[nodiscard]] const T* get() const { return mPtr; }
    
    [[nodiscard]] uint64_t typeHash() const { return mControlBlock->typeHash(); }

protected:
    T* mPtr { nullptr };
    ControlBlock* mControlBlock { nullptr };

private:
    template<typename> friend class Ref;
    template<typename> friend class Resource;
};

/**
 * @brief A reference to a resource. Use Ref\<T>::isValid() to check if the * resource has been destroyed.
 * @tparam T - The pointer type that you want to reference the resource with. This can point to a base type
 * of the derived type.
 * @see Resource\<T>
 */
template<typename T>
class Ref
{
public:
    friend void swap(Ref &lhs, Ref &rhs);
    
    explicit Ref() = default;  // Default constructor.
    
    // Default constructor from resource
    template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, bool> = true>
    Ref(const Resource<U> &other)
        : mPtr(static_cast<T*>(other.mPtr)), mControlBlock(other.mControlBlock)
    {
        incrementIfAble();
    }
    
    // Copy-constructor
    Ref(const Ref &other)
        : mPtr(other.mPtr), mControlBlock(other.mControlBlock)
    {
        incrementIfAble();
    }
    
    // Copy-constructor
    template<typename U, std::enable_if_t<std::is_convertible_v<U*, T*>, bool> = true>
    Ref(const Ref<U> &other)
        : mPtr(static_cast<T*>(other.mPtr)), mControlBlock(other.mControlBlock)
    {
        incrementIfAble();
    }
    
    
    // Destructor
    ~Ref()
    {
        decrementIfAble();
    }
    
    // Copy/move-assignment operator (with the move-assignment done by the compiler).
    Ref &operator=(Ref other)
    {
        swap(*this, other);
        return *this;
    }
    
    friend void swap(Ref &lhs, Ref &rhs)
    {
        using std::swap;
        
        swap(lhs.mPtr, rhs.mPtr);
        swap(lhs.mControlBlock, rhs.mControlBlock);
    }
    
    T* operator->() { return isValid() ? get() : throw InvalidReference(); }
    const T* operator->() const { return isValid() ? get() : throw InvalidReference(); }
    [[nodiscard]] T* get() { return mPtr; }
    [[nodiscard]] const T* get() const { return mPtr; }
    [[nodiscard]] uint64_t typeHash() const { return mControlBlock->typeHash(); }
    [[nodiscard]] bool isValid() const { return mControlBlock != nullptr && mControlBlock->isValid(); }

protected:
    void incrementIfAble()
    {
        if (mControlBlock)
            mControlBlock->incrementReference();
    }
    
    void decrementIfAble()
    {
        if (mControlBlock)
            mControlBlock->decrementReference();
    }
    
    T* mPtr { nullptr };
    ControlBlock* mControlBlock { nullptr };

private:
    template<typename> friend class Ref;
};

class InvalidReference
    : public std::exception
{
public:
    [[nodiscard]] const char *what() const override
    {
        return "The reference is trying to access a deleted resource pointer.";
    }
};