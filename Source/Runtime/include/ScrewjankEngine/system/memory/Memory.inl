#include <ScrewjankEngine/system/memory/Memory.hpp>

namespace sj
{
    template<class T>
    UniquePtr<T>::UniquePtr() : m_Pointer(nullptr)
    {

    }

    template <class T>
    UniquePtr<T>::UniquePtr(T* ptr) : m_Pointer(ptr)
    {
    }

    template <class T>
    UniquePtr<T>::UniquePtr(UniquePtr&& other)
    {
        m_Pointer = other.m_Pointer;
        other.m_Pointer = nullptr;
    }

    template <class T>
    UniquePtr<T>::~UniquePtr()
    {
        CleanUp();
    }

    template <class T>
    void UniquePtr<T>::operator=(UniquePtr&& other) noexcept
    {
        CleanUp();

        m_Pointer = other.m_Pointer;
        other.m_Pointer = nullptr;
    }

    template <class T>
    T* UniquePtr<T>::Release() noexcept
    {
        auto ptr = m_Pointer;
        m_Pointer = nullptr;
        return ptr;
    }

    template <class T>
    void UniquePtr<T>::Reset(T* ptr)
    {
        CleanUp();
        m_Pointer = ptr;
    }

    template <class T>
    void UniquePtr<T>::CleanUp()
    {
        if (m_Pointer != nullptr)
        {
            delete m_Pointer;
        }
    }

    template <typename T, typename... Args>
    constexpr UniquePtr<T> MakeUnique(HeapZoneBase* zone, Args&&... args)
    {
        // Allocate the memory using the desired allocator
        auto memory = zone->New<T>(std::forward<Args>(args)...);

        //  Pass ownership of memory to the unique_ptr
        //  Supply a custom deletion function that uses the correct allocator
        return UniquePtr<T>(memory);
    }

}