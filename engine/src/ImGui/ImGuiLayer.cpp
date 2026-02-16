#include "PyEngine/ImGui/ImGuiLayer.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "PyEngine/Core/Assert.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Platform/Window.hpp"
#include "PyEngine/Renderer/Renderer.hpp"
#include "PyEngine/Renderer/VulkanContext.hpp"

namespace PyEngine {

ImGuiLayer::ImGuiLayer(Window& window, VulkanContext& context, Renderer& renderer)
    : m_Window(window), m_Context(context), m_Renderer(renderer) {
    // Create descriptor pool for ImGui
    VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = 11;
    pool_info.pPoolSizes = pool_sizes;

    VkResult result = vkCreateDescriptorPool(m_Context.GetDevice(), &pool_info, nullptr, &m_DescriptorPool);
    PYENGINE_CORE_ASSERT(result == VK_SUCCESS, "Failed to create ImGui descriptor pool!");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    SetupImGuiStyle();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(m_Window.GetNativeWindow(), true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_Context.GetInstance();
    init_info.PhysicalDevice = m_Context.GetPhysicalDevice();
    init_info.Device = m_Context.GetDevice();
    init_info.QueueFamily = m_Context.GetQueueFamilies().GraphicsFamily.value();
    init_info.Queue = m_Context.GetGraphicsQueue();
    init_info.DescriptorPool = m_DescriptorPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    // Note: This is a simplified version that doesn't create a separate render
    // pass for ImGui In production, you might want a separate render pass
    ImGui_ImplVulkan_Init(&init_info, m_Renderer.GetRenderPass());

    // Upload Fonts
    VkCommandPool command_pool = VK_NULL_HANDLE;
    VkCommandPoolCreateInfo pool_info_cmd = {};
    pool_info_cmd.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info_cmd.queueFamilyIndex = m_Context.GetQueueFamilies().GraphicsFamily.value();
    vkCreateCommandPool(m_Context.GetDevice(), &pool_info_cmd, nullptr, &command_pool);

    VkCommandBuffer command_buffer;
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;
    vkAllocateCommandBuffers(m_Context.GetDevice(), &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);

    ImGui_ImplVulkan_CreateFontsTexture();

    vkEndCommandBuffer(command_buffer);
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    vkQueueSubmit(m_Context.GetGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_Context.GetGraphicsQueue());

    ImGui_ImplVulkan_DestroyFontsTexture();
    vkFreeCommandBuffers(m_Context.GetDevice(), command_pool, 1, &command_buffer);
    vkDestroyCommandPool(m_Context.GetDevice(), command_pool, nullptr);

    PYENGINE_CORE_INFO("ImGui layer initialized");
}

ImGuiLayer::~ImGuiLayer() {
    vkDeviceWaitIdle(m_Context.GetDevice());

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_DescriptorPool) {
        vkDestroyDescriptorPool(m_Context.GetDevice(), m_DescriptorPool, nullptr);
    }
}

void ImGuiLayer::Begin() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::End() {
    ImGui::Render();
}

void ImGuiLayer::RenderDrawData(VkCommandBuffer commandBuffer) {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void ImGuiLayer::SetupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    // ── Rounding & Spacing ─────────────────────────────────────
    style.WindowRounding = 2.0f;
    style.FrameRounding = 2.0f;
    style.GrabRounding = 2.0f;
    style.ScrollbarRounding = 4.0f;
    style.TabRounding = 2.0f;
    style.ChildRounding = 2.0f;
    style.PopupRounding = 3.0f;

    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.IndentSpacing = 18.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 8.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;
    style.WindowMenuButtonPosition = ImGuiDir_None;

    // ═══════════════════════════════════════════════════════════
    // PyEngine BLACK Theme — True black backgrounds
    // Inspired by Unity Pro dark, Unreal, and Godot dark themes
    // Accent: cyan-blue (#2D8CFF / #40B0FF) — PyEngine signature
    // ═══════════════════════════════════════════════════════════
    ImVec4* c = style.Colors;

    // ── Base backgrounds — TRUE BLACK ────────────────────────
    c[ImGuiCol_WindowBg] = ImVec4(0.051f, 0.051f, 0.055f, 1.000f);  // #0D0D0E — pitch black
    c[ImGuiCol_ChildBg] = ImVec4(0.051f, 0.051f, 0.055f, 1.000f);
    c[ImGuiCol_PopupBg] = ImVec4(0.078f, 0.078f, 0.086f, 0.980f);  // #141416

    // ── Borders — subtle, almost invisible ───────────────────
    c[ImGuiCol_Border] = ImVec4(0.140f, 0.140f, 0.155f, 0.600f);
    c[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);

    // ── Frames (input bg, checkbox bg) — very dark gray ──────
    c[ImGuiCol_FrameBg] = ImVec4(0.098f, 0.098f, 0.106f, 1.000f);  // #191A1B
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.130f, 0.130f, 0.142f, 1.000f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.165f, 0.165f, 0.180f, 1.000f);

    // ── Title bars — near black ──────────────────────────────
    c[ImGuiCol_TitleBg] = ImVec4(0.035f, 0.035f, 0.039f, 1.000f);        // #090A0A
    c[ImGuiCol_TitleBgActive] = ImVec4(0.055f, 0.055f, 0.063f, 1.000f);  // #0E0E10
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.035f, 0.035f, 0.039f, 0.750f);

    // ── Menu bar — black ─────────────────────────────────────
    c[ImGuiCol_MenuBarBg] = ImVec4(0.067f, 0.067f, 0.075f, 1.000f);  // #111113

    // ── Scrollbar ────────────────────────────────────────────
    c[ImGuiCol_ScrollbarBg] = ImVec4(0.051f, 0.051f, 0.055f, 0.400f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.220f, 0.220f, 0.240f, 1.000f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.310f, 0.310f, 0.340f, 1.000f);
    c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.400f, 0.400f, 0.440f, 1.000f);

    // ── Checkmark — PyEngine cyan accent ─────────────────────
    c[ImGuiCol_CheckMark] = ImVec4(0.180f, 0.550f, 1.000f, 1.000f);  // #2D8CFF

    // ── Slider ───────────────────────────────────────────────
    c[ImGuiCol_SliderGrab] = ImVec4(0.180f, 0.550f, 1.000f, 0.800f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.250f, 0.690f, 1.000f, 1.000f);  // #40B0FF

    // ── Buttons — dark with cyan hover ───────────────────────
    c[ImGuiCol_Button] = ImVec4(0.130f, 0.130f, 0.145f, 1.000f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.180f, 0.440f, 0.780f, 0.800f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.140f, 0.380f, 0.680f, 1.000f);

    // ── Headers (tree nodes, collapsing headers) ─────────────
    c[ImGuiCol_Header] = ImVec4(0.120f, 0.120f, 0.135f, 0.700f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.180f, 0.440f, 0.780f, 0.500f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.180f, 0.440f, 0.780f, 0.750f);

    // ── Separator ────────────────────────────────────────────
    c[ImGuiCol_Separator] = ImVec4(0.140f, 0.140f, 0.155f, 0.500f);
    c[ImGuiCol_SeparatorHovered] = ImVec4(0.180f, 0.550f, 1.000f, 0.700f);
    c[ImGuiCol_SeparatorActive] = ImVec4(0.180f, 0.550f, 1.000f, 1.000f);

    // ── Resize grip ──────────────────────────────────────────
    c[ImGuiCol_ResizeGrip] = ImVec4(0.180f, 0.550f, 1.000f, 0.150f);
    c[ImGuiCol_ResizeGripHovered] = ImVec4(0.180f, 0.550f, 1.000f, 0.600f);
    c[ImGuiCol_ResizeGripActive] = ImVec4(0.180f, 0.550f, 1.000f, 0.900f);

    // ── Tabs (docked windows) — black inactive, dark active ──
    c[ImGuiCol_Tab] = ImVec4(0.067f, 0.067f, 0.075f, 1.000f);  // #111113
    c[ImGuiCol_TabHovered] = ImVec4(0.180f, 0.440f, 0.780f, 0.600f);
    c[ImGuiCol_TabActive] = ImVec4(0.098f, 0.098f, 0.110f, 1.000f);  // #191A1C — subtle
    c[ImGuiCol_TabUnfocused] = ImVec4(0.051f, 0.051f, 0.055f, 1.000f);
    c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.082f, 0.082f, 0.090f, 1.000f);

    // ── Docking ──────────────────────────────────────────────
    c[ImGuiCol_DockingPreview] = ImVec4(0.180f, 0.550f, 1.000f, 0.600f);
    c[ImGuiCol_DockingEmptyBg] = ImVec4(0.039f, 0.039f, 0.043f, 1.000f);  // #0A0A0B

    // ── Text — bright on black ───────────────────────────────
    c[ImGuiCol_Text] = ImVec4(0.900f, 0.900f, 0.920f, 1.000f);  // near white
    c[ImGuiCol_TextDisabled] = ImVec4(0.420f, 0.420f, 0.450f, 1.000f);

    // ── Drag & drop ──────────────────────────────────────────
    c[ImGuiCol_DragDropTarget] = ImVec4(0.250f, 0.690f, 1.000f, 0.900f);  // bright cyan

    // ── Nav highlight ────────────────────────────────────────
    c[ImGuiCol_NavHighlight] = ImVec4(0.180f, 0.550f, 1.000f, 1.000f);
    c[ImGuiCol_NavWindowingHighlight] = ImVec4(1.000f, 1.000f, 1.000f, 0.700f);
    c[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.500f);

    // ── Modal dim — very dark ────────────────────────────────
    c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.600f);

    // ── Table ────────────────────────────────────────────────
    c[ImGuiCol_TableHeaderBg] = ImVec4(0.082f, 0.082f, 0.090f, 1.000f);
    c[ImGuiCol_TableBorderStrong] = ImVec4(0.180f, 0.180f, 0.200f, 1.000f);
    c[ImGuiCol_TableBorderLight] = ImVec4(0.130f, 0.130f, 0.145f, 1.000f);
    c[ImGuiCol_TableRowBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
    c[ImGuiCol_TableRowBgAlt] = ImVec4(1.000f, 1.000f, 1.000f, 0.030f);

    // ── Plot ─────────────────────────────────────────────────
    c[ImGuiCol_PlotLines] = ImVec4(0.180f, 0.550f, 1.000f, 1.000f);
    c[ImGuiCol_PlotLinesHovered] = ImVec4(0.250f, 0.690f, 1.000f, 1.000f);
    c[ImGuiCol_PlotHistogram] = ImVec4(0.180f, 0.550f, 1.000f, 0.800f);
    c[ImGuiCol_PlotHistogramHovered] = ImVec4(0.250f, 0.690f, 1.000f, 1.000f);

    // ── Text selection ───────────────────────────────────────
    c[ImGuiCol_TextSelectedBg] = ImVec4(0.180f, 0.440f, 0.780f, 0.350f);
}

}  // namespace PyEngine
