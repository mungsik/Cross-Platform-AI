#include <vulkan/vulkan_raii.hpp>

#include <iostream>

#include <cstring> // 문자열 비교

class ComputeApplication
{
public:
    void run()
    {
        initVulkan();
        // TODO: 초기화에 성공하면 확인용 메시지를 제출합니다.
    }

private:
    // TODO: validation layer 이름과 debug messenger 멤버를 추가합니다.
    // 02-validation-layer ## 1
    static constexpr const char* validationLayer = "VK_LAYER_KHRONOS_validation";

    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;

    void initVulkan()
    {
        createInstance();
        // TODO: 초기화 결과를 반환하고 debug messenger를 생성합니다.

    }

    // TODO: 레이어 지원 확인, 콜백, messenger 생성, 메시지 제출 함수를 추가합니다.
    bool checkValidationLayerSupport()
    {
        const auto layers = context.enumerateInstanceLayerProperties();
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

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void*)
    {
    }

    void createInstance()
    {
        // TODO: 레이어 지원 여부를 확인합니다.
        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "02-validation-layers";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        // TODO: 확장 목록에서 VK_EXT_debug_utils 지원 여부를 확인합니다.
        const auto extensions = context.enumerateInstanceExtensionProperties(); // C API: vkEnumerateInstanceExtensionProperties
        for (const auto& extension : extensions)
        {
            // TODO: VK_EXT_debug_utils와 이름이 일치하는지 확인합니다.
        }

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        // TODO: 레이어와 확장을 활성화하고 콜백 생성 정보를 연결합니다.

        instance = vk::raii::Instance(context, createInfo); // C API: vkCreateInstance
    }
};

int main()
{
    {
        ComputeApplication app;
        app.run();
    } // C API: vkDestroyInstance (app.instance 소멸자)

    std::cout << "Debug messenger and instance destroyed.\n";
    return 0;
}
