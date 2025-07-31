module;

export module sj.datadefs.ChunkTypes;
import game.datadefs;
import sj.datadefs.components;
import sj.std.containers.type_list;

export namespace sj
{
    using EngineComponentChunkTypes = type_list<
        TransformChunk, 
        CameraChunk,
        Mesh3DChunk
    >;
    using ComponentChunkTypeList = concat_type_lists<EngineComponentChunkTypes, GameComponentChunkTypes>::list;
    constexpr ComponentChunkTypeList g_componentChunkTypes;
}