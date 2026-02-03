module;

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

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
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        mRenderer->InitImGUI();
    }

    ~Editor() = default;

    void NewFrame()
    {

    }

    void Process(float deltaSeconds)
    {
        if(ImGui::Begin("Status"))
            ImGui::Text("timestep: %f", deltaSeconds);
        ImGui::End();
    }

    void EndFrame()
    {
    }

private:
    Renderer* mRenderer;
};

} // namespace sj