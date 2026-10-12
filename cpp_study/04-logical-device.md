## 01. {} vs ()

### {}: 변수를 만들고 0으로 초기화

```cpp
vk::DeviceQueueCreateInfo queueCreateInfo{};
```

"`DeviceQueueCreateInfo` 타입의 변수 `queueCreateInfo`를 만들고, 안의 필드를 기본값(0, `nullptr` 등)으로 채운다"는 뜻이에요. 파이썬의 `queue_create_info = DeviceQueueCreateInfo()`와 같아요.

### 빈 (): 함수 선언이 돼 버림 ⚠️

```cpp
vk::DeviceQueueCreateInfo queueCreateInfo();
```

C++ 컴파일러는 이 줄을 **"인자 없이 `DeviceQueueCreateInfo`를 돌려주는 `queueCreateInfo`라는 함수가 있다"**는 선언으로 읽어요.

```cpp
int getNumber();   // 이거랑 같은 모양 → 함수 선언
```

그래서 변수가 만들어지지 않아요. 다음 줄에서 `queueCreateInfo.queueFamilyIndex = ...`처럼 필드를 쓰면, "함수에는 필드가 없다"는 이상한 에러가 나요. C++에서 유명한 함정이고, 이름도 붙어 있어요 ("most vexing parse", 가장 짜증나는 해석).

파이썬에서는 `Foo()`가 항상 "객체 만들기"지만, C++에서는 빈 괄호를 변수 이름 뒤에 붙이면 함수 선언이 된다는 점이 달라요.

### 정리

| 쓰는 법 | 의미 |
|---|---|
| `Type name{};` | ✅ 변수 생성 + 기본값 초기화 (튜토리얼 방식) |
| `Type name;` | 변수 생성. 단, 기본 타입(`int` 등)은 값이 쓰레기값일 수 있어요 |
| `Type name();` | ❌ 함수 선언 |
| `Type name(값);` | 값을 넣어 생성. 괄호 안에 뭔가 있으면 변수가 돼요 (예: `vk::raii::Instance(context, createInfo)`) |

그래서 Vulkan 구조체는 항상 `{}`로 만든다고 기억해 두시면 편해요.

### 괄호 세 종류 한눈에

| 괄호 | 주 용도 |
|---|---|
| `()` 소괄호 | 함수 호출·선언, 조건문 `if (...)`, 값을 넣어 생성 |
| `{}` 중괄호 | 코드 블록, 변수 초기화 `Type x{};`, 배열 값 목록 `{1, 2, 3}` |
| `[]` 대괄호 | 인덱싱 `a[i]`, 배열 선언 `int a[3]`, 람다 `[](...){}` |

## 02. 주소 넘기기: &queuePriority

```cpp
queueCreateInfo.pQueuePriorities = &queuePriority;
```

"큐 생성 정보의 우선순위 칸(`pQueuePriorities`)에, `queuePriority` 변수가 있는 곳의 주소를 넣는다."

### 조각별 뜻

| 조각 | 뜻 |
|---|---|
| `queueCreateInfo.` | 앞에서 만든 큐 생성 정보 구조체의 필드에 접근해요 |
| `pQueuePriorities` | `p` = pointer(포인터)예요. "우선순위들이 있는 곳의 주소"를 담는 칸이에요 |
| `&queuePriority` | `queuePriority`(1.0)의 주소예요 |

### 왜 값(1.0)을 바로 안 넣고 주소를 넣나

01장 노트 "포인터 쓰는 이유"에서 정리한 원리랑 같아요. **크기가 정해지지 않은 데이터는 주소로 연결한다.**

이 칸은 우선순위를 큐 개수만큼 받아요.
- `queueCount = 1`이면 우선순위 1개 → `{1.0}`
- `queueCount = 3`이면 우선순위 3개 → `{1.0, 0.5, 0.5}`

개수가 그때그때 달라서 구조체 칸에 직접 넣을 수 없어요. 그래서 "우선순위 목록은 저기 있다"는 첫 번째 값의 주소를 넣어요. 지금은 하나만 요청했으니 변수 하나의 주소를 넣으면 돼요. `pApplicationName`에 문자열 주소를 넣은 것과 같은 방식이에요.

### 주소를 모르는 게 아니라, 크기를 몰라서 주소를 써요

구조체의 칸은 컴파일할 때 크기가 미리 고정돼요.

```
DeviceQueueCreateInfo
┌──────────────────────┬──────────┐
│ queueFamilyIndex     │ 4바이트   │  숫자 하나 → 크기 고정 → 값을 직접 넣음
│ queueCount           │ 4바이트   │  숫자 하나 → 크기 고정 → 값을 직접 넣음
│ pQueuePriorities     │ 8바이트   │  우선순위 몇 개? 1개? 3개? 16개? → 모름
└──────────────────────┴──────────┘
```

그래서 칸에는 주소만 넣고, 데이터는 다른 곳에 둬요. **주소는 무엇을 가리키든 항상 8바이트**라서 칸 크기가 고정될 수 있어요.

```
구조체 칸                         실제 데이터 (다른 곳)
pQueuePriorities: [0x1000] ───▶  0x1000: [1.0] [0.3] [0.7]
```

주소는 우리가 알고 있어요. `&queuePriority`가 바로 "이 변수의 주소를 알려 줘"라는 뜻이고, Vulkan은 그 주소로 가서 `queueCount` 개수만큼 읽어요.

> 비유: 택배 송장 칸에 물건을 넣을 수는 없으니 "물건은 이 주소에 있어요"라고 주소를 적는 것. 주소 칸은 물건 크기와 상관없이 항상 같은 크기예요.

### ⚠️ 같은 &인데 뜻이 달라요

| 위치 | 뜻 | 예 |
|---|---|---|
| 타입 뒤 (선언할 때) | 참조: 원본을 빌려 보기 | `const vk::raii::PhysicalDevice& candidate` |
| 변수 앞 (값으로 쓸 때) | 주소 꺼내기 | `&queuePriority`, `&appInfo` |

이 줄은 변수 앞에 붙었으니 **"주소 꺼내기"**예요.

## 03. 우선순위

GPU에게 주는 **"이 큐의 일을 얼마나 먼저 처리해 줄지"**에 대한 힌트예요.

### 비유: 은행 창구

03장에서 큐를 "일을 받는 창구"라고 했잖아요. 큐를 여러 개 만들면 그 창구들이 같은 GPU 직원들을 나눠 써야 해요. 우선순위는 이렇게 정해 두는 거예요.

```
큐 A (우선순위 1.0) → 급한 일: 먼저, 더 많이 처리해 줘
큐 B (우선순위 0.5) → 덜 급한 일: 여유 있을 때
```

- 값은 `0.0`(가장 낮음)부터 `1.0`(가장 높음)까지 아무 실수나 쓸 수 있어요 (`0.3f`, `0.75f`도 가능). 범위 밖(`1.5f`, `-0.1f`)은 안 돼요.
- 같은 장치 안의 큐들끼리 비교하는 상대적인 값이에요. 다른 프로그램보다 먼저 처리된다는 뜻은 아니에요.
- "힌트"일 뿐이에요. 실제로 얼마나 반영할지는 GPU 드라이버가 정해요. 드라이버가 몇 단계로 묶어 처리하는 경우가 많아서, 세밀한 숫자보다 큐들 사이의 크고 작음만 의미가 있어요.

### 지금 코드에서는

큐를 1개만 요청했으니 비교할 상대가 없어요. 그래서 사실상 의미가 없어요. 그래도 Vulkan 규칙상 큐마다 값을 하나씩 꼭 줘야 해서, 그냥 최댓값 `1.0f`를 넣은 거예요. README에 "큐가 하나일 때도 지정해야 합니다"라고 적힌 이유가 이거예요.

> `1.0f`의 `f`는 "`float`(32비트 실수)"라는 표시예요. 그냥 `1.0`이라고 쓰면 C++은 `double`(64비트)로 봐요. 파이썬의 `float`가 C++의 `double`에 해당해요.

### 큐를 여러 개 쓴다면

큐 개수만큼 우선순위 배열을 줘요. 배열의 순서가 곧 **패밀리 안의 큐 번호**예요.

```cpp
const float queuePriorities[] = {1.0f, 0.3f, 0.7f};
//                                 ↓      ↓      ↓
//                              0번 큐  1번 큐  2번 큐

queueCreateInfo.queueCount = 3;                       // 배열 칸 수와 같아야 함
queueCreateInfo.pQueuePriorities = queuePriorities;   // 배열은 이름만 써도 첫 칸의 주소 (& 불필요)
```

## 05. createLogicalDevice 흐름과 컴퓨트 패밀리

### ① 큐 요청서 작성

```cpp
const float queuePriority = 1.0f;

vk::DeviceQueueCreateInfo queueCreateInfo{};
queueCreateInfo.queueFamilyIndex = computeQueueFamilyIndex;
queueCreateInfo.queueCount = 1;
queueCreateInfo.pQueuePriorities = &queuePriority;
```

"큐 요청서(`queueCreateInfo`)를 빈 칸으로 하나 만들고, 이렇게 채운다."
- 어느 패밀리에서: 03장에서 찾은 컴퓨트 패밀리(0번)
- 몇 개: 1개
- 우선순위: `queuePriority`가 있는 곳의 주소 (1.0)

### ② 사용할 기능 목록

```cpp
vk::PhysicalDeviceFeatures enabledFeatures{};
```

"켤 기능 목록을 만드는데, 전부 꺼진 상태(`{}` = 0)로 둔다."
03장 출력의 `shaderFloat64 supported (not enabled)`처럼, 지원은 되지만 켜지 않은 기능들이에요. 이번 실습에는 특별히 켤 기능이 없어서 빈 목록이에요.

### ③ 장치 생성 정보 (최종 신청서)

```cpp
vk::DeviceCreateInfo createInfo{};
createInfo.queueCreateInfoCount = 1;
createInfo.pQueueCreateInfos = &queueCreateInfo;
createInfo.pEnabledFeatures = &enabledFeatures;
```

"장치 생성 신청서(`createInfo`)를 만들고, 앞에서 만든 것들을 연결한다."
- 큐 요청서 몇 장: 1장
- 큐 요청서 위치: `queueCreateInfo`의 주소
- 켤 기능 목록 위치: `enabledFeatures`의 주소

### ④ 장치 만들고 큐 가져오기

```cpp
device = vk::raii::Device(physicalDevice, createInfo);   // 이 GPU로, 이 신청서대로
computeQueue = vk::raii::Queue(device, computeQueueFamilyIndex, queueIndex);   // 패밀리 0번의 큐 0번
```

- **physicalDevice (물리 장치)**: 03장에서 고른 실제 GPU 하드웨어. 조회만 할 수 있어요.
- **device (논리 장치)**: 그 GPU를 내 프로그램이 쓰기 위해 연 "작업 연결". 큐를 받고, 메모리를 할당하고, 명령을 보내는 건 전부 이걸로 해요.

파이썬으로 치면 `physicalDevice`는 "DB 서버 주소", `device`는 `connect()`로 연 "DB 연결"에 가까워요.

### 전체 흐름

```
queuePriority ──▶ queueCreateInfo ──┐
                                    ├──▶ createInfo ──▶ device ──▶ computeQueue
              enabledFeatures ──────┘        ▲
                                    physicalDevice
```

작은 서류를 먼저 쓰고, 큰 신청서에 그 서류들의 주소를 붙이는 구조예요. 01장 `appInfo` → `createInfo` → `Instance` 순서와 똑같은 패턴이에요.

### 왜 1개일까?

1개인 건 두 가지인데, 둘 다 "컴퓨트 패밀리가 1개"라는 뜻은 아니에요.

- `queueCount = 1`: **큐**가 1개. "패밀리 0번 안에서 큐를 1개만 쓰겠다"는 뜻이에요. Tesla T4는 이 패밀리에 큐가 16개 있지만, 튜토리얼은 하나면 충분해요.
- `queueCreateInfoCount = 1`: **큐 요청서**가 1장. 요청서(`DeviceQueueCreateInfo`)는 패밀리 하나당 한 장이에요.

패밀리를 하나만 고른 건 03장 `findComputeQueueFamily()`가 그렇게 동작했기 때문이에요.

```cpp
if (family.queueCount > 0 && (family.queueFlags & vk::QueueFlagBits::eCompute))
{
    return index;   // ← 처음 찾은 패밀리에서 바로 끝냄
}
```

컴퓨트를 지원하는 패밀리가 여러 개 있어도 첫 번째 하나의 번호만 돌려주고 끝나요. 그 번호가 `computeQueueFamilyIndex`(= 0)에 저장됐고, 지금 그 번호를 쓰는 거예요.

```
GPU (Tesla T4)
 ├─ 패밀리 0 ← 03장에서 고른 것 (컴퓨트 지원하는 첫 번째)
 │   ├─ 큐 0 ← 이번에 요청한 1개
 │   ├─ 큐 1
 │   └─ ... (16개)
 ├─ 패밀리 1
 └─ ...
```

나중에 다른 패밀리(예: 전송 전용)의 큐도 같이 쓰고 싶으면, 요청서를 2장 만들고 `queueCreateInfoCount = 2`로 쓰면 돼요.

### 패밀리란?

**패밀리(Queue Family)는 "할 수 있는 일이 똑같은 큐들의 묶음"**이에요.

```
은행 (GPU)
 ├─ 종합 창구 구역  : 예금, 대출, 송금 다 됨   → 창구 16개
 ├─ 송금 전용 구역  : 송금만                 → 창구 2개
 └─ 대출 전용 구역  : 대출, 송금             → 창구 8개
```

- 큐 = 창구 하나
- 패밀리 = 같은 업무를 보는 창구들이 모인 구역

같은 구역 안의 창구들은 할 수 있는 일이 완전히 같아요. 그래서 Vulkan은 큐를 하나하나 설명하지 않고 "이 구역은 이런 일을 하고, 창구가 몇 개 있다"는 식으로 알려 줘요. 03장에서 `getQueueFamilyProperties()`로 받은 게 바로 이 구역별 안내판이에요.

#### 이 서버의 Tesla T4

| 패밀리 | 큐 개수 | 할 수 있는 일 |
|---|---|---|
| 0 | 16 | 그래픽 + 컴퓨트 + 전송 ← 03장에서 고른 것 |
| 1 | 2 | 전송 |
| 2 | 8 | 컴퓨트 + 전송 |
| 3 | 2 | 전송 + 영상 디코딩 |
| 4 | 1 | 전송 + 영상 인코딩 |

- 컴퓨트를 지원하는 패밀리는 0번과 2번 두 개예요. `findComputeQueueFamily()`는 처음 찾은 0번을 돌려줬어요.
- 03장 출력의 `Queues in selected family: 16`은 0번 패밀리의 큐 개수였어요.
- (위 표에서는 모든 패밀리에 붙은 `SPARSE_BINDING`은 생략했어요.)

GPU 안에는 실제로 하드웨어 일꾼이 따로 있어요. 그래픽 엔진, 계산 엔진, 데이터 복사 엔진, 영상 인코더 같은 것들이에요. 패밀리는 그 하드웨어 구성을 그대로 보여 주는 거예요. 예를 들어 1번(전송 전용)을 쓰면 계산하는 동안 데이터 복사를 따로 맡길 수 있어요.

#### 직접 확인하는 법

```sh
vulkaninfo 2>/dev/null | grep -E "queueProperties\[|queueCount|queueFlags"
```

- `vulkaninfo --summary`(요약판)에는 큐 패밀리 정보가 안 나와요.
- `grep -A3`처럼 아래 몇 줄만 보면 구분선 때문에 `queueFlags`까지 안 닿아요. 필요한 줄만 `-E`로 골라 봐요.
- `'DISPLAY' environment variable not set`은 화면 없는 서버라서 나오는 안내예요. `2>/dev/null`로 숨길 수 있어요.
- 출력 맨 아래의 큐 1개짜리 장치는 CPU로 흉내 내는 소프트웨어 장치(`llvmpipe`)로, Tesla T4와 별개예요.

| `queueFlags` 플래그 | 뜻 | 코드에서 |
|---|---|---|
| `QUEUE_GRAPHICS_BIT` | 그래픽(화면 그리기) | `vk::QueueFlagBits::eGraphics` |
| `QUEUE_COMPUTE_BIT` | 컴퓨트(계산) | `vk::QueueFlagBits::eCompute` |
| `QUEUE_TRANSFER_BIT` | 전송(데이터 복사) | `vk::QueueFlagBits::eTransfer` |
| `QUEUE_SPARSE_BINDING_BIT` | 메모리를 조각조각 연결하는 고급 기능 | `vk::QueueFlagBits::eSparseBinding` |

## 06. 빌드와 실행 결과

```sh
cd ~/Cross-Platform-AI/examples/04-logical-device   # 장마다 자기 폴더에서!
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target logical_device_example
./build/main
```

```
Logical device created.
Compute queue ready: family 0, queue 0.
Device destroyed.
```

| 출력 | 뜻 |
|---|---|
| `Logical device created.` | Tesla T4의 논리 장치(작업 연결)를 열었어요 |
| `Compute queue ready: family 0, queue 0.` | 패밀리 0번의 큐 0번을 `computeQueue`에 받아 뒀어요 |
| `Device destroyed.` | `main()`의 안쪽 `{ }` 블록이 끝나면서 `app`이 정리됐어요. RAII라서 `device`도 자동으로 해제된 뒤에 출력돼요 |

- 03장 폴더에서 `--target logical_device_example`을 하면 `No rule to make target`이 나요. 03장 `build/`는 03장 타깃만 알기 때문이에요.
- cmake 출력의 `CXX`는 C++를 뜻해요 (`+`를 눕혀 `x`로 쓴 관례). `CC`는 C 컴파일러, `CXX`는 C++ 컴파일러예요.
