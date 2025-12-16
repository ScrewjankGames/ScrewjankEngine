module;

#include <cstddef>
#include <functional>
#include <memory>

export module sj.std.signal;
import sj.std.containers.map;
import sj.std.type_info;

export namespace sj
{

using signal_handle = size_t;

template <typename T>
class signal;

template <class R, class ... Args>
class signal<R(Args...)>
{
public:
    using slot_type = std::function<R(Args...)>;

    signal() = default;

    template <class Fn>
    signal_handle connect(Fn&& receiver)
    {
        signal_handle id = mHandleCounter++;
        mSlots[id] = std::forward<Fn>(receiver);
        return id;
    }

    void disconnect(signal_handle id)
    {
        mSlots.erase(id);
    }

    void emit(const Args&& ... args)
    {
        for(auto& slot : mSlots.values())
            slot(args ... );
    }

private:
    size_t mHandleCounter = 0;
    dynamic_flat_map<size_t, slot_type> mSlots;
};

} // namespace sj