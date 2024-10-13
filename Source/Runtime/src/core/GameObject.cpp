// Parent Include
#include <ScrewjankEngine/core/GameObject.hpp>

namespace sj
{
    GameObject::GameObject(const GameObjectPrototype& proto) 
        : m_id(proto.id), m_transform(proto.transform)
    {

    }

} // namespace sj