#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <string>
#include <vector>

namespace PyEngine {

class VulkanContext;

struct PipelineConfig {
    VkRenderPass RenderPass;
    uint32_t Subpass = 0;
    VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode PolygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace FrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    bool DepthTestEnable = true;
    bool DepthWriteEnable = true;
};

class Pipeline {
public:
    Pipeline(VulkanContext& context, const std::string& vertShaderPath, const std::string& fragShaderPath,
             VkPipelineLayout layout, const PipelineConfig& config);
    ~Pipeline();

    void Bind(VkCommandBuffer commandBuffer);
    VkPipeline GetPipeline() const { return m_Pipeline; }

private:
    VkShaderModule CreateShaderModule(const std::vector<char>& code);

private:
    VulkanContext& m_Context;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;
};

}  // namespace PyEngine
