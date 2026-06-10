module;
#include <ScrewjankStd/Assert.hpp>

export module sj.engine.core.RenderSystem;
import sj.std;
import sj.datadefs.AssetDB;
import sj.engine.ecs.ECSRegistry;
import sj.engine.ecs.Identifiers;
import sj.engine.core.Mesh3DComponent;
import sj.engine.core.TransformComponent;
import sj.engine.rendering.Renderer;
import sj.engine.system.threading.ThreadContext;

export namespace sj
{
class RenderSystem
{
public:
    static constexpr type_list<Mesh3DComponent> kOwnedComponents;

    RenderSystem()
    {
    }

    void Initialize(Renderer* renderer)
    {
        mRenderer = renderer;
    }

    void OnCreate(GameObjectId goId, Mesh3DComponent* component)
    {
        mRenderer->AddMeshReference(component->model_id);

        if(component->texture_id != kInvalidAssetID)
            mRenderer->AddTextureReference(component->texture_id);
    }

    void OnDestroy(GameObjectId goId, Mesh3DComponent* component)
    {
        mRenderer->RemoveMeshReference(component->model_id);
        mRenderer->RemoveTextureReference(component->texture_id);
    }

    void Process(ECSRegistry& ecs, Renderer& renderer, const Mat44& cameraMatrix, float deltaTime)
    {
        auto components = ecs.GetComponents<Mesh3DComponent>();
        if(components.empty())
            return;

        scratchpad_scope scope = ThreadContext::GetScratchpad();
        sj::dynamic_vector<Renderer::MeshDrawArg> meshDrawArgs(&scope.get_allocator());

        for(const auto& [goId, mesh3D] : components)
        {
            sj::TransformComponent* goTransform = ecs.GetComponent<sj::TransformComponent>(goId);

            meshDrawArgs.emplace_back(
                Renderer::MeshDrawArg {.modelToWorld = goTransform->localToParentTransform,
                                        .modelId = mesh3D.model_id,
                                        .textureId = mesh3D.texture_id});
        }

        renderer.DrawPass(cameraMatrix, meshDrawArgs);
    }

private:
    Renderer* mRenderer = nullptr;
};
} // namespace sj