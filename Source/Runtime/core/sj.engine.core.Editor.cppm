module;

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

export module sj.engine.core.Editor;
import sj.engine.core.Window;
import sj.engine.rendering.Renderer;
import sj.engine.system.memory.MemorySystem;

export namespace sj
{
#ifndef SJ_GOLD

class Editor
{
public:
    Editor(Window& mainWindow, Renderer& renderer)
    {
        // // Setup Dear ImGui context
        // IMGUI_CHECKVERSION();
        // ImGui::CreateContext();
        // ImGuiIO& io = ImGui::GetIO();
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
        // // Setup Dear ImGui style
        // ImGui::StyleColorsDark();

        //ImGui_ImplGlfw_InitForVulkan(mainWindow.GetWindowHandle(), true);

        //auto vulkanInitInfo = renderer.GetImGuiInitInfo();
        //ImGui_ImplVulkan_Init(&vulkanInitInfo);
    }

    void Process(float deltaSeconds)
    {
        MemoryResourceScope _(MemorySystem::GetDebugMemoryResource());

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(0,
                                     ImGui::GetMainViewport(),
                                     ImGuiDockNodeFlags_PassthruCentralNode);

        if(ImGui::Begin("Status"))
            ImGui::Text("timestep: %f", deltaSeconds);
        ImGui::End();

        ImGui::Render();
    }

private:
};

#endif

} // namespace sj