#include <vulkan/vulkan_raii.hpp>

#include <cstring>
#include <iostream>
#include <cstdint>
#include <optional>
#include <utility>

class ComputeApplication
{
public:
    bool run() // run이라는 함수를 만들고, 이 함수는 참/거짓(bool)을 돌려준다. py : def run(self) -> bool:
    { // 03-physical-devices.md : ## 1.
        if (!initVulkan()) // Vulkan 초기화(initVulkan)를 해보고, 그게 실패했다면..
        {
            return false;
        }
        return true;
    }

private:
    static constexpr const char* validationLayer = "VK_LAYER_KHRONOS_validation";

    // 03-physical-devices.md : ## 2.
    // Context  →  Instance  →  DebugMessenger
    // (시작점)    (Context 필요)  (Instance 필요)
    vk::raii::Context context; // Vulkan을 쓰기 위한 시작점(Context)을 하나 만든다.
    vk::raii::Instance instance = nullptr; // Vulkan 인스턴스를 담을 자리를 만들어두는데, 지금은 비워둔다.
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr; // 디버그 메시지 전달자를 담을 자리를 만들어두는데, 이것도 지금은 비워둔다.
    
    // TODO: 물리적 디바이스와 큐 패밀리 인덱스 멤버를 추가합니다.
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    std::uint32_t computeQueueFamilyIndex = 0;
    
    bool pickPhysicalDevice()
    {
        // 03-physical-devices.md : ## 03.
        auto physicalDevices = instance.enumeratePhysicalDevices(); // GPU 목록 가져오기
        if (physicalDevices.empty())
        {
            std::cerr << "No Vulkan physical devices found.\n"; // cerr : character error
            return false;
        }
        for (auto& candidate : physicalDevices)
        {
            std::cerr << "No Vulkan 1.3 pyhsical device with a compute queue family found.\n";
            return false;
        }
    }

    bool initVulkan()
    {
        // TODO: pickPhysicalDevice()를 초기화 순서에 추가합니다.
        return createInstance() && setupDebugMessenger();
    }

    bool checkValidationLayerSupport()
    {
        const auto layers = context.enumerateInstanceLayerProperties(); // C API: vkEnumerateInstanceLayerProperties
        for (const auto& layer : layers)
        {
            if (std::strcmp(layer.layerName, validationLayer) == 0)
            {
                return true;
            }
        }
        std::cerr << "Required layer not found: " << validationLayer << '\n';
        return false;
    }

    bool createInstance()
    {
        if (!checkValidationLayerSupport())
        {
            return false;
        }

        const auto extensions = context.enumerateInstanceExtensionProperties(); // C API: vkEnumerateInstanceExtensionProperties
        bool debugUtilsAvailable = false;
        for (const auto& extension : extensions)
        {
            if (std::strcmp(extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
            {
                debugUtilsAvailable = true;
            }
        }
        if (!debugUtilsAvailable)
        {
            std::cerr << "Required instance extension not found: VK_EXT_debug_utils\n";
            return false;
        }

        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "03-physical-devices";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        const char* requiredExtensions[] = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
        auto debugCreateInfo = makeDebugMessengerCreateInfo();
        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = &validationLayer;
        createInfo.enabledExtensionCount = 1;
        createInfo.ppEnabledExtensionNames = requiredExtensions;
        createInfo.pNext = &debugCreateInfo;

        instance = vk::raii::Instance(context, createInfo); // C API: vkCreateInstance
        return true;
    }

    static vk::DebugUtilsMessengerCreateInfoEXT makeDebugMessengerCreateInfo()
    {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                   | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        createInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                               | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                               | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        createInfo.setPfnUserCallback(debugCallback);
        return createInfo;
    }

    bool setupDebugMessenger()
    {
        debugMessenger = instance.createDebugUtilsMessengerEXT(makeDebugMessengerCreateInfo()); // C API: vkCreateDebugUtilsMessengerEXT
        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void*)
    {
        std::cerr << "[debug "
                  << vk::to_string(static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity))
                  << ' ' << vk::to_string(vk::DebugUtilsMessageTypeFlagsEXT(type)) << "] "
                  << (callbackData->pMessageIdName ? callbackData->pMessageIdName : "unnamed")
                  << ": " << callbackData->pMessage << '\n';
        return VK_FALSE;
    }

    // TODO: findComputeQueueFamily(), pickPhysicalDevice(), printDeviceInfo()를 추가합니다.
};

int main()
{
    {
        ComputeApplication app;
        if (!app.run())
        {
            return 1;
        }
    }

    return 0;
}
