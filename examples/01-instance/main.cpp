#include <vulkan/vulkan_raii.hpp> // Vulkan-Hpp의 RAII 버전 헤더

#include <iostream> // "iostream 파일을 이 소스에 포함시켜라"

class ComputeApplication 
{
public:
    void run() // 반환값이 없음. '파이썬 : def run(self) -> None:'
    {
        initVulkan(); // 초기화 함수 호출
    }

private:
    // TODO: Context와 Instance 멤버를 추가합니다.
    // '::' 범위 지정 연산자. 범위(scope) A 안에서 B라는 이름을 찾아라
    // vk : Vulkan
    // RAII(C++의 설계 기법) : Resource Acquisition Is Initialization. 객체가 만들어질 때 자원을 얻고, 사라질 때 자원을 자동으로 해제한다는 뜻.
    vk::raii::Context context; // 타입 : vk::raii::Context, 이름 : context
    vk::raii::Instance instance = nullptr; // nullptr = 빈 상태. python의 None

    void initVulkan()
    {
        // TODO: createInstance()를 호출합니다.
        createInstance();

    }

    // TODO: createInstance() 함수를 추가하고 생성 정보를 채웁니다.
    void createInstance()
    {
        /*
        {}는 "모든 필드를 기본값으로 채워서 만들어라" 라는 뜻. 정수 필드는 0, 포인터 필드는 nullptr.

        int a{};      // a = 0
        int b{5};     // b = 5
        vk::ApplicationInfo appInfo{};   // 모든 필드가 기본값인 상태로 생성
        */

        vk::ApplicationInfo appInfo{}; // vk라는 네임 스페이스 안에 들어 있는 ApplicationInfo라는 타입(구조체) 

        // 프로그램 이름과 버전 // 
        appInfo.pApplicationName = "01-instance";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

        // 엔진 정보 //
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

        // Vulkan API 버전 지정 // 
        appInfo.apiVersion = VK_API_VERSION_1_3; // "appInfo라는 변수의 apiVersion 필드에 값을 넣어라"

        const auto extensions = context.enumerateInstanceExtensionProperties();

        // cout : character output. 즉, 화면 출력. #include <iostream>으로 가져온 것
        // << : 오른쪽 값을 왼쪽으로 보냄
        std::cout << "Available instance extensions:\n";

        /*
        & : 참조. 항목을 복사하지 않고 원본을 그대로 봄

        C++은 b = a를 하면 복사가 일어남. for문도 마찬가지라서 auto extension이라고만 쓰면 매번 항목(약 260바이트)을 복사함. &를 붙이면 복사 없이 원본을 가리킴. 그리고 출력만 할 거라 const로 "안 바꿈"을 표시함.
        목록을 읽기만 할 때는 'const auto&' 이라고 많이씀
        
        */ 
        for (const auto& extension : extensions)
        {
            std::cout << "  " << extension.extensionName << '\n';
        }

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;

        instance = vk::raii::Instance(context, createInfo);
        std::cout << "Instance created.\n";
    }
};

int main()
{
    {
        ComputeApplication app;
        app.run();
    } // C API: vkDestroyInstance (app.instance 소멸자)

    std::cout << "Instance destroyed.\n";
    return 0;
}
