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

    // ── Font Loading ─────────────────────────────────────────
    float fontSize = 15.0f;
    float iconSize = 14.0f;
    bool fontsLoaded = false;

    // Try to load Inter font (professional, Unity-like)
    auto interPath = std::filesystem::current_path() / "assets" / "fonts" / "Inter-Regular.ttf";
    auto interBoldPath = std::filesystem::current_path() / "assets" / "fonts" / "Inter-Bold.ttf";
    auto faPath = std::filesystem::current_path() / "assets" / "fonts" / "fa-solid-900.ttf";

    if (std::filesystem::exists(interPath)) {
        // Primary font: Inter Regular
        ImFontConfig fontConfig;
        fontConfig.OversampleH = 3;
        fontConfig.OversampleV = 2;
        fontConfig.PixelSnapH = true;
        io.Fonts->AddFontFromFileTTF(interPath.string().c_str(), fontSize, &fontConfig);

        // Merge FontAwesome icons into the default font
        if (std::filesystem::exists(faPath)) {
            ImFontConfig iconConfig;
            iconConfig.MergeMode = true;
            iconConfig.PixelSnapH = true;
            iconConfig.GlyphMinAdvanceX = iconSize;
            static const ImWchar icon_ranges[] = {0xf000, 0xf999, 0};
            io.Fonts->AddFontFromFileTTF(faPath.string().c_str(), iconSize, &iconConfig, icon_ranges);
        }

        // Bold font (index 1)
        if (std::filesystem::exists(interBoldPath)) {
            ImFontConfig boldConfig;
            boldConfig.OversampleH = 3;
            boldConfig.OversampleV = 2;
            io.Fonts->AddFontFromFileTTF(interBoldPath.string().c_str(), fontSize, &boldConfig);
        }

        fontsLoaded = true;
        PYENGINE_CORE_INFO("[ImGui] Loaded Inter font ({}px)", fontSize);
    }

    if (!fontsLoaded) {
        PYENGINE_CORE_WARN("[ImGui] Custom fonts not found, using default");
        io.Fonts->AddFontDefault();
    }

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
    style.FrameRounding = 3.0f;
    style.ScrollbarRounding = 2.0f;
    style.TabRounding = 2.0f;
    style.PopupRounding = 2.0f;
    style.ChildRounding = 2.0f;
    style.GrabRounding = 2.0f;

    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.IndentSpacing = 16.0f;
    style.ScrollbarSize = 12.0f;
    style.GrabMinSize = 8.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    style.TabBorderSize = 0.0f;

    // ═══════════════════════════════════════════════════════════
    // Unity 2024 Dark Theme — Professional Color Palette
    // ═══════════════════════════════════════════════════════════
    ImVec4* c = style.Colors;

    // Base backgrounds
    ImVec4 bgDark    = ImVec4(0.118f, 0.118f, 0.118f, 1.0f);   // #1E1E1E
    ImVec4 bgMid     = ImVec4(0.157f, 0.157f, 0.157f, 1.0f);   // #282828
    ImVec4 bgLight   = ImVec4(0.200f, 0.200f, 0.200f, 1.0f);   // #333333
    ImVec4 bgPopup   = ImVec4(0.137f, 0.137f, 0.137f, 1.0f);   // #232323

    // Accent
    ImVec4 accent    = ImVec4(0.259f, 0.588f, 0.980f, 1.0f);   // #4296FA
    ImVec4 accentHov = ImVec4(0.310f, 0.640f, 1.000f, 1.0f);
    ImVec4 accentAct = ImVec4(0.200f, 0.500f, 0.900f, 1.0f);

    // Text
    ImVec4 textMain  = ImVec4(0.860f, 0.860f, 0.860f, 1.0f);   // #DBDBDB
    ImVec4 textDim   = ImVec4(0.500f, 0.500f, 0.500f, 1.0f);

    // Backgrounds
    c[ImGuiCol_WindowBg]     = bgMid;
    c[ImGuiCol_ChildBg]      = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_PopupBg]      = bgPopup;

    // Frame (inputs, dropdowns)
    c[ImGuiCol_FrameBg]        = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    c[ImGuiCol_FrameBgActive]  = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

    // Title bars
    c[ImGuiCol_TitleBg]          = bgDark;
    c[ImGuiCol_TitleBgActive]    = bgDark;
    c[ImGuiCol_TitleBgCollapsed] = bgDark;
    c[ImGuiCol_MenuBarBg]        = bgDark;

    // Buttons
    c[ImGuiCol_Button]        = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    c[ImGuiCol_ButtonActive]  = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);

    // Headers
    c[ImGuiCol_Header]        = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);
    c[ImGuiCol_HeaderActive]  = accent;

    // Tabs
    c[ImGuiCol_Tab]               = bgDark;
    c[ImGuiCol_TabHovered]        = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);
    c[ImGuiCol_TabActive]         = bgMid;
    c[ImGuiCol_TabUnfocused]      = bgDark;
    c[ImGuiCol_TabUnfocusedActive]= bgMid;

    // Borders
    c[ImGuiCol_Border]       = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
    c[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Accents
    c[ImGuiCol_NavHighlight] = accent;
    c[ImGuiCol_CheckMark]    = accent;

    // Sliders
    c[ImGuiCol_SliderGrab]       = ImVec4(0.40f, 0.40f, 0.40f, 1.0f);
    c[ImGuiCol_SliderGrabActive] = accent;

    // Scrollbar
    c[ImGuiCol_ScrollbarBg]          = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.0f);
    c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);

    // Resize grip
    c[ImGuiCol_ResizeGrip]        = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_ResizeGripHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    c[ImGuiCol_ResizeGripActive]  = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Text
    c[ImGuiCol_Text]           = textMain;
    c[ImGuiCol_TextDisabled]   = textDim;
    c[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.4f);

    // Separator
    c[ImGuiCol_Separator]        = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
    c[ImGuiCol_SeparatorHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    c[ImGuiCol_SeparatorActive]  = accent;

    // Docking
    c[ImGuiCol_DockingPreview] = ImVec4(accent.x, accent.y, accent.z, 0.5f);
    c[ImGuiCol_DockingEmptyBg] = bgDark;
}

}  // namespace PyEngine
