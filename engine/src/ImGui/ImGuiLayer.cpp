#include "PyEngine/ImGui/ImGuiLayer.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "PyEngine/Core/Assert.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/ImGui/IconsFontAwesome6.h"
#include "PyEngine/Platform/Window.hpp"
#include "PyEngine/Renderer/Renderer.hpp"
#include "PyEngine/Renderer/VulkanContext.hpp"
#include <filesystem>

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

    // Load custom fonts (Roboto + FontAwesome)
    // float fontSize = 16.0f;
    // bool fontsLoaded = false;
    
    // Bypassing custom TTF loading because Roboto-Regular.ttf causes an STB truetype assertion
    io.Fonts->AddFontDefault();

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

    // ── Rounding & Spacing (Unity exact match approach) ─────────────────────────
    style.WindowRounding = 0.0f;
    style.FrameRounding = 3.0f;     // Input fields and buttons
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ChildRounding = 0.0f;

    style.WindowPadding = ImVec2(4.0f, 4.0f);
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.ItemSpacing = ImVec2(4.0f, 3.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.IndentSpacing = 14.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.TabBorderSize = 1.0f;

    // ═══════════════════════════════════════════════════════════
    // Unity Dark Theme Palette
    // Unity standard dark theme colors (approximate)
    // Backgrounds: #383838, #282828
    // Accents: #2B5D87 (Blue highlight)
    // ═══════════════════════════════════════════════════════════
    ImVec4* c = style.Colors;

    // Backgrounds
    c[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);  // #1A1A1A
    c[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    c[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);  // #141414

    // UI elements background (Input fields, dropdowns)
    c[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f); 
    c[ImGuiCol_FrameBgActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);

    // Window formatting (Title bar and background matching)
    c[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    c[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

    // Unity Accent Blue: #3A72B0 (approx 0.227, 0.447, 0.690)
    ImVec4 unityBlue = ImVec4(0.227f, 0.447f, 0.690f, 1.0f);
    ImVec4 unityBlueHover = ImVec4(0.278f, 0.533f, 0.812f, 1.0f);
    ImVec4 unityBlueActive = ImVec4(0.188f, 0.380f, 0.592f, 1.0f);

    // Interactions
    c[ImGuiCol_Button] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);         // Unity button idle (#585858)
    c[ImGuiCol_ButtonHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.0f);  // Button hover  (#666666)
    c[ImGuiCol_ButtonActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);   // Button active (#474747)

    // Header (In Inspector/Hierarchy)
    c[ImGuiCol_Header] = ImVec4(0.16f, 0.16f, 0.16f, 1.0f); 
    c[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    c[ImGuiCol_HeaderActive] = unityBlue; // Selection background active

    // Tabs
    c[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);         // Non-selected tab (#2d2d2d)
    c[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);  // Tab hover
    c[ImGuiCol_TabActive] = c[ImGuiCol_WindowBg];                // Selected tab matches background
    c[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    c[ImGuiCol_TabUnfocusedActive] = c[ImGuiCol_WindowBg];

    // Borders
    c[ImGuiCol_Border] = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);  // Dark thin borders between panels (#212121)
    c[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Accents / Highlights
    c[ImGuiCol_NavHighlight] = unityBlue;
    c[ImGuiCol_CheckMark] = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);  // Off-white checkmark

    // Sliders
    c[ImGuiCol_SliderGrab] = ImVec4(0.40f, 0.40f, 0.40f, 1.0f);
    c[ImGuiCol_SliderGrabActive] = unityBlue;

    // Scrollbar (very subtle in Unity)
    c[ImGuiCol_ScrollbarBg] = c[ImGuiCol_WindowBg];
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
    c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);

    // Resizing
    c[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    c[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    c[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Text
    c[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);  // Off-white for readability
    c[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);
    c[ImGuiCol_TextSelectedBg] = ImVec4(unityBlue.x, unityBlue.y, unityBlue.z, 0.5f); // Text selection

    // Panels/Separators
    c[ImGuiCol_Separator] = ImVec4(0.14f, 0.14f, 0.14f, 1.0f); // Similar to border
    c[ImGuiCol_SeparatorHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    c[ImGuiCol_SeparatorActive] = unityBlue;

    // Docking
    c[ImGuiCol_DockingPreview] = ImVec4(unityBlue.x, unityBlue.y, unityBlue.z, 0.6f);
    c[ImGuiCol_DockingEmptyBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
}

}  // namespace PyEngine
