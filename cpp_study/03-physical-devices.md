## 01. '!'를 먼저 쓰는 이유

나중에 단계가 늘어날 걸 대비한 구조라서 그래요. 튜토리얼이 진행되면 `run()`은 보통 이렇게 바뀌어요.

```cpp
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
```

이렇게 **"실패하면 바로 나가기"**를 위에서부터 쌓으면 다음과 같은 장점이 있어요.
- 단계를 추가할 때 같은 모양의 `if`를 하나 더 붙이기만 하면 돼요.
- 성공한 흐름이 위에서 아래로 쭉 읽혀요.
- `if` 안에 `if`가 계속 들어가는 깊은 중첩이 생기지 않아요.

반대로 성공을 먼저 확인하는 방식(`!` 없이)으로 단계를 늘리면 이렇게 돼요.

```cpp
if (initVulkan())
{
    if (runCompute())
    {
        return true;
    }
}
return false;
```

단계가 늘어날수록 들여쓰기가 점점 깊어져요.

## 02. context란?

### 02-1

#### 먼저 알아둘 점: Vulkan 원래 기능이 아니에요

Vulkan 공식 C API에는 Context라는 개념이 없어요. `vk::raii::Context`는 Vulkan을 C++로 편하게 쓰도록 만든 Vulkan-Hpp 라이브러리가 따로 만든 클래스예요. 그래서 코드 주석에도 다른 함수들처럼 `// C API: ...`라는 대응 표시가 없어요.

#### Context가 하는 일

**1. Vulkan 라이브러리를 불러와요**

Vulkan 함수들은 내 프로그램 안에 들어있지 않아요. 컴퓨터에 설치된 Vulkan 로더 파일에 들어 있어요.
- 리눅스: `libvulkan.so`
- 윈도우: `vulkan-1.dll`

Context가 만들어질 때 이 파일을 찾아서 불러와요. Python으로 치면 이것과 비슷해요.

```python
import ctypes
vulkan = ctypes.CDLL("libvulkan.so")   # 라이브러리 불러오기
```

**2. Instance 없이도 쓸 수 있는 함수들을 제공해요**

Vulkan 함수는 대부분 Instance가 있어야 호출할 수 있어요. 하지만 Instance를 만들기 전에 써야 하는 함수가 몇 개 있어요.

| Context로 하는 일 | C API 함수 |
|---|---|
| 사용 가능한 레이어 목록 확인 | `vkEnumerateInstanceLayerProperties` |
| 사용 가능한 확장 기능 목록 확인 | `vkEnumerateInstanceExtensionProperties` |
| Instance 만들기 | `vkCreateInstance` |

이 코드에서도 Context를 바로 이런 용도로 써요.

```cpp
context.enumerateInstanceLayerProperties();      // 레이어 있는지 확인
context.enumerateInstanceExtensionProperties();  // 확장 기능 있는지 확인
```

**3. 끝나면 라이브러리를 정리해요**

RAII 객체라서, Context가 사라질 때 불러왔던 라이브러리도 자동으로 해제돼요.

#### 비유하자면

Vulkan을 큰 건물이라고 생각해보세요.

- Context = 건물 입구의 안내데스크. 들어가기 전에 "어떤 시설(레이어, 확장 기능)이 있나요?"라고 물어볼 수 있어요.
- Instance = 안내데스크에서 받은 출입증. 이게 있어야 건물 안(GPU 찾기, 계산 작업 등)으로 들어갈 수 있어요.

그래서 순서가 항상 Context → Instance → 나머지가 돼요.

### 02-2. `vk::raii::Context context;` 코드 뜻

```cpp
vk::raii::PhysicalDevice   physicalDevice   = nullptr;
└──────── 타입 ────────┘   └─ 변수 이름 ─┘   └ 초기값

std::uint32_t             computeQueueFamilyIndex = 0;  // 타입: 정수
vk::raii::Context         context;                      // 타입: Context 클래스
vk::raii::PhysicalDevice  physicalDevice = nullptr;     // 타입: PhysicalDevice 클래스
ComputeApplication        app;                          // 타입: 우리가 만든 클래스
```

| 부분 | 정체 | 뜻 |
|---|---|---|
| `vk` | 네임스페이스 | Vulkan-Hpp 전체를 담은 큰 폴더 |
| `raii` | 네임스페이스 (`vk` 안에 있음) | RAII 방식 클래스들을 담은 하위 폴더 |
| `Context` | 클래스 | 객체를 찍어낼 설계도 |
| `context` | 변수(객체) | 그 설계도로 만든 실제 물건 |

`::`는 "~ 안에 있는"이라는 뜻이고, 네임스페이스는 이름이 겹치지 않게 묶어두는 폴더 같은 거예요. Python의 패키지 경로와 비슷해요.

```python
from vk.raii import Context   # vk 패키지 → raii 모듈 → Context 클래스
context = Context()           # 객체 생성
```

그래서 우리말로 하면

> "vk 네임스페이스 안의 raii 네임스페이스에 있는 Context 클래스로 context라는 객체를 하나 만든다."

## 03. auto

`auto`는 오른쪽 값을 보고 타입을 추측하는 문법이에요.

```cpp
auto properties = candidate.getProperties();  // 오른쪽을 보면 타입을 알 수 있음 → OK

auto physicalDevices = instance.enumeratePhysicalDevices();
└타입┘ └─ 변수 이름 ─┘   └──────── 함수 호출 ────────┘
```

앞에서 본 `타입 변수이름 = 값;` 모양과 똑같아요.
- `instance.enumeratePhysicalDevices()`는 함수를 실행해서 GPU 목록을 가져와요.
- `physicalDevices`는 그 결과를 담아두는 변수예요.
- `auto`는 타입 자리에 들어간 거예요.

그러니까 `auto`가 줄여준 건 함수 이름이 아니라 타입 이름이에요. 원래 타입을 직접 쓰면 이렇게 길어요.

```cpp
std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
```

`auto`라고 쓰면 컴파일러가 "오른쪽 함수가 돌려주는 값을 보니 이 타입이구나" 하고 알아서 채워줘요. 타입이 길거나 뻔할 때 많이 써요.

## 04. std::optional, const ... &

```cpp
static std::optional<std::uint32_t> findComputeQueueFamily(
    const vk::raii::PhysicalDevice& candidate)
```

| 조각 | 뜻 |
|---|---|
| `static` | 객체(`this`)의 멤버 변수를 쓰지 않는 함수예요. 받은 `candidate`만 보고 일해요. 파이썬의 `@staticmethod`와 같아요. |
| `std::optional<std::uint32_t>` | 반환 타입이에요. "0 이상의 정수가 있거나, 없거나" 둘 중 하나를 돌려줘요. |
| `findComputeQueueFamily` | 함수 이름이에요. "컴퓨트 큐 패밀리 찾기"라는 뜻이에요. |
| `const vk::raii::PhysicalDevice& candidate` | 매개변수예요. 검사할 GPU 하나를 받아요. |

### std::optional

파이썬으로 치면 `-> int | None`이에요.

```python
def find_compute_queue_family(candidate) -> int | None:
    ...
    return 2      # 찾음
    return None   # 못 찾음
```

C++에서는 `int` 자리에 `None`을 그냥 넣을 수 없어요. 그래서 "값이 없을 수도 있다"를 표현하려면 `optional`로 감싸야 해요. 못 찾았을 때는 `return std::nullopt;`라고 써요. `nullopt`가 파이썬의 `None` 역할이에요.

### const ... &

- `&` (참조): GPU 객체를 복사하지 않고 원본을 빌려서 봐요.
- `const`: 빌려 보기만 하고 바꾸지 않겠다는 약속이에요.
- 합치면 "읽기 전용으로 빌려 보기"예요. 파이썬은 원래 객체를 이렇게 넘기지만, C++은 아무것도 안 붙이면 통째로 복사하기 때문에 이렇게 명시해 줘요.

### 정리

"GPU 하나를 읽기 전용으로 받아서, 컴퓨트 명령을 처리할 수 있는 큐 패밀리의 번호를 찾으면 그 번호를, 못 찾으면 '없음'을 돌려주는 함수"예요.

## 05. std::move()

```cpp
physicalDevice = std::move(candidate);
```

후보 GPU를 멤버 변수로 옮겨 담아요. 복사 대신 이사를 시키는 거예요. `physicalDevices` 목록은 `pickPhysicalDevice()` 함수가 끝날 때 사라지지만(함수 안에서 만든 지역 변수라서), 옮겨 둔 `physicalDevice`는 남아요.

### 1. 왜 멤버 변수에 담아야 하나: 수명

```cpp
bool pickPhysicalDevice()
{
    auto physicalDevices = ...;   // 지역 변수: 함수 안에서 태어남
    for (auto& candidate : physicalDevices) { ... }
    return ...;
}                                 // ← 여기서 physicalDevices와 안의 GPU들이 전부 정리됨
```

C++은 `{ }` 블록이 끝나면 그 안에서 만든 변수를 즉시 정리해요.
파이썬도 함수가 끝나면 지역 변수는 사라지잖아요. 다만 파이썬은 다른 곳에서 참조하고 있으면 객체가 살아남지만, C++은 블록 끝에서 무조건 정리돼요.

그래서 고른 GPU를 함수가 끝난 뒤에도 쓰려면, 함수 밖에 있는 멤버 변수 `physicalDevice`로 옮겨 놔야 해요. 파이썬으로 치면 `self.physical_device = candidate`예요.

### 2. `=`의 차이: 파이썬 vs C++

```python
self.physical_device = candidate   # 파이썬: 같은 객체에 이름표 하나 더 붙이기
```

```cpp
physicalDevice = candidate;              // C++: 내용을 복사해서 새로 하나 만들기
physicalDevice = std::move(candidate);   // C++: 내용을 옮기기 (이사)
```

파이썬의 `=`는 항상 "이름표 붙이기"예요. 반면 C++의 `=`는 기본이 복사예요. 그래서 "복사하지 말고 옮겨라"라고 하고 싶을 때 `std::move`를 써요.

### 3. 옮기면(move) 무슨 일이 생기나

```
move 전                         move 후
candidate      → [GPU 핸들]     candidate      → (빈 상태, nullptr)
physicalDevice → (비어 있음)     physicalDevice → [GPU 핸들]
```

- 내용물(GPU 핸들)이 `candidate`에서 `physicalDevice`로 넘어가요.
- `candidate`는 빈 껍데기가 돼요. 그러니 move 다음에는 `candidate`를 쓰면 안 돼요. 바로 `return true;`로 끝내니까 이 코드에서는 문제없어요.
- 함수가 끝나면서 목록이 정리될 때 빈 껍데기만 정리되고, 진짜 핸들은 `physicalDevice`에 안전하게 남아요.

### 4. 솔직하게: 여기서는 복사해도 동작해요

헤더를 확인해 보면 `vk::raii::PhysicalDevice`는 복사도 허용되고 크기도 작아요. 그래서 이 줄은 `physicalDevice = candidate;`로 써도 돌아가요.

그래도 move를 쓰는 이유는 이거예요.
- 습관: `vk::raii::Instance`, `vk::raii::Device` 같은 다른 RAII 객체는 복사가 금지돼 있어서 move만 돼요. 이 튜토리얼 전체에서 같은 방식으로 쓰려는 거예요.
- 의도 표현: "이 후보는 이제 쓸 일이 없으니 멤버로 넘긴다"는 뜻이 코드에 드러나요.

> 한 줄 요약: 함수가 끝나면 사라질 후보 GPU를, 계속 살아 있는 멤버 변수로 이사시키는 것이에요.

### 5. 그냥 candidate를 계속 쓰면 안 되나?

안 돼요. `candidate`는 `pickPhysicalDevice()` 안에서만 존재하는 이름이기 때문이에요.

나중에 다른 함수들도 이 GPU가 필요해요.
- `printDeviceInfo()`: 고른 GPU 정보 출력 (이번 장)
- 다음 장들: 논리 장치(Device) 만들기, 메모리 할당하기 등

그런데 이 함수들에서는 `candidate`가 보이지 않아요.

```python
class App:
    def pick_physical_device(self):
        for candidate in devices:
            ...
            return True         # candidate는 여기서 끝

    def print_device_info(self):
        print(candidate)        # ❌ NameError: 다른 함수의 지역 변수
        print(self.physical_device)  # ✅ 멤버 변수는 어디서나 보임
```

C++도 똑같아요.
- `candidate`: 반복문 안에서만 쓰는 임시 이름 (지역 변수)
- `physicalDevice`: 클래스 전체가 같이 쓰는 보관함 (멤버 변수)

그래서 "후보 중에 합격한 걸 보관함에 넣어 둔다"는 게 `physicalDevice = std::move(candidate);`예요. 같은 일을 하는 `computeQueueFamilyIndex = *queueFamily;`도 지역 변수 `queueFamily`를 멤버 변수로 옮겨 담는 거예요.

## 06. 빌드 결과 해석

```
mungsik_polarpulse_ai@vulkan-tutorial:~/Cross-Platform-AI/examples/03-physical-devices$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
-- The CXX compiler identification is GNU 13.3.0
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- Found Vulkan: /usr/lib/x86_64-linux-gnu/libvulkan.so (found suitable version "1.3.275", minimum required is "1.3") found components: glslc missing components: glslangValidator
-- Configuring done (0.5s)
-- Generating done (0.0s)
-- Build files have been written to: /home/mungsik_polarpulse_ai/Cross-Platform-AI/examples/03-physical-devices/build
mungsik_polarpulse_ai@vulkan-tutorial:~/Cross-Platform-AI/examples/03-physical-devices$ cmake --build build --target physical_devices
[ 50%] Building CXX object CMakeFiles/physical_devices.dir/main.cpp.o
[100%] Linking CXX executable main
[100%] Built target physical_devices
mungsik_polarpulse_ai@vulkan-tutorial:~/Cross-Platform-AI/examples/03-physical-devices$ ./build/main
Selected physical device: Tesla T4
Device type: DiscreteGpu
Device API version: 1.4.329
Compute queue family index: 0
Queues in selected family: 16
shaderFloat64 supported (not enabled): yes
maxComputeWorkGroupCount: 2147483647, 65535, 65535
maxComputeWorkGroupSize: 1024, 1024, 64
maxComputeWorkGroupInvocations: 1024
maxStorageBufferRange: 4294967295 bytes
```

### 1. 빌드 과정 (명령 3개)

| 명령 | 하는 일 | 파이썬 비유 |
|---|---|---|
| `cmake -S . -B build ...` | 설정(configure): 컴파일러와 Vulkan을 찾고 빌드 계획을 `build/` 폴더에 만들어요 | 환경 확인 |
| `cmake --build build --target physical_devices` | 빌드: `main.cpp`를 기계어로 컴파일(50%)하고 실행 파일로 링크(100%)해요 | (파이썬엔 이 단계 없음) |
| `./build/main` | 만들어진 프로그램 실행 | `python main.py` |

- `Found Vulkan ... 1.3.275`: 설치된 Vulkan SDK 버전이에요. 1.3 이상이 필요한데 충족됐어요.
- `missing components: glslangValidator`: 셰이더 도구 하나가 없다는 안내예요. 이 튜토리얼은 같은 일을 하는 `glslc`를 쓰고, `glslc`는 이미 설치돼 있어요(`found components: glslc`). 그래서 무시해도 돼요. 나중 장에서 `glslangValidator`를 직접 쓰라고 하면 그때 `sudo apt install glslang-tools`로 설치하면 돼요.

### 2. 실행 결과 (`printDeviceInfo()`가 출력한 것)

| 출력 | 뜻 |
|---|---|
| `Selected physical device: Tesla T4` | 고른 GPU는 NVIDIA Tesla T4예요 (GCP 서버 GPU) |
| `Device type: DiscreteGpu` | 내장 그래픽이 아닌 독립형 GPU예요 |
| `Device API version: 1.4.329` | 드라이버가 Vulkan 1.4까지 지원해요. 1.3 이상이라 첫 번째 `continue` 관문을 통과했어요 |
| `Compute queue family index: 0` | `findComputeQueueFamily()`가 돌려준 값이에요. 0번 패밀리가 컴퓨트를 지원했어요 |
| `Queues in selected family: 16` | 그 패밀리에 큐(창구)가 16개 있어요. `queueCount > 0` 조건이 이거예요 |
| `shaderFloat64 supported (not enabled): yes` | 64비트 실수(`double`) 연산을 지원해요. 다만 아직 켜지는 않았어요 |
| `maxComputeWorkGroupCount` | 한 번에 띄울 수 있는 작업 그룹 수의 최대치예요 (x, y, z 방향) |
| `maxComputeWorkGroupSize` | 작업 그룹 하나의 최대 크기예요 (x, y, z 방향) |
| `maxComputeWorkGroupInvocations: 1024` | 작업 그룹 하나에 들어갈 수 있는 스레드 수의 최대치예요 (x×y×z ≤ 1024) |
| `maxStorageBufferRange: 4294967295 bytes` | 셰이더가 한 번에 다룰 수 있는 버퍼 크기예요. 약 4GB (= `uint32_t` 최댓값) |

아래쪽 `max...` 값들은 이 GPU의 한계 스펙이에요. 뒤 장에서 컴퓨트 셰이더를 실행할 때 "이 범위 안에서만 일을 나눠 줄 수 있다"는 기준이 돼요. 지금은 "이런 값이 있구나" 정도만 알아 두면 돼요.
