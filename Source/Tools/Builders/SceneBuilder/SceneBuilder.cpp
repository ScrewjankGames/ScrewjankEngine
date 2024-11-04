// Shared Includes
#include <ScrewjankShared/IO/File.hpp>
#include <ScrewjankShared/DataDefinitions/ScenePrototype.hpp>
#include <ScrewjankShared/DataDefinitions/GameObjectPrototype.hpp>
#include <ScrewjankShared/DataDefinitions/Components/CameraComponent.hpp>
#include <ScrewjankShared/DataDefinitions/Components/ScriptComponent.hpp>
#include <ScrewjankShared/utils/Assert.hpp>

// Library Includes
#include <nlohmann/json.hpp>

// STD Includes
#include <new>
#include <cstdio>
#include <vector>
#include <fstream>

using namespace sj;

struct ScriptComponentPrototype
{
    ScriptComponent component;
    nlohmann::json userData;
    size_t userDataSize;
};

template<class T>
void WriteComponentListHeader(File& outputFile, const std::vector<std::any>& components)
{
    ComponentListHeader header {T::kTypeId, static_cast<uint32_t>(components.size())};
    outputFile.WriteStruct(header);
}

template <class T>
void WriteComponentList(File& outputFile, const std::vector<std::any>& components)
{
    WriteComponentListHeader<T>(outputFile, components);

    for(const std::any& componentEntry : components)
    {
        T component = std::any_cast<T>(componentEntry);
        outputFile.WriteStruct(component);
    }
}

template<>
void WriteComponentList<ScriptComponentPrototype>(File& outputFile,
                                                  const std::vector<std::any>& components)
{
    WriteComponentListHeader<ScriptComponent>(outputFile, components);

    const size_t componentsStart = outputFile.CursorPos();
    const size_t componentsEnd = componentsStart + sizeof(ScriptComponent) * components.size();

    for(const std::any& componentEntry : components)
    {
        ScriptComponentPrototype proto =
            std::any_cast<ScriptComponentPrototype>(componentEntry);

        outputFile.WriteStruct(proto.component);
    }
}

Mat44 ExtractTransformFromJson(const nlohmann::json& json)
{
    std::array<float, 3> translationVec = json["translation"].get<std::array<float, 3>>();
    Vec4 t(translationVec[0], translationVec[1], translationVec[2], 1.0f);

    std::array<float, 3> scaleVec = json["scale"].get<std::array<float, 3>>();
    Vec4 s(scaleVec[0], scaleVec[1], scaleVec[2], 1.0f);

    std::array<float, 3> eulerVec = json["rotation"].get<std::array<float, 3>>();
    Vec3 eulers(eulerVec[0], eulerVec[1], eulerVec[2]);

    return BuildTransform(s, eulers, t);
}

using ComponentList = std::vector<std::any>;

CameraComponent BuildCameraComponent(GameObjectId gameobjectId, const nlohmann::json& component)
{
    return {gameobjectId,
            ExtractTransformFromJson(component),
            component["fov"],
            component["nearPlane"],
            component["farPlane"]};
}

ScriptComponentPrototype BuildScriptComponent(GameObjectId gameobjectId,
                                              const nlohmann::json& component)
{
    ScriptComponentPrototype proto;
    proto.component.ownerGameobjectId = gameobjectId;
    proto.component.scriptTypeId = StringHash(component["script_type"].get<std::string>().c_str()).AsInt();
    return proto;
}

void BuildComponent(GameObjectId gameobjectId, 
                    const nlohmann::json& componentJson,
                    std::map<uint32_t, ComponentList>& out_components)
{
    std::string componentType = componentJson["type"];

    StringHash componentTypeHash(componentType.c_str());
    uint32_t componentTypeId = componentTypeHash.AsInt();

    std::any component;
    switch(componentTypeId)
    {
    case CameraComponent::kTypeId:
        component = BuildCameraComponent(gameobjectId, componentJson);
        break;
    case ScriptComponent::kTypeId:
        component = BuildScriptComponent(gameobjectId, componentJson);
        break;
    default:
        //SJ_ASSERT(false, "Unkown component type %s found in scene", componentType.c_str());
        return;
    }

    auto it = out_components.find(componentTypeId);
    if(it == out_components.end())
    {
        out_components.emplace(
                componentTypeId, // Key
                ComponentList {component} // value 
        );
    }
    else
    {
        it->second.emplace_back(component);
    }
}

void BuildGameObject(uint32_t sceneId, 
                     const nlohmann::json& go,
                     std::vector<GameObjectPrototype>& out_goPrototypes,
                     std::map<uint32_t, ComponentList>& out_components)
{
    GameObjectPrototype prototype 
    {
        GameObjectId(sceneId, static_cast<uint32_t>(out_goPrototypes.size())),
        ExtractTransformFromJson(go)
    };

    if(go.contains("components"))
    {
        for(const nlohmann::json& component : go["components"])
        {
            BuildComponent(prototype.id, component, out_components);
        }
    }

    out_goPrototypes.emplace_back(prototype);
}

int main(int argc, char** argv)
{
    const char* inputFilePath = argv[1];
    const char* outputFilePath = argv[2];

    std::ifstream stream(inputFilePath);
    nlohmann::json document = nlohmann::json::parse(stream);

    ScenePrototype scenePrototype;
    scenePrototype.name = document["name"].get<std::string>().c_str();
    scenePrototype.memory = document["memory"].get<uint32_t>();

    std::vector<GameObjectPrototype> goPrototypes;
    std::map<uint32_t, ComponentList> components;
    if(document.contains("game_objects"))
    {
        auto goList = document["game_objects"];
        SJ_ASSERT(goList.is_array(), "Expected array of game objects");

        nlohmann::json gameobjects = document["game_objects"];
        for(const nlohmann::json& go : gameobjects)
        {
            BuildGameObject(scenePrototype.name.AsInt(), go, goPrototypes, components);
        }
    }

    scenePrototype.numGameObjects = static_cast<uint32_t>(goPrototypes.size());
    scenePrototype.numComponentLists = static_cast<uint32_t>(components.size());
    scenePrototype.scriptPoolSize = 0;
    if(components.find(ScriptComponent::kTypeId) != components.end())
    {
        scenePrototype.scriptPoolSize = static_cast<uint32_t>(components[ScriptComponent::kTypeId].size());
    }

    File outputFile;
    outputFile.Open(outputFilePath, File::OpenMode::kWriteBinary);
    outputFile.WriteStruct(scenePrototype);
    for(const GameObjectPrototype& goProto : goPrototypes)
    {
        outputFile.WriteStruct(goProto);
    }

    for(const auto& entry : components)
    {
        switch(entry.first)
        {
        case CameraComponent::kTypeId:
            WriteComponentList<CameraComponent>(outputFile, entry.second);
            break;
        case ScriptComponent::kTypeId:
            WriteComponentList<ScriptComponentPrototype>(outputFile, entry.second);
        default:
            break;
        }
    }

    return 0;
}