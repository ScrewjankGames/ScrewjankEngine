module;

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <implot.h>

#include <optional>
#include <memory>

export module sj.editor:Editor;
import :IPanel;
import :AssetDrawer;

import sj.engine;
import sj.std;

export namespace sj
{
class Editor
{
public:
    Editor() = default;

    void Initialize(auto& program)
    {
        mRenderer = program.template GetModule<Renderer>();
        mWindow = program.template GetModule<Window>();

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

        mPanels.emplace_back(new AssetDrawer(mWindow));
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
        const bool captureMouse = ImGui::GetIO().WantCaptureMouse;
        const bool captureKeyboard = ImGui::GetIO().WantCaptureKeyboard;

        return captureMouse || captureKeyboard;
    }

    bool ProcessEvent(const PresentEvent& evt)
    {
        mPresentEvent = evt;
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
        MainMenuBar();

        for(std::unique_ptr<IPanel>& panel : mPanels)
        {
            if(ImGui::Begin(panel->GetName().data(), nullptr, ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_MenuBar))
                panel->Draw();
            ImGui::End();
        }

        if(ImGui::Begin("Scene Graph", nullptr, ImGuiWindowFlags_NoFocusOnAppearing))
            ImGui::Text("TODO");
        ImGui::End();

        if(ImGui::Begin("Property Editor", nullptr, ImGuiWindowFlags_NoFocusOnAppearing))
            ImGui::Text("TODO");
        ImGui::End();

        if(ImGui::Begin("Viewport"))
        {
            if(ImGui::IsWindowFocused())
            {
                ImGui::SetNextFrameWantCaptureKeyboard(false);
                ImGui::SetNextFrameWantCaptureMouse(false);
            }

            // Render game scene into window
            if(mPresentEvent)
            {
                ImGui::Image((ImTextureID)(intptr_t)mPresentEvent->image,
                             ImVec2(mPresentEvent->width, mPresentEvent->height));
                mPresentEvent = std::nullopt;
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
        mRenderer->RenderImGui(ImGui::GetDrawData());
    }

private:
    void MainMenuBar()
    {
        if(ImGui::BeginMainMenuBar())
        {
            if(ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("New Scene"))
                {
                }
                if(ImGui::MenuItem("Open", "Ctrl+O"))
                {
                }
                if(ImGui::MenuItem("Save", "Ctrl+S"))
                {
                }
                if(ImGui::MenuItem("Save As.."))
                {
                }
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("Edit"))
            {
                if(ImGui::MenuItem("Undo", "Ctrl+Z"))
                {
                }
                if(ImGui::MenuItem("Redo", "Ctrl+Y", false, false))
                {
                } // Disabled item
                ImGui::Separator();
                if(ImGui::MenuItem("Cut", "Ctrl+X"))
                {
                }
                if(ImGui::MenuItem("Copy", "Ctrl+C"))
                {
                }
                if(ImGui::MenuItem("Paste", "Ctrl+V"))
                {
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    sj::dynamic_vector<std::unique_ptr<IPanel>> mPanels;
    std::optional<PresentEvent> mPresentEvent;
    Renderer* mRenderer = nullptr;
    Window* mWindow = nullptr;
};

} // namespace sj