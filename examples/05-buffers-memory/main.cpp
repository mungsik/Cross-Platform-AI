#include <vulkan/vulkan_raii.hpp>

// TODO: <array>와 <cstddef> 헤더를 추가합니다.
#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>
#include <utility>
#include <array>
#include <cstddef>

class ComputeApplication
{
public:
    bool run()
    {
        if (!initVulkan())
        {
            return false;
        }
        return true;
    }

private:
    static constexpr const char* validationLayer = "VK_LAYER_KHRONOS_validation";

    // TODO: 버퍼 크기와 BufferResource 구조체를 추가합니다.
    // 05-buffers-memory.md : # 01
    static_assert(sizeof(float) == 4);
    static constexpr std::uint32_t elementCount = 8;
    static constexpr vk::DeviceSize bufferSize = sizeof(float) * elementCount;

    struct BufferResource
    {
        vk::raii::DeviceMemory memory = nullptr;
        vk::raii::Buffer buffer = nullptr;
    };

    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
    vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    std::uint32_t computeQueueFamilyIndex = 0;
    vk::raii::Device device = nullptr;
    vk::raii::Queue computeQueue = nullptr;
    // TODO: inputA, inputB, output 멤버를 추가합니다.
    // 05-buffers-memory.md : ## 02
    BufferResource inputA;
    BufferResource inputB;
    BufferResource output;

    bool initVulkan()
    {
        // TODO: 마지막에 createStorageBuffers() 호출을 연결합니다.
        return createInstance()
            && setupDebugMessenger()
            && pickPhysicalDevice()
            && createLogicalDevice()
            && createStorageBuffers();
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
        appInfo.pApplicationName = "05-buffers-memory";
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

    bool pickPhysicalDevice()
    {
        auto physicalDevices = instance.enumeratePhysicalDevices(); // C API: vkEnumeratePhysicalDevices
        if (physicalDevices.empty())
        {
            std::cerr << "No Vulkan physical devices found.\n";
            return false;
        }

        for (auto& candidate : physicalDevices)
        {
            const auto properties = candidate.getProperties(); // C API: vkGetPhysicalDeviceProperties
            if (properties.apiVersion < VK_API_VERSION_1_3)
            {
                continue;
            }
            const auto queueFamily = findComputeQueueFamily(candidate);
            if (!queueFamily)
            {
                continue;
            }

            physicalDevice = std::move(candidate);
            computeQueueFamilyIndex = *queueFamily;
            return true;
        }
        std::cerr << "No Vulkan 1.3 physical device with a compute queue family found.\n";
        return false;
    }

    static std::optional<std::uint32_t> findComputeQueueFamily(
        const vk::raii::PhysicalDevice& candidate)
    {
        const auto queueFamilies = candidate.getQueueFamilyProperties(); // C API: vkGetPhysicalDeviceQueueFamilyProperties
        for (std::uint32_t index = 0; index < queueFamilies.size(); ++index)
        {
            const auto& family = queueFamilies[index];
            if (family.queueCount > 0 && (family.queueFlags & vk::QueueFlagBits::eCompute))
            {
                return index;
            }
        }
        return std::nullopt;
    }


    bool createLogicalDevice()
    {
        const float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.queueFamilyIndex = computeQueueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        vk::PhysicalDeviceFeatures enabledFeatures{};
        vk::DeviceCreateInfo createInfo{};
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.pEnabledFeatures = &enabledFeatures;

        device = vk::raii::Device(physicalDevice, createInfo); // C API: vkCreateDevice
        computeQueue = vk::raii::Queue(device, computeQueueFamilyIndex, 0); // C API: vkGetDeviceQueue
        return true;
    }
    // TODO: createStorageBuffers(), createBuffer(), findMemoryType(),
    // writeAndVerifyBuffer()를 추가합니다.
    bool createStorageBuffers() // 버퍼 생성
    {
        const std::array buffers = {&inputA, &inputB, &output};
        for (auto* buffer : buffers) // buffers 배열에 들어 있는 주소를 하나씩 꺼내서 buffer라는 포인터 변수에 담는다
        {
            if (!createBuffer(bufferSize, *buffer))
            {
                return false;
            }
        }

        /* GPU 입력값이 없으므로 테스트용 입력값 만들기*/
        std::array<float, elementCount> valuesA{};
        std::array<float, elementCount> valuesB{};
        const std::array<float, elementCount> initialOutput{};
        for (std::uint32_t i = 0; i < elementCount; ++i)
        {
            valuesA[i] = static_cast<float>(i);
            valuesB[i] = 10.0f + static_cast<float>(i);
        }
        /* 여기까지*/

        if (!writeAndVerifyBuffer(inputA, valuesA, "Input A")
            || !writeAndVerifyBuffer(inputB, valuesB, "Input B")
            || !writeAndVerifyBuffer(output, initialOutput, "Output (initial)"))
        {
            return false;
        }

        std::cout << "Host-visible storage buffers ready: " << buffers.size()
                  << " x " << bufferSize << " bytes.\n"
                  << "CPU write/readback verified. No GPU dispatch yet.\n";
        return true;
    }

    bool createBuffer(vk::DeviceSize size, BufferResource& resource)
    {
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = size;
        bufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer; // enumStorageBuffer. 셰이더가 읽고 쓸 수 있는 버퍼
        bufferInfo.sharingMode = vk::SharingMode::eExclusive; // sharingMode : 큐 패밀리에서 자원을 공유하는 방식

        resource.buffer = vk::raii::Buffer(device, bufferInfo); // device(GPU)에게 bufferInfo 조건대로 버퍼를 만들어 달라고 하고, 그걸 resource.buffer에 넣는다
        const auto requirements = resource.buffer.getMemoryRequirements(); // 이 버퍼를 올리려면 메모리가 얼마나, 어떤 조건으로 필요한지 드라이버에게 묻는다
        const auto properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent; // "CPU가 볼 수 있고(HostVisible), CPU가 쓴 내용이 자동으로 GPU와 맞춰지는(HostCoherent) 메모리를 원한다

        std::uint32_t memoryTypeIndex = 0;
        if (!findMemoryType(requirements.memoryTypeBits, properties, memoryTypeIndex))
        {
            return false;
        }

        vk::MemoryAllocateInfo allocationInfo{}; // 메모리 할당 요청서(allocationInfo)를 빈 상태(0)로 만든다
        allocationInfo.allocationSize = requirements.size; // 요청서에 '크기는 드라이버가 알려준 만큼' 이라고 적는다
        allocationInfo.memoryTypeIndex = memoryTypeIndex; // 요청서에 '메모리 타입은 방금 찾으 번호' 라고 적는다

        resource.memory = vk::raii::DeviceMemory(device, allocationInfo); // device에게 요청서대로 메모리를 할당해 달라고 하고, 그 메모리를 resource.memory에 넣는다
        resource.buffer.bindMemory(*resource.memory, 0); // 버퍼(설계도)를 방금 받은 메모리의 0번째 바이트 위치부터 연결한다

        std::cout << "Buffer memory: size " << requirements.size
                  << ", alignment " << requirements.alignment
                  << ", type " << memoryTypeIndex << '\n';
        return true;
    }

    bool findMemoryType(std::uint32_t typeFilter, vk::MemoryPropertyFlags properties, std::uint32_t& typeIndex)
    {
        const auto memoryProperties = physicalDevice.getMemoryProperties();
        for (std::uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
        {
            const bool allowed = (typeFilter & (1u << i)) != 0;
            const bool hasProperties = (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties;

            if (allowed && hasProperties)
            {
                typeIndex = i;
                return true;
            }
        }
        std::cerr << "No compatible HOST_VISIBLE | HOST_COHERENT memory type found.\n";
        return false;
    }

    bool writeAndVerifyBuffer(BufferResource& resource,
                              const std::array<float, elementCount>& values, // resource : 기록할 버퍼, values : 기록할 값
                              const char* label)
    {
        void* mapped = resource.memory.mapMemory(0, bufferSize); // GPU 버퍼의 CPU용 주소 받기
        std::memcpy(mapped, values.data(), static_cast<std::size_t>(bufferSize)); // memcpy = memory copy. CPU 배열 -> GPU 버퍼
        resource.memory.unmapMemory(); // C API: vkUnmapMemory

        std::array<float, elementCount> readback{};
        mapped = resource.memory.mapMemory(0, bufferSize); // C API: vkMapMemory
        std::memcpy(readback.data(), mapped, static_cast<std::size_t>(bufferSize));
        resource.memory.unmapMemory(); // C API: vkUnmapMemory

        if (readback != values)
        {
            std::cerr << label << ": CPU data verification failed.\n";
            return false;
        }

        std::cout << label << ':';
        for (float value : readback)
        {
            std::cout << ' ' << value;
        }
        std::cout << '\n';
        return true;
    }
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

    std::cout << "Buffers and memory destroyed.\n";
    return 0;
}
