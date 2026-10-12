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

auto는 오른쪽 값을 보고 타입을 추측하는 문법이에요.

auto properties = candidate.getProperties();  // 오른쪽을 보면 타입을 알 수 있음 → OK

auto physicalDevices = instance.enumeratePhysicalDevices();
└타입┘ └─ 변수 이름 ─┘   └──────── 함수 호출 ────────┘

앞에서 본 타입 변수이름 = 값; 모양과 똑같아요.
- instance.enumeratePhysicalDevices()는 함수를 실행해서 GPU 목록을 가져와요.
- physicalDevices는 그 결과를 담아두는 변수예요.
- auto는 타입 자리에 들어간 거예요.

그러니까 auto가 줄여준 건 함수 이름이 아니라 타입 이름이에요. 원래 타입을 직접 쓰면 이렇게 길어요.

std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

auto라고 쓰면 컴파일러가 "오른쪽 함수가 돌려주는 값을 보니 이 타입이구나" 하고 알아서 채워줘요. 타입이 길거나 뻔할 때 많이 써요.

## 04. std::optional, const ... &
┌────────────────────────────────────┬────────────────────────────────────────────────────────────────┐
│                조각                │                               뜻                               │
├────────────────────────────────────┼────────────────────────────────────────────────────────────────┤
│ static                             │ 객체(this)의 멤버 변수를 쓰지 않는 함수예요. 받은 candidate만  │
│                                    │ 보고 일해요. 파이썬의 @staticmethod와 같아요.                  │
├────────────────────────────────────┼────────────────────────────────────────────────────────────────┤
│ std::optional<std::uint32_t>       │ 반환 타입이에요. "0 이상의 정수가 있거나, 없거나" 둘 중 하나를 │
│                                    │  돌려줘요.                                                     │
├────────────────────────────────────┼────────────────────────────────────────────────────────────────┤
│ findComputeQueueFamily             │ 함수 이름이에요. "컴퓨트 큐 패밀리 찾기"라는 뜻이에요.         │
├────────────────────────────────────┼────────────────────────────────────────────────────────────────┤
│ const vk::raii::PhysicalDevice&    │ 매개변수예요. 검사할 GPU 하나를 받아요.                        │
│ candidate                          │                                                                │
└────────────────────────────────────┴────────────────────────────────────────────────────────────────┘

std::optional

파이썬으로 치면 -> int | None이에요.

def find_compute_queue_family(candidate) -> int | None:
    ...
    return 2      # 찾음
    return None   # 못 찾음

C++에서는 int 자리에 None을 그냥 넣을 수 없어요. 그래서 "값이 없을 수도 있다"를 표현하려면 optional로 감싸야 해요. 못 찾았을 때는 return std::nullopt;라고 써요. nullopt가 파이썬의 None 역할이에요.

const ... &

- & (참조): GPU 객체를 복사하지 않고 원본을 빌려서 봐요.
- const: 빌려 보기만 하고 바꾸지 않겠다는 약속이에요.
- 합치면 "읽기 전용으로 빌려 보기"예요. 파이썬은 원래 객체를 이렇게 넘기지만, C++은 아무것도 안 붙이면 통째로 복사하기 때문에 이렇게 명시해 줘요.

정리

"GPU 하나를 읽기 전용으로 받아서, 컴퓨트 명령을 처리할 수 있는 큐 패밀리의 번호를 찾으면 그 번호를, 못 찾으면 '없음'을 돌려주는 함수"예요.

## 05. std::move()
 physicalDevice = std::move(candidate);: 후보 GPU를 멤버 변수로 옮겨 담아요. 복사 대신 이사를 시키는 거예요. 반복문이 끝나면 physicalDevices 목록은 사라지지만, 옮겨 둔 physicalDevice는 남아요.

  너가 말한 이 부분 좀 더 자세히 설명해줘

자세히 설명할게요. 그 전에 제가 한 말 중 하나를 바로잡을게요. physicalDevices 목록은 반복문이 끝날 때가 아니라 pickPhysicalDevice() 함수가 끝날 때 사라져요. 함수 안에서 만든 지역 변수라서요.

1. 왜 멤버 변수에 담아야 하나: 수명

bool pickPhysicalDevice()
{
    auto physicalDevices = ...;   // 지역 변수: 함수 안에서 태어남
    for (auto& candidate : physicalDevices) { ... }
    return ...;
}                                 // ← 여기서 physicalDevices와 안의 GPU들이 전부 정리됨

C++은 { } 블록이 끝나면 그 안에서 만든 변수를 즉시 정리해요.
파이썬도 함수가 끝나면 지역 변수는 사라지잖아요. 다만 파이썬은 다른 곳에서 참조하고 있으면 객체가 살아남지만, C++은 블록 끝에서 무조건 정리돼요.

그래서 고른 GPU를 함수가 끝난 뒤에도 쓰려면, 함수 밖에 있는 멤버 변수 physicalDevice(32줄)로 옮겨 놔야 해요. 파이썬으로 치면 self.physical_device = candidate예요.

2. = 의 차이: 파이썬 vs C++

self.physical_device = candidate   # 파이썬: 같은 객체에 이름표 하나 더 붙이기

physicalDevice = candidate;              // C++: 내용을 복사해서 새로 하나 만들기
physicalDevice = std::move(candidate);   // C++: 내용을 옮기기 (이사)

파이썬의 =는 항상 "이름표 붙이기"예요. 반면 C++의 =는 기본이 복사예요. 그래서 "복사하지 말고 옮겨라"라고 하고 싶을 때 std::move를 써요.

3. 옮기면(move) 무슨 일이 생기나

move 전                         move 후
candidate      → [GPU 핸들]     candidate      → (빈 상태, nullptr)
physicalDevice → (비어 있음)     physicalDevice → [GPU 핸들]

- 내용물(GPU 핸들)이 candidate에서 physicalDevice로 넘어가요.
- candidate는 빈 껍데기가 돼요. 그러니 move 다음에는 candidate를 쓰면 안 돼요. 바로 return true;로 끝내니까 이 코드에서는 문제없어요.
- 함수가 끝나면서 목록이 정리될 때 빈 껍데기만 정리되고, 진짜 핸들은 physicalDevice에 안전하게 남아요.

4. 솔직하게: 여기서는 복사해도 동작해요

앞에서 헤더를 확인했을 때 vk::raii::PhysicalDevice는 복사도 허용되고 크기도 작았어요. 그래서 이 줄은 physicalDevice = candidate;로 써도 돌아가요.

그래도 move를 쓰는 이유는 이거예요.
- 습관: vk::raii::Instance, vk::raii::Device 같은 다른 RAII 객체는 복사가 금지돼 있어서 move만 돼요. 이 튜토리얼 전체에서 같은 방식으로 쓰려는 거예요.
- 의도 표현: "이 후보는 이제 쓸 일이 없으니 멤버로 넘긴다"는 뜻이 코드에 드러나요.

▎ 한 줄 요약: 함수가 끝나면 사라질 후보 GPU를, 계속 살아 있는 멤버 변수로 이사시키는 것이에요.

✻ Churned for 19s · done 1:06 PM

❯ 왜 굳이 다른 객체로 옮기는거지? 그냥 candidate 계속 쓰면 안돼?
⧉ Selected 1 lines from main.cpp in Visual Studio Code

안 돼요. candidate는 pickPhysicalDevice() 안에서만 존재하는 이름이기 때문이에요.

나중에 다른 함수들도 이 GPU가 필요해요.
- printDeviceInfo(): 고른 GPU 정보 출력 (이번 장)
- 다음 장들: 논리 장치(Device) 만들기, 메모리 할당하기 등

그런데 이 함수들에서는 candidate가 보이지 않아요.

class App:
    def pick_physical_device(self):
        for candidate in devices:
            ...
            return True         # candidate는 여기서 끝

    def print_device_info(self):
        print(candidate)        # ❌ NameError: 다른 함수의 지역 변수
        print(self.physical_device)  # ✅ 멤버 변수는 어디서나 보임

C++도 똑같아요.
- candidate: 반복문 안에서만 쓰는 임시 이름 (지역 변수)
- physicalDevice: 클래스 전체가 같이 쓰는 보관함 (멤버 변수)

그래서 "후보 중에 합격한 걸 보관함에 넣어 둔다"는 게 physicalDevice = std::move(candidate);예요. 같은 일을 하는 computeQueueFamilyIndex = *queueFamily;도 지역 변수 queueFamily를 멤버 변수로 옮겨 담는 거예요.