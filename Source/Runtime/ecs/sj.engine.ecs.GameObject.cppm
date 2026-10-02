module;

export module sj.engine.ecs.GameObject;
import sj.std;
import sj.engine.ecs.Identifiers;
import sj.engine.ecs.ECSRegistry;

export namespace sj
{
class GameObject
{
public:
    GameObject(GameObjectId id, ECSRegistry& r) : mGoId(id), mRegistry(&r)
    {
    }

    [[nodiscard]] mat44 GetTransformLW() const;
    void SetTransformLW(const mat44& m);

    template <class T>
    T* GetComponent()
    {
        return mRegistry->GetComponent<T>(mGoId);
    }

private:
    GameObjectId mGoId;
    ECSRegistry* mRegistry;
};
} // namespace sj