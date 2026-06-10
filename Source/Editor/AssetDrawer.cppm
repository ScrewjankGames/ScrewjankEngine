module;

#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>

#include <SDL3/SDL_dialog.h>
#include <cstddef>
#include <imgui.h>

#include <string_view>
#include <filesystem>
#include <random>

export module sj.editor:AssetDrawer;
import :IPanel;
import sj.engine.Window;

export namespace sj
{
class AssetDrawer : public IPanel
{
public:
    AssetDrawer(Window* window) : mWindow(window)
    {
    }

    ~AssetDrawer() override = default;

    std::string_view GetName() const override
    {
        return "Asset Drawer";
    }

    void Draw() override
    {
        if(ImGui::BeginMenuBar())
        {
            if(ImGui::Button("Import"))
            {
                SDL_ShowOpenFileDialog(ImportFileDialogCallback,
                                       this,
                                       mWindow->GetWindowHandle(),
                                       nullptr,
                                       0,
                                       "",
                                       false);
            }
            ImGui::EndMenuBar();
        }

        ImGui::Text("Stuff goes here");
    }

private:
    static void ImportFileDialogCallback(void* userdata, const char* const* filelist, int filter)
    {
        auto fileIt = filelist;

        while(*fileIt != nullptr)
        {
            SJ_ENGINE_LOG_INFO("Importing {}", *fileIt);

            ++fileIt;
        }
    }

    Window* mWindow = nullptr;
};
} // namespace sj