module;

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

export module sj.engine.core.Editor;
import sj.engine.core.Program;
import sj.engine.core.Window;
import sj.engine.rendering.Renderer;
import sj.engine.system.memory.MemorySystem;

export namespace sj
{
class Editor : public IModule
{
public:
    Editor(Program& program) : mProgram(&program), mRenderer(program.GetModule<Renderer>())
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        Window* display = program.GetModule<Window>();
        ImGui_ImplSDL3_InitForVulkan(display->GetWindowHandle());
        mRenderer->InitImGui();
    }

    ~Editor()
    {
        mRenderer->DeInitImGui();
    }

    void NewFrame() override
    {
        MemoryResourceScope _(MemorySystem::GetDebugMemoryResource());

        // Start the Dear ImGui frame
        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0,
                                     ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode);

        if(ImGui::Begin("Status"))
            ImGui::Text("timestep: %f", mProgram->GetDeltaSeconds());
        ImGui::End();
    }

    void EndFrame() override
    {
        ImGui::Render();
    }

private:
    Program* mProgram;
    Renderer* mRenderer;
};

} // namespace sj