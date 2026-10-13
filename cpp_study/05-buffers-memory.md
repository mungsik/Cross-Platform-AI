## 00. 버퍼란?

버퍼 : 데이터를 담아두는 연속된 메모리(바이트) 공간

### 00-1.Vulkan에서 버퍼가 특별한 이유

파이썬에서는 np.zeros() 한 줄이면 메모리 확보까지 끝나요. Vulkan에서는 이 일이 두 단계로 나뉘어 있어요. 그래서 BufferResource 안에 멤버가 두 개 있는 거예요.

struct BufferResource
{
    vk::raii::DeviceMemory memory = nullptr;  // ② 실제 메모리 (GPU가 접근하는 진짜 공간)
    vk::raii::Buffer buffer = nullptr;        // ① "이런 버퍼를 쓸 거야"라는 설명서/손잡이
};

┌──────────────────┬────────────────────────────────────────────────────────────────────┬─────────────────┐
│                  │                              하는 일                               │      비유       │
├──────────────────┼────────────────────────────────────────────────────────────────────┼─────────────────┤
│ vk::Buffer       │ 크기와 용도(예: 셰이더가 읽고 쓰는 storage buffer)를 정의하는 핸들 │ 집 설계도       │
├──────────────────┼────────────────────────────────────────────────────────────────────┼─────────────────┤
│ vk::DeviceMemory │ GPU 쪽에서 실제로 할당한 메모리                                    │ 땅              │
├──────────────────┼────────────────────────────────────────────────────────────────────┼─────────────────┤
│ bindMemory       │ 둘을 연결해요                                                      │ 땅 위에 집 짓기 │
└──────────────────┴────────────────────────────────────────────────────────────────────┴─────────────────┘

이렇게 나눠둔 이유는 GPU 메모리에 종류가 여러 개이기 때문이에요. CPU가 직접 쓸 수 있는 메모리도 있고, GPU만 빠르게 접근하는 메모리(VRAM)도 있어요. Vulkan은 어떤 메모리를 쓸지 프로그래머가 직접 고르게 해요. 그래서 이 과정에 보통 findMemoryType 같은 함수가 등장해요.

### 00-2. numpy vs vulkan 버퍼

CPU 메모리와 GPU 메모리는 따로 있어요

┌─────────── CPU 쪽 ───────────┐        ┌─────────── GPU 쪽 ───────────┐
│  RAM                          │        │  VRAM (GPU 전용 메모리)       │
│  np.zeros(8) ← 여기 생김      │ ◀PCIe▶ │  vk::DeviceMemory ← 여기 생김 │
│  (CPU가 계산)                 │        │  (GPU 셰이더가 계산)          │
└───────────────────────────────┘        └───────────────────────────────┘

- np.zeros는 CPU의 RAM에 메모리를 잡아요. 그 배열로 하는 계산(a + b)도 CPU가 해요.
- GPU 셰이더는 드라이버를 통해 GPU용으로 할당된 메모리만 읽고 쓸 수 있어요. 파이썬 프로세스가 아무렇게나 잡은 RAM 주소는 GPU 입장에서 접근할 방법이 없어요.
- Vulkan 버퍼(vk::Buffer + vk::DeviceMemory)는 "GPU가 접근할 수 있는 메모리를 주세요"라고 드라이버에 요청하는 공식 창구예요.

파이썬에서도 사실 같은 일을 하고 있어요

PyTorch를 써봤다면 이미 경험한 개념이에요.

import torch
a = torch.zeros(8)                  # CPU RAM에 생김  ≈ np.zeros
b = torch.zeros(8, device="cuda")   # GPU 메모리에 생김 ≈ Vulkan 버퍼
c = a.to("cuda")                    # CPU → GPU 복사

device="cuda"라고 쓰는 순간, PyTorch가 내부에서 GPU 메모리를 할당해요(CUDA의 cudaMalloc). Vulkan 버퍼를 만드는 건 그 숨겨진 과정을 C++에서 직접 하는 것이에요.

┌───────────────────────────────┬─────────────────────────────────────────────────────────────────────┐
│         파이썬 (자동)         │                            Vulkan (수동)                            │
├───────────────────────────────┼─────────────────────────────────────────────────────────────────────┤
│ torch.zeros(8, device="cuda") │ vk::Buffer 생성 → 메모리 타입 선택 → DeviceMemory 할당 → bindMemory │
├───────────────────────────────┼─────────────────────────────────────────────────────────────────────┤
│ a.to("cuda")                  │ 메모리 매핑(mapMemory) 후 memcpy, 또는 복사 명령 실행               │
├───────────────────────────────┼─────────────────────────────────────────────────────────────────────┤
│ c.cpu()                       │ 결과 버퍼를 매핑해서 읽기                                           │
└───────────────────────────────┴─────────────────────────────────────────────────────────────────────┘

그럼 왜 GPU를 쓰냐면

float 8개 정도면 np.zeros로 CPU에서 계산하는 게 훨씬 빠르고 편해요. 이 예제는 흐름을 배우려고 작게 만든 거예요. 원소가 수백만 개이고 같은 계산을 병렬로 해야 할 때(딥러닝, 이미지 처리 등) GPU가 수천 개 코어로 한꺼번에 처리해서 큰 차이가 나요.

정리하면 np.zeros는 CPU용 그릇이고, Vulkan 버퍼는 GPU용 그릇이에요. GPU에게 일을 시키려면 GPU용 그릇에 데이터를 담아서 넘겨야 해요.

## 01. 버퍼 크기 정하기: static_assert, constexpr

```cpp
static_assert(sizeof(float) == 4);
static constexpr std::uint32_t elementCount = 8;
static constexpr vk::DeviceSize bufferSize = sizeof(float) * elementCount;
```

### static_assert(sizeof(float) == 4);

"컴파일할 때 `float` 하나가 4바이트인지 확인하고, 아니면 빌드를 멈춰라."
- `sizeof(타입)`: 그 타입이 메모리에서 몇 바이트를 차지하는지 알려 줘요.
- `static_assert(조건)`: 컴파일할 때 검사하는 `assert`예요. 파이썬의 `assert`는 실행 중에 검사하지만, 이건 빌드 단계에서 검사해요. 조건이 틀리면 프로그램이 아예 만들어지지 않아요.
- 왜 확인하나: 버퍼 크기를 "4바이트 × 8개"로 계산하는데, 혹시 `float`가 4바이트가 아닌 특이한 환경이면 계산이 틀어져요. 그런 실수를 미리 막는 안전장치예요. (거의 모든 환경에서 4바이트라 보통은 그냥 통과해요.)

### static constexpr std::uint32_t elementCount = 8;

"원소 개수는 8개로 고정한다."
- 02장에서 배운 `static constexpr` 조합이에요. 클래스 전체에 하나만 있고(`static`), 컴파일할 때 값이 확정돼요(`constexpr`).
- `std::uint32_t`: 0 이상의 32비트 정수예요.
- 파이썬으로는 클래스 상수 `ELEMENT_COUNT = 8`에 해당해요.

### static constexpr vk::DeviceSize bufferSize = sizeof(float) * elementCount;

"버퍼 크기 = `float` 하나 크기(4) × 개수(8) = 32바이트."
- `vk::DeviceSize`: Vulkan에서 메모리 크기를 나타낼 때 쓰는 타입이에요. 실제로는 64비트 정수(`uint64_t`)예요. GPU 메모리는 수 GB가 넘을 수 있어서 32비트로는 부족하거든요.
- 계산도 컴파일할 때 끝나요. 실행 파일 안에는 그냥 `32`가 들어가 있어요.

### 정리

```
float 1개 = 4바이트  (static_assert로 확인)
× 8개              (elementCount)
= 32바이트          (bufferSize) → 버퍼 하나의 크기
```

파이썬 numpy로 치면 `np.zeros(8, dtype=np.float32).nbytes == 32`를 미리 계산해 둔 거예요.

### Q1. constexpr이 뭐야?

**"이 값은 컴파일할 때 이미 정해진다"**는 표시예요. 풀네임은 **constant expression**(상수 표현식)이에요. 02장 노트에서도 정리했어요.

| | 의미 |
|---|---|
| `const` | 한 번 정하면 바꿀 수 없음 (값은 실행 중에 정해져도 됨) |
| `constexpr` | 바꿀 수 없음 + 컴파일할 때 값이 확정됨 |

```cpp
const int a = 사용자입력();          // OK: 실행해 봐야 값을 알지만 이후엔 못 바꿈
constexpr int b = 8;                // OK: 컴파일할 때 이미 8
constexpr int c = 사용자입력();      // ❌ 에러: 컴파일할 때 값을 알 수 없음
```

여기서 꼭 `constexpr`이어야 하는 이유가 있어요. README 뒤쪽에 이런 코드가 나와요.

```cpp
std::array<float, elementCount> valuesA{};
```

`std::array`는 크기가 고정된 배열이라서, 칸 수를 컴파일할 때 알아야 해요. 그래서 `elementCount`가 `constexpr`이어야 이 자리에 들어갈 수 있어요. 파이썬 리스트처럼 실행 중에 크기가 정해지는 배열은 `std::vector`를 써요.

### Q2. 왜 원소 개수를 8로 고정해?

튜토리얼용으로 고른 작은 숫자예요. 8이어야 하는 특별한 규칙은 없어요.
- 눈으로 확인하기 쉬워요. 05장 마지막에 값을 써 넣고 다시 읽어서 출력하는데, 8개 정도면 터미널 한 줄에 다 보여요. 100만 개면 맞는지 확인할 수 없잖아요.
- 원리는 똑같아요. 8개든 100만 개든 버퍼를 만들고, 메모리를 잡고, 값을 쓰는 과정은 같아요. 숫자 하나만 바꾸면 돼요.
- 고정(`constexpr`)한 이유는 위 Q1처럼 `std::array` 크기로 쓰기 위해서예요.

실제 AI 모델에서는 버퍼 크기가 텐서 모양에 따라 수 MB~GB가 되고, 실행 중에 정해지기도 해요. 그럴 때는 `constexpr` 대신 일반 변수와 `std::vector`를 써요.

### Q3. 바이트랑 비트 차이가 뭐야?

| | 뜻 | 담을 수 있는 것 |
|---|---|---|
| 비트(bit) | 가장 작은 단위. 0 또는 1 하나 | 스위치 하나 (켜짐/꺼짐) |
| 바이트(byte) | 비트 8개 묶음 | 0~255 숫자 하나, 영어 글자 하나 |

```
1바이트 = [0][1][0][0][0][0][0][1]   ← 비트 8개 (이건 숫자 65, 글자 'A')
```

지금까지 나온 것들에 대입하면 이래요.

| 타입 | 바이트 | 비트 |
|---|---|---|
| `char` | 1 | 8 |
| `float`, `uint32_t` | 4 | 32 ← 이름에 붙은 "32"가 비트 수예요 |
| `double`, `vk::DeviceSize`, 주소(포인터) | 8 | 64 ← "64비트 시스템"의 64 |
| 이번 버퍼 (`float` × 8) | 32 | 256 |

- 메모리 크기는 보통 바이트로 세요 (`bufferSize` = 32바이트, GPU 메모리 16GB).
- 타입 이름이나 플래그는 비트로 말할 때가 많아요 (`uint32_t`, 03장 `queueFlags`의 "비트 스위치").

## 02. member vs. field

멤버(member)가 더 넓은 말이고, 필드(field)는 멤버 중 "변수"만 가리키는 말이에요.

```
멤버 (클래스·구조체 안에 들어 있는 모든 것)
 ├─ 멤버 변수 = 필드 = 데이터 멤버   ← 값을 담음
 │    예: inputA, device, computeQueueFamilyIndex
 └─ 멤버 함수 (메서드)              ← 일을 함
      예: createLogicalDevice(), pickPhysicalDevice()
```

파이썬으로 치면 이래요.

```python
class ComputeApplication:
    def __init__(self):
        self.device = None        # 필드 (= 멤버 변수, 속성/attribute)
    def create_logical_device(self):   # 멤버 함수 (메서드)
        ...
# 둘 다 이 클래스의 "멤버"
```

### 언제 어떤 말을 쓰나

- 멤버 변수 / 데이터 멤버: C++에서 공식적으로 쓰는 말이에요.
- 필드: Java·C# 쪽에서 많이 쓰는 말이에요. C++에서도 특히 구조체의 칸을 말할 때 자주 써요 (`appInfo.apiVersion` 필드). 01장 노트에서 "칸 = 필드"라고 정리했던 그거예요.
- 그래서 TODO 주석의 "`inputA`, `inputB`, `output` 멤버를 추가합니다"는 "멤버 변수(필드)를 추가하라"는 뜻이에요. 사실상 같은 말로 봐도 돼요.

### 지금 코드에서

```cpp
BufferResource inputA;
BufferResource inputB;
BufferResource output;
```

이 세 줄은 `ComputeApplication`의 **필드(멤버 변수)**예요. 그리고 각각의 타입인 `BufferResource` 안에도 `memory`, `buffer`라는 필드가 있어요.

```
ComputeApplication의 필드: inputA
                          └─ BufferResource의 필드: memory, buffer
```

그래서 나중에 `inputA.buffer`처럼 `.`을 두 번 타고 들어가요.

이 필드들은 `device` **아래**에 선언해요. 정리 순서가 역순이라 버퍼들이 `device`보다 먼저 정리돼요. 버퍼는 `device`로 만든 거라서 이 순서가 맞아요.

### 왜 struct 안이 아니라 따로 선언하나

`struct BufferResource { ... }`는 **틀(타입)**을 정의하는 곳이고, `inputA` 등은 그 틀로 만든 **실제 물건(변수)**이에요.
- 틀 안에 같은 틀의 물건을 넣으면 "상자 안에 같은 상자가 끝없이" 들어가서 컴파일 에러가 나요.
- 타입 정의는 쓰기 전에 있어야 해서 위쪽에, 변수는 정리 순서 때문에 `device` 아래쪽에 둬요.

## 03. buffer

버퍼는 GPU가 계산할 때 읽고 쓰는 데이터 덩어리예요. 파이썬으로 치면 숫자 배열(numpy 배열)을 GPU 쪽에 둔 거예요.

다만 Vulkan에서는 두 가지로 나뉘어 있어요.

| | 정체 | 비유 |
|---|---|---|
| 버퍼 (`vk::raii::Buffer`) | "32바이트, 계산용"이라고 적힌 설명서·이름표 | 택배 송장 |
| 메모리 (`vk::raii::DeviceMemory`) | 데이터가 실제로 저장되는 공간 | 실제 상자 |

버퍼를 만든다고 저장 공간이 생기지 않아요. 둘을 따로 만들고 연결해야 해요.

```
① 버퍼 만들기          "크기 32바이트, 용도는 계산용"
② 필요한 메모리 확인     getMemoryRequirements()
③ 메모리 종류 고르기     findMemoryType()
④ 메모리 할당 + 연결     DeviceMemory, bindMemory
⑤ CPU에서 값 쓰기       mapMemory → memcpy
```

```cpp
struct BufferResource
{
    vk::raii::DeviceMemory memory = nullptr;   // 공간
    vk::raii::Buffer buffer = nullptr;         // 설명서
};
```

`memory`를 먼저 선언했으니 정리는 반대 순서예요. `buffer`가 먼저 정리되고, 그다음 `memory`가 정리돼요.

### 메모리 요구사항과 원하는 성질

```cpp
const auto requirements = resource.buffer.getMemoryRequirements();
const auto properties = vk::MemoryPropertyFlagBits::eHostVisible
                      | vk::MemoryPropertyFlagBits::eHostCoherent;
```

| 필드 | 뜻 |
|---|---|
| `requirements.size` | 실제로 할당해야 하는 크기 (요청보다 클 수 있음) |
| `requirements.alignment` | 메모리 시작 위치가 몇의 배수여야 하는지 |
| `requirements.memoryTypeBits` | 쓸 수 있는 메모리 종류 목록 (비트 스위치) |

Vulkan에서 **host = CPU 쪽**, device = GPU 쪽이에요.

| 플래그 | 뜻 |
|---|---|
| `eHostVisible` | CPU가 이 메모리에 직접 접근할 수 있음 (map 가능) |
| `eHostCoherent` | CPU가 쓴 값이 flush 없이 반영됨 (GPU 완료를 기다려 주는 기능은 아님) |

## 04. enum 값 앞의 e

`eStorageBuffer`, `eExclusive`의 `e`는 **enum**(열거형) 값이라는 표시예요. 값의 뜻과는 상관없어요.

```
e  +  Exclusive
│      └─ 값의 뜻: "독점"
└─ 표시: "이건 enum의 선택지 중 하나예요"
```

- 파이썬에서 enum 값을 대문자(`SharingMode.EXCLUSIVE`)로 쓰듯이, Vulkan-Hpp는 앞에 `e`를 붙여요.
- C API의 `VK_BUFFER_USAGE_STORAGE_BUFFER_BIT`가 `vk::BufferUsageFlagBits::eStorageBuffer`로 바뀐 거예요.
- 숫자로 시작하는 값(`VK_IMAGE_TYPE_1D` → `e1D`)도 이름으로 쓸 수 있게 하려는 규칙이에요.

### 선택지 예시

`vk::SharingMode`는 둘 중 하나만 골라요.

| 값 | 뜻 |
|---|---|
| `eExclusive` | 한 큐 패밀리만 사용 ← 우리가 고른 것 |
| `eConcurrent` | 여러 큐 패밀리가 같이 사용 |

`vk::BufferUsageFlagBits`는 `FlagBits`라서 `|`로 여러 개를 켤 수 있어요.

| 값 | 용도 |
|---|---|
| `eTransferSrc` / `eTransferDst` | 복사해 보내는 쪽 / 받는 쪽 |
| `eUniformBuffer` | 셰이더가 읽기만 하는 작은 상수 데이터 |
| **`eStorageBuffer`** | 셰이더가 읽고 쓰는 큰 데이터 ← 우리가 고른 것 |
| `eVertexBuffer` / `eIndexBuffer` | 그래픽용 정점 / 정점 순서 |
| `eIndirectBuffer` | GPU가 실행 인자를 읽음 |

## 05. mapMemory와 memcpy

```cpp
void* mapped = resource.memory.mapMemory(0, bufferSize);                    // ① 문 열기 (주소 받기)
std::memcpy(mapped, values.data(), static_cast<std::size_t>(bufferSize));   // ② 실제로 값 넣기 ★
resource.memory.unmapMemory();                                              // ③ 문 닫기
```

| 단계 | 하는 일 | 데이터 이동 |
|---|---|---|
| ① `mapMemory` | Vulkan 메모리를 CPU가 쓸 수 있는 주소(`mapped`)로 연결 | 없음 |
| ② `memcpy` | `values`의 32바이트를 `mapped` 주소로 복사 | **여기서 넣음** |
| ③ `unmapMemory` | CPU 주소 연결을 닫음 (데이터는 그대로 남음) | 없음 |

> 비유: `mapMemory` = 창고 문 열고 입구 주소 받기, `memcpy` = 물건 옮겨 넣기, `unmapMemory` = 문 닫기

`writeAndVerifyBuffer()`는 이걸 두 번 해요.

```
① 쓰기:  CPU 배열 values ──memcpy──▶ 버퍼 메모리
② 읽기:  버퍼 메모리     ──memcpy──▶ CPU 배열 readback   (memcpy 앞뒤가 바뀜)
③ 비교:  readback == values ?
```

## 06. 빌드와 실행 결과

```sh
cd ~/Cross-Platform-AI/examples/05-buffers-memory
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target buffers_memory_example
./build/main
```

```
Buffer memory: size 32, alignment 16, type 3
Buffer memory: size 32, alignment 16, type 3
Buffer memory: size 32, alignment 16, type 3
Input A: 0 1 2 3 4 5 6 7
Input B: 10 11 12 13 14 15 16 17
Output (initial): 0 0 0 0 0 0 0 0
Host-visible storage buffers ready: 3 x 32 bytes.
CPU write/readback verified. No GPU dispatch yet.
Buffers and memory destroyed.
```

| 출력 | 뜻 |
|---|---|
| `size 32` | 드라이버가 요구한 크기가 요청한 32바이트와 같음 |
| `alignment 16` | 시작 위치는 16의 배수여야 함. 오프셋 0이라 만족 |
| `type 3` | Tesla T4에서 `HostVisible + HostCoherent`를 갖춘 메모리 종류 번호 |
| `Input A` / `Input B` | CPU가 써 넣은 값을 다시 읽어 보니 그대로 |
| `Output (initial)` | `{}`로 초기화해서 전부 0. 나중에 GPU가 결과를 씀 |
| `No GPU dispatch yet.` | 이번 장은 CPU에서 쓰고 읽기까지 |
| `Buffers and memory destroyed.` | RAII가 버퍼·메모리를 `device`보다 먼저 정리 |
