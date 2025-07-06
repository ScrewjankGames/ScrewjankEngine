module;

export module sj.datadefs.ChunkTypes;
import game.datadefs;
import sj.datadefs.components;
import sj.std.containers;

export namespace sj
{
    using EngineComponentChunkTypes = type_list<TransformChunk, CameraChunk>;
    using ComponentChunkTypeList = concat_type_lists<EngineComponentChunkTypes, GameComponentChunkTypes>::list;
    constexpr ComponentChunkTypeList g_componentChunkTypes;
}