module;

#include <fstream>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <ScrewjankStd/Assert.hpp>

export module sj.engine.rendering.vk.MeshBuffers;
import sj.datadefs.assets.Mesh;
import sj.engine.rendering.vk.Buffer;
import sj.engine.rendering.vk.ImmediateCommandContext;

export namespace sj::vk
{
    class MeshBuffers
    {
    public:
        void Init(const char* assetPath, VmaAllocator allocator, ImmediateCommandContext& ctx)
        {
            std::ifstream meshFile;
            meshFile.open(assetPath, std::ios::in | std::ios::binary);

            SJ_ASSERT(meshFile.is_open(), "Failed to load mesh file {}!", assetPath);

            MeshHeader header;
            meshFile.read(reinterpret_cast<char*>(&header), sizeof(header));

            // Detect Index Type from Index Size
            {
                switch(header.indexSize)
                {
                case sizeof(uint32_t):
                    m_indexType = VK_INDEX_TYPE_UINT32;
                    break;
                case sizeof(uint16_t):
                    m_indexType = VK_INDEX_TYPE_UINT16;
                case sizeof(uint8_t):
                    m_indexType = VK_INDEX_TYPE_UINT8;
                default:
                    m_indexType = VK_INDEX_TYPE_UINT32;
                    SJ_ASSERT(false, "Failed to deduce index type for asset {}", assetPath);
                    break;
                }
            }

            // Read verts into GPU memory
            {
                VkDeviceSize bufferSizeBytes = sizeof(MeshVertex) * header.numVerts;
                SJ_ASSERT(bufferSizeBytes < std::numeric_limits<std::streamsize>::max(),
                          "Buffer too large for single read");

                // Stage vertex data in host visible buffer
                sj::vk::BufferResource stagingBuffer =
                    sj::vk::MakeStagingBuffer(allocator, bufferSizeBytes);

                // Copy data from file to GPU
                meshFile.read(reinterpret_cast<char*>(stagingBuffer.GetMappedMemory()),
                              static_cast<std::streamsize>(bufferSizeBytes));

                m_vertexBuffer = sj::vk::BufferResource(allocator,
                                                        bufferSizeBytes,
                                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

                ctx.ImmediateSubmit([src = stagingBuffer.GetBuffer(),
                                     dst = m_vertexBuffer.GetBuffer(),
                                     bufferSizeBytes](VkCommandBuffer cmd) {
                    sj::vk::CopyBuffer(cmd, src, dst, bufferSizeBytes);
                });

                stagingBuffer.DeInit(allocator);
            }

            // Read Indices into GPU memory
            {
                m_indexCount = header.numIndices;

                VkDeviceSize bufferSizeBytes = header.indexSize * header.numIndices;
                SJ_ASSERT(bufferSizeBytes < std::numeric_limits<std::streamsize>::max(),
                          "Buffer too large for single read");
                sj::vk::BufferResource stagingBuffer =
                    sj::vk::MakeStagingBuffer(allocator, bufferSizeBytes);

                // Copy data from file to GPU
                meshFile.read(reinterpret_cast<char*>(stagingBuffer.GetMappedMemory()),
                              static_cast<std::streamsize>(bufferSizeBytes));

                m_indexBuffer = sj::vk::BufferResource(allocator,
                                                       bufferSizeBytes,
                                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                ctx.ImmediateSubmit([src = stagingBuffer.GetBuffer(),
                                     dst = m_indexBuffer.GetBuffer(),
                                     bufferSizeBytes](VkCommandBuffer cmd) {
                    sj::vk::CopyBuffer(cmd, src, dst, bufferSizeBytes);
                });

                stagingBuffer.DeInit(allocator);
            }

            meshFile.close();
        }

        void DeInit(VmaAllocator allocator)
        {
            m_indexBuffer.DeInit(allocator);
            m_vertexBuffer.DeInit(allocator);
        }

        [[nodiscard]] VkBuffer GetVertexBuffer() const
        {
            return m_vertexBuffer.GetBuffer();
        }

        [[nodiscard]] VkIndexType GetIndexType() const
        {
            return m_indexType;
        }

        [[nodiscard]] VkBuffer GetIndexBuffer() const
        {
            return m_indexBuffer.GetBuffer();
        }

        [[nodiscard]] uint32_t GetIndexCount() const
        {
            return m_indexCount;
        }

    private:
        BufferResource m_vertexBuffer;
        BufferResource m_indexBuffer;
        VkIndexType m_indexType = VK_INDEX_TYPE_UINT32;
        uint32_t m_indexCount = 0;
    };
} // namespace sj::vk