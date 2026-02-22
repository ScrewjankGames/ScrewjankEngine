module;

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <implot.h>

#include <optional>

export module sj.engine.core.Editor;
import sj.engine.core.Program;
import sj.engine.core.Window;
import sj.engine.rendering.Renderer;
import sj.engine.system.memory.MemorySystem;

export namespace sj
{
class Editor
{
public:
    Editor() = default;

    void Initialize(auto& program)
    {
        mRenderer = program.template GetModule<Renderer>();

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        mRenderer->InitImGuiBackend();
    }

    ~Editor()
    {
        mRenderer->TeardownImGuiBackend();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    bool ProcessEvent(const SDL_Event& evt)
    {
        ImGui_ImplSDL3_ProcessEvent(&evt);
        return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
    }

    bool ProcessEvent(const RenderTarget& image)
    {
        if(true)
            return false;

        mViewportTexture = &image;
        return true;
    }

    void NewFrame()
    {
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0,
                                     ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode);
    }

    void Process(float deltaSeconds)
    {
        if(ImGui::BeginMainMenuBar())
        {
            ImGui::Button("File");
            ImGui::EndMainMenuBar();
        }

        if(ImGui::Begin("Scene Graph", nullptr, ImGuiWindowFlags_NoFocusOnAppearing))
            ImGui::Text("TODO");
        ImGui::End();

        if(ImGui::Begin("Property Editor", nullptr, ImGuiWindowFlags_NoFocusOnAppearing))
            ImGui::Text("TODO");
        ImGui::End();

        if(ImGui::Begin("Asset Drawer", nullptr, ImGuiWindowFlags_NoFocusOnAppearing))
            ImGui::Text("TODO");
        ImGui::End();

        if(ImGui::Begin("Viewport"))
        {
            // Render game scene into window
            if(mViewportTexture)
            {
                ImGui::Image((ImTextureID)(intptr_t)(SDL_GPUTexture*)mViewportTexture,
                             ImVec2(mViewportTexture->GetWidth(), mViewportTexture->GetHeight()));
                mViewportTexture = nullptr;
            }
        }
        ImGui::End();

        if(ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoFocusOnAppearing))
            ImGui::Text("timestep: %f", deltaSeconds);
        ImGui::End();
    }

    void EndFrame()
    {
        ImGui::Render();
        //mRenderer->RenderImGui(ImGui::GetDrawData());
    }

private:
    const RenderTarget* mViewportTexture;
    Renderer* mRenderer;
};

} // namespace sj