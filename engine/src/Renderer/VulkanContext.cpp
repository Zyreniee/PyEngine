#include "PyEngine/Renderer/VulkanContext.hpp"
#include "PyEngine/Core/Assert.hpp"
#include "PyEngine/Core/Log.hpp"
#include "PyEngine/Platform/Window.hpp"
#include <cstring>
#include <set>

namespace PyEngine {

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  }
  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

VulkanContext::VulkanContext(Window &window) : m_Window(window) {
  CreateInstance();
  SetupDebugMessenger();
  m_Surface = m_Window.CreateVulkanSurface(m_Instance);
  PickPhysicalDevice();
  CreateLogicalDevice();
}

VulkanContext::~VulkanContext() {
  if (m_Device) {
    vkDestroyDevice(m_Device, nullptr);
  }

  if (m_Surface) {
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
  }

  if (m_EnableValidationLayers && m_DebugMessenger) {
    DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
  }

  if (m_Instance) {
    vkDestroyInstance(m_Instance, nullptr);
  }
}

void VulkanContext::CreateInstance() {
  if (m_EnableValidationLayers && !CheckValidationLayerSupport()) {
    PYENGINE_CORE_ERROR("Validation layers requested but not available!");
  }

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "PyEngine Application";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "PyEngine";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = GetRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (m_EnableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(m_ValidationLayers.size());
    createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

    debugCreateInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = DebugCallback;
    createInfo.pNext = &debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
  PYENGINE_CORE_ASSERT(result == VK_SUCCESS,
                       "Failed to create Vulkan instance!");
  PYENGINE_CORE_INFO("Vulkan instance created successfully");
}

void VulkanContext::SetupDebugMessenger() {
  if (!m_EnableValidationLayers)
    return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = DebugCallback;

  VkResult result = CreateDebugUtilsMessengerEXT(m_Instance, &createInfo,
                                                 nullptr, &m_DebugMessenger);
  PYENGINE_CORE_ASSERT(result == VK_SUCCESS,
                       "Failed to set up debug messenger!");
}

void VulkanContext::PickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
  PYENGINE_CORE_ASSERT(deviceCount > 0,
                       "Failed to find GPUs with Vulkan support!");

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

  // Prefer discrete GPU
  for (const auto &device : devices) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        IsDeviceSuitable(device)) {
      m_PhysicalDevice = device;
      m_GPUName = properties.deviceName;
      break;
    }
  }

  // Fallback to any suitable device
  if (m_PhysicalDevice == VK_NULL_HANDLE) {
    for (const auto &device : devices) {
      if (IsDeviceSuitable(device)) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        m_PhysicalDevice = device;
        m_GPUName = properties.deviceName;
        break;
      }
    }
  }

  PYENGINE_CORE_ASSERT(m_PhysicalDevice != VK_NULL_HANDLE,
                       "Failed to find suitable GPU!");
  PYENGINE_CORE_INFO("Selected GPU: {}", m_GPUName);
}

void VulkanContext::CreateLogicalDevice() {
  m_QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {
      m_QueueFamilyIndices.GraphicsFamily.value(),
      m_QueueFamilyIndices.PresentFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceFeatures.fillModeNonSolid = VK_TRUE;

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.pEnabledFeatures = &deviceFeatures;

  const std::vector<const char *> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  if (m_EnableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(m_ValidationLayers.size());
    createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  VkResult result =
      vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device);
  PYENGINE_CORE_ASSERT(result == VK_SUCCESS,
                       "Failed to create logical device!");

  vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.GraphicsFamily.value(), 0,
                   &m_GraphicsQueue);
  vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.PresentFamily.value(), 0,
                   &m_PresentQueue);

  PYENGINE_CORE_INFO("Logical device created successfully");
}

bool VulkanContext::CheckValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : m_ValidationLayers) {
    bool layerFound = false;
    for (const auto &layerProperties : availableLayers) {
      if (strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }
    if (!layerFound) {
      return false;
    }
  }
  return true;
}

std::vector<const char *> VulkanContext::GetRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  Window::GetRequiredInstanceExtensions(&glfwExtensionCount, &glfwExtensions);

  std::vector<const char *> extensions(glfwExtensions,
                                       glfwExtensions + glfwExtensionCount);

  if (m_EnableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

bool VulkanContext::IsDeviceSuitable(VkPhysicalDevice device) {
  QueueFamilyIndices indices = FindQueueFamilies(device);

  // Check for swap chain support
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       nullptr);
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                       availableExtensions.data());

  bool swapChainSupport = false;
  for (const auto &extension : availableExtensions) {
    if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
      swapChainSupport = true;
      break;
    }
  }

  return indices.IsComplete() && swapChainSupport;
}

QueueFamilyIndices VulkanContext::FindQueueFamilies(VkPhysicalDevice device) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto &queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.GraphicsFamily = i;
    }

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);
    if (presentSupport) {
      indices.PresentFamily = i;
    }

    if (indices.IsComplete()) {
      break;
    }
    i++;
  }

  return indices;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    PYENGINE_CORE_WARN("Validation layer: {}", pCallbackData->pMessage);
  }
  return VK_FALSE;
}

} // namespace PyEngine
