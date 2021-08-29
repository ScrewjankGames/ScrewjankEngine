#include <iostream>

#ifndef ENGINE_ASSET_FOLDER
    #error "BuildTool Environment not set properly! Won't be able to find engine assets for all-build"
#endif // !ENGINE_ASSET_FOLDER

#ifndef GAME_ASSET_FOLDER
    #error "BuildTool Environment not set properly!
#endif // !GAME_ASSET_FOLDER

constexpr int kVersionMajor = 0;
constexpr int kVersionMinor = 1;

int main(int argc, char* argv[])
{
    std::cout << "Running BuildTool version " << kVersionMajor << "." << kVersionMinor << "\n";
    std::cout << "Build Environment: \n"
              << "\t Engine Asset Folder: " ENGINE_ASSET_FOLDER "\n"
              << "\t Game Asset Folder: " GAME_ASSET_FOLDER "\n";

    // If no args, build everything
    // Else strip file extension and call builders
}