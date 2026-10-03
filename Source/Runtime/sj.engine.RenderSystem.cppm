module;
#include <ScrewjankStd/Assert.hpp>

export module sj.engine.RenderSystem;
import sj.std;
import sj.engine.ecs;
import sj.datadefs.AssetDB;
import sj.engine.TransformComponent;
import sj.engine.debug.DebugDraw;
import sj.engine.rendering.Renderer;
import sj.engine.system.threading.ThreadContext;

export namespace sj
{
struct Mesh3DComponent
{
    AssetID model_id;
    AssetID texture_id;
};

struct DirectionalLightComponent
{
    vec4 dir;
    vec4 color;
};

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

    void OnCreate(GameObject go, Mesh3DComponent* component)
    {
        mRenderer->AddMeshReference(component->model_id);

        if(component->texture_id != kInvalidAssetID)
            mRenderer->AddTextureReference(component->texture_id);
    }

    void OnDestroy(GameObject go, Mesh3DComponent* component)
    {
        mRenderer->RemoveMeshReference(component->model_id);
        mRenderer->RemoveTextureReference(component->texture_id);
    }

    void Process(ECSRegistry& ecs, Renderer& renderer, const mat44& cameraMatrix, float deltaTime)
    {
        scratchpad_scope scope = ThreadContext::GetScratchpad();
        sj::dynamic_vector<Renderer::MeshDrawArg> meshDrawArgs(&scope);

        auto drawables = ecs.QueryWithIds<TransformComponent, Mesh3DComponent>();
        for(const auto&& [goId, transform, mesh3D] : drawables)
        {
            GameObject go(goId, ecs);
            meshDrawArgs.emplace_back(
                Renderer::MeshDrawArg {.modelToWorld = go.GetTransformLW(),
                                       .modelId = mesh3D.model_id,
                                       .textureId = mesh3D.texture_id});
        }

        renderer.ExecuteMainDrawPass(cameraMatrix, meshDrawArgs);
    }

private:
    Renderer* mRenderer = nullptr;
};
} // namespace sj