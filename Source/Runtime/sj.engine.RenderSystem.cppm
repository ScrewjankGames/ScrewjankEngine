module;
#include <ScrewjankStd/Assert.hpp>

export module sj.engine.RenderSystem;
import sj.std;
import sj.datadefs.AssetDB;
import sj.engine.ecs.ECSRegistry;
import sj.engine.ecs.Identifiers;
import sj.engine.Mesh3DComponent;
import sj.engine.DirectionalLightComponent;
import sj.engine.TransformComponent;
import sj.engine.rendering.Renderer;
import sj.engine.system.threading.ThreadContext;

export namespace sj
{
class RenderSystem
{
public:
    static constexpr type_list<Mesh3DComponent, DirectionalLightComponent> kRegisteredComponents;
    static constexpr type_list<Mesh3DComponent> kRequestedLifetimeCallbacks;

    RenderSystem() = default;

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

    void Process(ECSRegistry& ecs, Renderer& renderer, const mat44& cameraMatrix, float deltaTime)
    {
        scratchpad_scope scope = ThreadContext::GetScratchpad();
        sj::dynamic_vector<Renderer::MeshDrawArg> meshDrawArgs(&scope.get_allocator());
        
        auto drawables = ecs.Query<TransformComponent, Mesh3DComponent>();
        for(const auto&& [transform, mesh3D] : drawables)
        {
            meshDrawArgs.emplace_back(
                Renderer::MeshDrawArg {.modelToWorld = transform.localToParent,
                                       .modelId = mesh3D.model_id,
                                       .textureId = mesh3D.texture_id});
        }

        renderer.DrawPass(cameraMatrix, meshDrawArgs);
    }

private:
    Renderer* mRenderer = nullptr;
};
} // namespace sj