## 01. '!'를 먼저 쓰는 이유

나중에 단계가 늘어날 걸 대비한 구조라서 그래요. 튜토리얼이 진행되면 run()은 보통 이렇게 바뀌어요.

bool run()
{
    if (!initVulkan())       // 초기화 실패하면 중단
    {
        return false;
    }
    if (!runCompute())       // 계산 실패하면 중단
    {
        return false;
    }
    return true;             // 다 통과하면 성공
}

이렇게 **"실패하면 바로 나가기"**를 위에서부터 쌓으면 다음과 같은 장점이 있어요.
- 단계를 추가할 때 같은 모양의 if를 하나 더 붙이기만 하면 돼요.
- 성공한 흐름이 위에서 아래로 쭉 읽혀요.
- if 안에 if가 계속 들어가는 깊은 중첩이 생기지 않아요.

반대로 성공을 먼저 확인하는 방식(! 없이)으로 단계를 늘리면 이렇게 돼요.

if (initVulkan())
{
    if (runCompute())
    {
        return true;
    }
}
return false;

단계가 늘어날수록 들여쓰기가 점점 깊어져요.

## 02. context란?

### 02-1

먼저 알아둘 점: Vulkan 원래 기능이 아니에요

Vulkan 공식 C API에는 Context라는 개념이 없어요. vk::raii::Context는 Vulkan을 C++로 편하게 쓰도록 만든 Vulkan-Hpp 라이브러리가 따로 만든 클래스예요. 그래서 코드 주석에도 다른 함수들처럼 // C API: ...라는 대응 표시가 없어요.

Context가 하는 일

1. Vulkan 라이브러리를 불러와요

Vulkan 함수들은 내 프로그램 안에 들어있지 않아요. 컴퓨터에 설치된 Vulkan 로더 파일에 들어 있어요.
- 리눅스: libvulkan.so
- 윈도우: vulkan-1.dll

Context가 만들어질 때 이 파일을 찾아서 불러와요. Python으로 치면 이것과 비슷해요.
import ctypes
vulkan = ctypes.CDLL("libvulkan.so")   # 라이브러리 불러오기

2. Instance 없이도 쓸 수 있는 함수들을 제공해요

Vulkan 함수는 대부분 Instance가 있어야 호출할 수 있어요. 하지만 Instance를 만들기 전에 써야 하는 함수가 몇 개 있어요.

┌─────────────────────────────────┬────────────────────────────────────────┐
│        Context로 하는 일        │               C API 함수               │
├─────────────────────────────────┼────────────────────────────────────────┤
│ 사용 가능한 레이어 목록 확인    │ vkEnumerateInstanceLayerProperties     │
├─────────────────────────────────┼────────────────────────────────────────┤
│ 사용 가능한 확장 기능 목록 확인 │ vkEnumerateInstanceExtensionProperties │
├─────────────────────────────────┼────────────────────────────────────────┤
│ Instance 만들기                 │ vkCreateInstance                       │
└─────────────────────────────────┴────────────────────────────────────────┘

이 코드에서도 Context를 바로 이런 용도로 써요.
context.enumerateInstanceLayerProperties();      // 레이어 있는지 확인
context.enumerateInstanceExtensionProperties();  // 확장 기능 있는지 확인

3. 끝나면 라이브러리를 정리해요

RAII 객체라서, Context가 사라질 때 불러왔던 라이브러리도 자동으로 해제돼요.

비유하자면

Vulkan을 큰 건물이라고 생각해보세요.

- Context = 건물 입구의 안내데스크. 들어가기 전에 "어떤 시설(레이어, 확장 기능)이 있나요?"라고 물어볼 수 있어요.
- Instance = 안내데스크에서 받은 출입증. 이게 있어야 건물 안(GPU 찾기, 계산 작업 등)으로 들어갈 수 있어요.

그래서 순서가 항상 Context → Instance → 나머지가 돼요.

### 02-2. vk::raii::Context context; 코드 뜻

vk::raii::PhysicalDevice   physicalDevice   = nullptr;
└──────── 타입 ────────┘   └─ 변수 이름 ─┘   └ 초기값

std::uint32_t             computeQueueFamilyIndex = 0;  // 타입: 정수
vk::raii::Context         context;                      // 타입: Context 클래스
vk::raii::PhysicalDevice  physicalDevice = nullptr;     // 타입: PhysicalDevice 클래스
ComputeApplication        app;                          // 타입: 우리가 만든 클래스

┌─────────┬─────────────────────────────┬─────────────────────────────────────┐
│  부분   │            정체             │                 뜻                  │
├─────────┼─────────────────────────────┼─────────────────────────────────────┤
│ vk      │ 네임스페이스                │ Vulkan-Hpp 전체를 담은 큰 폴더      │
├─────────┼─────────────────────────────┼─────────────────────────────────────┤
│ raii    │ 네임스페이스 (vk 안에 있음) │ RAII 방식 클래스들을 담은 하위 폴더 │
├─────────┼─────────────────────────────┼─────────────────────────────────────┤
│ Context │ 클래스                      │ 객체를 찍어낼 설계도                │
├─────────┼─────────────────────────────┼─────────────────────────────────────┤
│ context │ 변수(객체)                  │ 그 설계도로 만든 실제 물건          │
└─────────┴─────────────────────────────┴─────────────────────────────────────┘

::는 "~ 안에 있는"이라는 뜻이고, 네임스페이스는 이름이 겹치지 않게 묶어두는 폴더 같은 거예요. Python의 패키지 경로와 비슷해요.

from vk.raii import Context   # vk 패키지 → raii 모듈 → Context 클래스
context = Context()           # 객체 생성

그래서 우리말로 하면

▎ "vk 네임스페이스 안의 raii 네임스페이스에 있는 Context 클래스로 context라는 객체를 하나 만든다."

## 03. auto

auto physicalDevices = instance.enumeratePhysicalDevices();
└타입┘ └─ 변수 이름 ─┘   └──────── 함수 호출 ────────┘

앞에서 본 타입 변수이름 = 값; 모양과 똑같아요.
- instance.enumeratePhysicalDevices()는 함수를 실행해서 GPU 목록을 가져와요.
- physicalDevices는 그 결과를 담아두는 변수예요.
- auto는 타입 자리에 들어간 거예요.

그러니까 auto가 줄여준 건 함수 이름이 아니라 타입 이름이에요. 원래 타입을 직접 쓰면 이렇게 길어요.

std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

auto라고 쓰면 컴파일러가 "오른쪽 함수가 돌려주는 값을 보니 이 타입이구나" 하고 알아서 채워줘요. 타입이 길거나 뻔할 때 많이 써요.