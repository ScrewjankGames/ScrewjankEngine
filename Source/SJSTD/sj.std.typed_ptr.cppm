module;

#include <ScrewjankStd/Assert.hpp>

export module sj.std.typed_ptr;
import sj.std.type_info;

export namespace sj
{
class typed_ptr
{
    template <class T>
    typed_ptr(T* ptr) : mPtr(ptr), mId(type_id_of<T>)
    {
    }

    typed_ptr(void* ptr, TypeId id) : mPtr(ptr), mId(id)
    {
    }

    bool is(TypeId typeId)
    {
        return mId == typeId;
    }

    template <class T>
    bool is()
    {
        return is(type_id_of<T>);
    }

    template <class T>
    T* as()
    {
        SJ_ASSERT(is<T>(), "Invalid typed_ptr cast");
        return static_cast<T*>(mPtr);
    }

private:
    void* mPtr;
    TypeId mId;
};
} // namespace sj