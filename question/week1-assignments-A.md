# 1주차 질문과 답변

원본 질문: [week1-assignments-Q.md](week1-assignments-Q.md)

---

## 질문 1. — 03~04장 · Queue와 QueueFamily

**질문**

> Queue가 정확히 무엇인가요? Queue와 QueueFamily의 관계는?

**답변**

**Queue (큐)** 는 CPU가 GPU에게 일을 **제출(submit)하는 창구**예요. CPU는 할 일을 커맨드 버퍼(command buffer)에 녹화해서 큐에 넣고, GPU는 큐에 들어온 순서대로 꺼내 실행해요.

**QueueFamily (큐 패밀리)** 는 **할 수 있는 일(그래픽·컴퓨트·전송 등)이 똑같은 큐들의 묶음**이에요. GPU 안의 하드웨어 엔진 구성을 그대로 보여 줘요.

| | 비유 (은행) | 코드 |
|---|---|---|
| Queue | 창구 하나 | `vk::raii::Queue computeQueue` |
| QueueFamily | 같은 업무를 보는 창구들의 구역 | `computeQueueFamilyIndex` (구역 번호) |

**관계**

- 큐는 반드시 어느 한 패밀리에 속해요. 그래서 큐를 가리킬 때는 `(패밀리 번호, 그 안의 큐 번호)` 두 개로 지정해요. 04장의 `vk::raii::Queue(device, computeQueueFamilyIndex, 0)`이 "0번 패밀리의 0번 큐"예요.
- 큐는 우리가 따로 만들지 않아요. **논리 장치를 만들 때** `vk::DeviceQueueCreateInfo`로 "몇 번 패밀리에서 큐 몇 개"를 요청하면 함께 생기고, 그다음 꺼내 오기만 해요.
- 03장에서 `getQueueFamilyProperties()`로 패밀리 목록을 보고, 컴퓨트를 지원하는 패밀리를 골랐어요.

**이 서버의 Tesla T4** (`vulkaninfo`로 확인)

| 패밀리 | 큐 개수 | 할 수 있는 일 |
|---|---|---|
| 0 | 16 | 그래픽 + 컴퓨트 + 전송 ← 03장에서 고른 것 |
| 1 | 2 | 전송 |
| 2 | 8 | 컴퓨트 + 전송 |
| 3 | 2 | 전송 + 영상 디코딩 |
| 4 | 1 | 전송 + 영상 인코딩 |

**추가**

- 하드웨어에 따라 큐 개수는 다르다. 패밀리 구성과 큐 최대 개수는 정해져있음
- 큐 개수가 실제 하드웨어 엔진 개수와 꼭 같지는 않음. 0번 패밀리의 큐 16개가 계산 엔진 16개라는 뜻이 아니고, 드라이버가 일을 받는 창구를 16개 열어 둔 것. 
- 그래서 큐를 많이 요청한다고 무조건 빨라지지 않음

---

## 질문 2. — 05장 · 버퍼 생성, 메모리 할당, `bindMemory()`

**질문**

> 버퍼 생성, 메모리 할당, `bindMemory()`는 각각 무엇을 하나요?

**답변**

| 단계 | 코드 | 하는 일 | 비유 |
|---|---|---|---|
| 버퍼 생성 | `vk::raii::Buffer(device, bufferInfo)` | **크기와 용도**(예: `eStorageBuffer`)만 정한 핸들을 만듦. 아직 데이터를 담을 메모리는 없음 | 집 설계도 |
| 메모리 할당 | `vk::raii::DeviceMemory(device, allocInfo)` | `getMemoryRequirements()`로 버퍼가 요구하는 크기·정렬·가능한 메모리 유형을 확인하고, 그중 원하는 속성의 유형(`findMemoryType()`)에서 **실제 메모리**를 확보 | 땅 |
| `bindMemory()` | `buffer.bindMemory(*memory, 0)` | 버퍼를 그 메모리의 특정 위치(offset)에 **연결**. 이제 버퍼를 통해 그 메모리를 읽고 쓸 수 있음 | 땅 위에 집 짓기 |

- 버퍼와 메모리를 나눈 이유: GPU 메모리에는 종류(CPU가 볼 수 있는 것, GPU 전용으로 빠른 것 등)가 여러 개라서, 어떤 메모리를 쓸지 프로그래머가 고르게 하려고요. 큰 메모리 하나를 할당해서 여러 버퍼를 서로 다른 offset에 연결하는 것도 가능해요.
- 한 버퍼의 `bindMemory()`는 **한 번만** 할 수 있고 나중에 다른 메모리로 바꿀 수 없어요.

**추가**

- 버퍼는 최대 크기로 잡아놓고 일부만 씀. kv cache와 유사
- 예를 들어, T4 GPU를 `vulkaninfo` 로 확인해보면 다음과 같음

│         한계          │  T4 값   │                                뜻                                │
├───────────────────────┼──────────┼──────────────────────────────────────────────────────────────────┤
│ VRAM (DeviceLocal 힙) │ 15 GiB   │ 전체 메모리                                                      │
├───────────────────────┼──────────┼──────────────────────────────────────────────────────────────────┤
│ maxStorageBufferRange │ 약 4 GiB │ 스토리지 버퍼 descriptor 하나가 볼 수 있는 최대 범위             │
├───────────────────────┼──────────┼──────────────────────────────────────────────────────────────────┤
│ maxUniformBufferRange │ 64 KiB   │ 유니폼 버퍼 descriptor 하나의 최대 범위 (작은 설정값용이라 작음) │
├───────────────────────┼──────────┼──────────────────────────────────────────────────────────────────┤
│ maxBufferSize         │ 1 TiB    │ 버퍼 하나의 최대 크기 (실제로는 VRAM이 먼저 모자람)

- 버퍼와 메모리 연결

──────┬───────────────────────────────────────────────────┬──────────────────────────────────────────┐
│ 순서 │         코드 (05-buffers-memory/main.cpp)         │                 하는 일                  │
├──────┼───────────────────────────────────────────────────┼──────────────────────────────────────────┤
│      │                                                   │ 원하는 크기(32바이트)와                  │
│ 1    │ vk::raii::Buffer(device, bufferInfo)              │ 용도(eStorageBuffer)로 버퍼 생성. 아직   │
│      │                                                   │ 메모리 없음                              │
├──────┼───────────────────────────────────────────────────┼──────────────────────────────────────────┤
│ 2    │ getMemoryRequirements() (260줄)                   │ 드라이버에게 "이 버퍼를 올리려면 어떤    │
│      │                                                   │ 메모리가 필요해?"라고 물음               │
├──────┼───────────────────────────────────────────────────┼──────────────────────────────────────────┤
│      │ findMemoryType(requirements.memoryTypeBits,       │ 허용된 유형 중 원하는 속성(eHostVisible  │
│ 3    │ properties, ...) (264줄)                          │ | eHostCoherent)을 가진 유형을 고름 →    │
│      │                                                   │ T4에서는 3번                             │
├──────┼───────────────────────────────────────────────────┼──────────────────────────────────────────┤
│ 4    │ allocationSize = requirements.size (270줄)        │ 드라이버가 알려 준 크기로 메모리 할당    │
├──────┼───────────────────────────────────────────────────┼──────────────────────────────────────────┤
│ 5    │ bindMemory(*memory, 0)                            │ 버퍼와 메모리 연결

- `메모리` 란 device memory를 말함. 코드에서는 `vk::raii::DeviceMemory` 타입.
    - GPU가 접근할 수 있는 실제 저장 공간. 즉, 바이트들이 실제로 놓이는 자리. 이게 물리적으로 어디에 있는지는 고른 메모리 유형에 따라 다름.
        - `eDeviceLocal` 유형 -> 그래픽카드의 VRAM

- 버퍼마다 필요한 메모리는 다름. --> `getMemoryRequirements()`
    - 크기 : 1KB 버퍼와 4GB 버퍼는 필요한 바이트 수가 다름

---

## 질문 3. — 05장 · `HostVisible`, `HostCoherent`, `DeviceLocal`

**질문**

> `HostVisible`, `HostCoherent`, `DeviceLocal`은 각각 무엇을 보장하나요?
> `HostVisible`과 `DeviceLocal`을 동시에 가질 수 있나요?

**답변**

| 속성 | 보장하는 것 |
|---|---|
| `eHostVisible` (호스트 가시) | CPU(host)가 `mapMemory()`로 이 메모리를 **직접 읽고 쓸 수 있다** | 
| `eHostCoherent` (호스트 일관성) | CPU 쪽 캐시 때문에 생기는 불일치를 드라이버·하드웨어가 처리해 준다. 그래서 CPU가 쓴 값을 GPU에 보이게 하려고 `flushMappedMemoryRanges()`, GPU가 쓴 값을 CPU에 보이게 하려고 `invalidateMappedMemoryRanges()`를 **따로 호출하지 않아도 된다** |
| `eDeviceLocal` (장치 로컬) | GPU가 접근하기에 **가장 효율적인** 메모리다 (외장 GPU에서는 보통 VRAM) |

**`HostVisible`과 `DeviceLocal`을 동시에 가질 수 있나요? → 네, 가능해요.**

이 서버의 `vulkaninfo` 결과에 실제로 있어요.

| 장치 | 메모리 유형 | 속성 | 힙 크기 |
|---|---|---|---|
| Tesla T4 | 3번 (05장에서 쓴 것) | HostVisible + HostCoherent | 10.97 GiB (CPU 쪽 RAM) |
| Tesla T4 | 1, 2번 | DeviceLocal | 15 GiB (VRAM) |
| Tesla T4 | **5번** | **DeviceLocal + HostVisible** + HostCoherent | **246 MiB** |
| llvmpipe (CPU 소프트웨어 장치) | 0번 | DeviceLocal + HostVisible + HostCoherent + HostCached | 14.63 GiB |

- 외장 GPU(T4)에서는 VRAM 중 CPU가 PCIe로 직접 들여다볼 수 있는 **작은 창**(BAR, 여기서는 246 MiB)만 두 속성을 함께 가져요. 최근 GPU는 Resizable BAR로 이 창이 VRAM 전체까지 커지기도 해요.
- 내장 GPU나 llvmpipe처럼 CPU와 메모리를 함께 쓰는 장치는 대부분의 메모리가 두 속성을 모두 가져요.

**추가**

- `mapMemory()`의 장점
    - map(매핑) : GPU 쪽 메모리를 CPU의 주소 공간에 연결해서 보이게 하는 것
    - `mapMemory()`가 돌려주는 `void* mapped`: CPU가 그 메모리를 가리키는 포인터
        - `void* mapped` : `mapMemory()` 가 `void` 타입의 값(주소)를 돌려주고, `mapped` 라는 이름의 변수에 받아둔 것
        - `void*`(void pointer) : 타입 없는 포인터, 범용 포인터. 주소는 있는데 거기 무슨 타입이 있는지는 모름.
        - Vulkan 메모리는 그냥 바이트 덩어리이기 때문에 `float`을 넣을지, `int`를 넣을지 프로그래머가 선택
    - CPU ──memcpy──▶ HostVisible 메모리 ◀── GPU가 읽음

- DeviceLocal이란?
    - 장치 로컬. 여기서 device = GPU, local = 가까운

외장 GPU (T4, 이 서버): 메모리가 둘로 나뉘어 있어요

┌──── 메인보드 ────┐            ┌──── 그래픽카드 (T4) ────┐
│ CPU              │            │ GPU                      │
│ RAM (10.97 GiB)  │ ◀─ PCIe ─▶ │ VRAM (15 GiB)            │
│  ↑ HostVisible   │            │  ↑ DeviceLocal           │
└──────────────────┘            └──────────────────────────┘
- DeviceLocal = 그래픽카드 위의 VRAM만이에요. CPU RAM과는 물리적으로 다른 칩이에요.
- CPU RAM은 DeviceLocal이 아니에요. 05장 3번 유형이 이 RAM이고, GPU는 PCIe를 건너서 읽어요(느림).
- 5번 유형(246 MiB)은 VRAM의 일부를 CPU에게 창문처럼 열어 준 것이에요. CPU RAM을 쓰는 게 아니라, CPU가 PCIe 너머의 VRAM을 들여다보는 거예요.

내장 GPU (노트북, 휴대폰, Apple M 칩): 메모리가 하나예요

┌──────── 칩 하나 ────────┐
│ CPU        GPU          │
│   ╲        ╱            │
│    RAM (공유)           │
│  ↑ DeviceLocal + HostVisible │
└─────────────────────────┘
- 여기는 VRAM이 따로 없어요. CPU와 GPU가 같은 RAM을 써요.
- 그 RAM이 GPU에게 가장 가까운 메모리라 DeviceLocal이 붙고, CPU도 볼 수 있으니 HostVisible도 붙어요.

---

## 질문 4. — 05장 · `mapMemory` / `memcpy` / `unmapMemory`

**질문**

> 다음 세 줄이 각각 하는 일을 설명해주세요.
> ```
> void* mapped = memory.mapMemory(0, bufferSize);
> std::memcpy(mapped, values.data(), bufferSize);
> memory.unmapMemory();
> ```
> - `unmapMemory()` 이후에도 기록한 데이터가 남아 있나요?
> - `HostCoherent`이면 GPU가 계산한 결과를 CPU에서 언제든 읽어도 되나요?

**답변**

```cpp
void* mapped = memory.mapMemory(0, bufferSize); // 1. 주소 받기
std::memcpy(mapped, values.data(), bufferSize); // 2. 그 주소로 값 쓰기
memory.unmapMemory(); // 3. 닫기. `mapped` 는 이제 쓰면 안됨
```

| 줄 | 하는 일 |
|---|---|
| `mapMemory(0, bufferSize)` | Vulkan 메모리의 0바이트부터 `bufferSize`(32바이트)까지를 **CPU 주소 공간에 연결(매핑)** 하고, 그 시작 주소를 `void*`(타입 없는 포인터)로 돌려줌 |
| `std::memcpy(mapped, values.data(), bufferSize)` | CPU 배열 `values`의 첫 칸 주소에서 32바이트를 `mapped` 주소로 **바이트 그대로 복사**. `mapped`가 Vulkan 메모리와 연결돼 있으니 실제로는 그 메모리에 값이 써짐. |
| `unmapMemory()` | 매핑을 해제. 이제 `mapped` 포인터는 **쓰면 안 됨** (가리키던 연결이 끊김). Vulkan은 한 메모리를 동시에 두 번 map 하는 걸 허락하지 않기 때문에 미리 정리. |

파이썬으로 치면 `np.frombuffer`로 GPU 메모리를 들여다보는 창을 열고 → 값을 복사하고 → 창을 닫는 것과 같아요.

- `memcpy`
    - memory copy
    - `std::memcpy(받을 곳, 보낼 곳, 바이트 수)
        - 쓰기 : `std::memcpy(mapped, values.data(), bufferSize)` -> `values`에서 GPU 메모리로
        - 읽기 : `std::memcpy(readback.data(), mapped, bufferSize)`-> GPU 메모리에서 `readback`으로
- `values` : 코드에서 임의로 정한 값. `values = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0}` 
- `values.data()` 리턴값
    values (std::array<float, 8>)
 [0.0][1.0][2.0][3.0][4.0][5.0][6.0][7.0]
  ↑
  values.data() → 이 칸의 주소 (const float*)


**`unmapMemory()` 이후에도 기록한 데이터가 남아 있나요? → 네, 남아 있어요.**
unmap은 CPU가 들여다보던 **창(주소 연결)을 닫는 것**일 뿐이고, 메모리 자체(`vk::raii::DeviceMemory`)는 해제될 때까지 그대로 있어요. 05장에서 unmap한 뒤 다시 map해서 읽어 보니 같은 값이 나왔던 것(`readback == values`)이 그 증거예요. 메모리가 `HostCoherent`이므로 unmap 전에 flush를 하지 않아도 GPU가 이 값을 볼 수 있어요.

**`HostCoherent`이면 GPU가 계산한 결과를 CPU에서 언제든 읽어도 되나요? → 아니요.**
`HostCoherent`는 **캐시 불일치**(flush/invalidate) 문제만 없애 줄 뿐, **시간 순서**는 보장하지 않아요.

- GPU 작업은 CPU와 **비동기**로 돌아가요. 큐에 제출한 직후 읽으면 GPU가 아직 계산 중이라 0이나 중간값이 읽힐 수 있어요.
- 그래서 GPU 작업이 **끝났다는 걸 확인한 뒤**에 읽어야 해요: 제출할 때 fence를 걸고 `waitForFences()`로 기다리기
- 정석대로라면 커맨드 버퍼 안에 "컴퓨트 셰이더의 쓰기 → 호스트(CPU)의 읽기"로 넘어가는 메모리 장벽(pipeline barrier, `eHostRead`)도 넣어서, GPU가 쓴 내용이 CPU에 보이도록 해요.
- 덧붙여, `HostCoherent`라고 해서 CPU가 읽기에 빠른 것도 아니에요. CPU 읽기가 빠른 건 `HostCached` 메모리예요 (T4의 4번 유형).

---

## 질문 5. — 06~07장 · 네 가지 타입 설정

**질문**

> 다음 설정은 각각 무엇을 선언하나요? 어느 것끼리 일치해야 하고, 어느 것은 달라도 되나요?
>
> ```
> 버퍼 생성의 usage
> 레이아웃 binding의 descriptorType
> 풀의 poolSize.type
> 갱신 요청의 write.descriptorType
> ```

**답변**

| 설정 | 무엇을 선언하나 |
|---|---|
| 버퍼 생성의 `usage` (`vk::BufferUsageFlagBits::eStorageBuffer`) | 이 **버퍼가 어떤 용도로 쓰일 수 있는지** 허락 목록. 비트 플래그라 여러 개를 `\|`로 함께 줄 수 있음 (예: `eStorageBuffer \| eTransferDst`) |
| layout binding의 `descriptorType` | layout의 그 **칸(binding)에 어떤 종류의 descriptor가 오는지** (06장) |
| 풀의 `poolSize.type` | 풀(창고)이 **어떤 종류의 descriptor를 몇 개까지** 떼어 줄 수 있는지, 종류별 용량 (07장) |
| 갱신 요청의 `write.descriptorType` | 지금 셋의 그 칸에 **써 넣는 descriptor의 종류** (07장) |

**반드시 일치해야 하는 것**

1. **`write.descriptorType` = 그 칸의 layout `descriptorType`.** 칸 규칙이 스토리지 버퍼인데 다른 종류를 써 넣으면 안 돼요 (검증 레이어가 에러를 냄).
2. **풀에는 layout이 쓰는 종류가 있어야 하고, 용량도 충분해야 해요.** 칸 종류가 스토리지 버퍼인데 풀에 그 종류의 용량이 없거나 모자라면 셋 할당이 실패해요 (`eErrorOutOfPoolMemory`).
3. **버퍼 `usage`에 그 descriptor 종류에 맞는 플래그가 들어 있어야 해요.** 스토리지 버퍼 descriptor로 연결할 버퍼는 `usage`에 `eStorageBuffer`가 있어야 해요.

**달라도 되는 것**

- 버퍼 `usage`에는 **다른 플래그가 더 있어도** 돼요 (`eTransferDst` 등). 필요한 플래그를 "포함"하기만 하면 돼요.
- 풀에는 **다른 종류의 용량이 더 있어도** 되고, 용량이 layout이 쓰는 양보다 **커도** 돼요.
- **개수는 서로 같지 않아도** 돼요. layout의 `descriptorCount`는 칸 하나당 개수(1)이고, `poolSize.descriptorCount`는 창고 전체 용량(3)이에요.

정리: 종류의 이름은 `usage`(버퍼) ⊇ layout = write, 그리고 풀은 그 종류를 충분히 담을 수 있어야 해요.

### 예시 1. 우리 코드에서 네 설정이 있는 곳 (`examples/07-descriptor-sets/main.cpp`)

```cpp
// ① 버퍼 생성 (05장, 254줄) — "이 버퍼는 스토리지 버퍼로 연결해도 된다"
bufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer;

// ② layout의 칸 규칙 (06장, 322줄) — "0·1·2번 칸에는 스토리지 버퍼 descriptor가 온다"
bindings[i].descriptorType = vk::DescriptorType::eStorageBuffer;

// ③ 풀의 용량 (07장, 337~338줄) — "창고에 스토리지 버퍼 descriptor 3개분이 있다"
poolSize.type = vk::DescriptorType::eStorageBuffer;
poolSize.descriptorCount = 3;

// ④ 셋에 써 넣기 (07장, 384줄) — "i번 칸에 스토리지 버퍼 descriptor를 쓴다"
writes[i].descriptorType = vk::DescriptorType::eStorageBuffer;
```

네 곳 모두 `eStorageBuffer`라서 문제없이 통과했어요. 다만 ①은 타입이 `vk::BufferUsageFlagBits`(버퍼 용도 플래그)이고, ②~④는 `vk::DescriptorType`(descriptor 종류)이에요. 이름은 비슷하지만 **서로 다른 enum**이에요. ①은 **버퍼**에 대한 허락이고, ②~④는 **descriptor**(연결 쪽지)에 대한 설정이에요.

### 예시 2. 빵 공장 비유로 보면

| 설정 | 비유 |
|---|---|
| ① 버퍼 `usage` | 재료 통에 붙은 **인증 스티커**: "이 통은 밀가루 투입구에 연결해도 됨" |
| ② layout `descriptorType` | 기계 설계도의 **투입구 모양**: "0번 구멍은 밀가루 호스 전용" |
| ③ 풀 `poolSize.type` | 창고에 쌓아 둔 **호스 재고**: "밀가루 호스 3개" |
| ④ write `descriptorType` | 실제로 연결할 때 꺼낸 **호스 종류**: "밀가루 호스로 0번 구멍에 연결" |

- 구멍 모양(②)과 꽂는 호스(④)가 다르면 안 맞아요.
- 창고(③)에 그 호스가 없으면 꺼낼 수 없어요.
- 통(①)에 인증 스티커가 없으면 그 호스로 연결하면 안 돼요.
- 통에 스티커가 여러 장 붙어 있거나(①에 플래그가 더 있음), 창고에 다른 호스까지 넉넉히 있는 건(③ 용량이 더 큼) 괜찮아요.

### 예시 3. 하나만 틀리면 어떻게 되나

| 틀린 곳 | 바꾼 코드 | 결과 |
|---|---|---|
| ④가 ②와 다름 | `writes[i].descriptorType = eUniformBuffer;` | `updateDescriptorSets()`에서 **검증 레이어 에러**. 칸은 스토리지 버퍼용인데 유니폼 버퍼를 쓰려 함 |
| ③에 그 종류가 없음 | `poolSize.type = eUniformBuffer;` (스토리지 버퍼 용량 0) | `allocateDescriptorSets()`가 **실패** (`eErrorOutOfPoolMemory`). 창고에 맞는 재고가 없음 |
| ③ 용량이 모자람 | `poolSize.descriptorCount = 2;` | 셋 하나에 3개가 필요한데 2개뿐이라 **할당 실패** |
| ①에 플래그가 없음 | `bufferInfo.usage = eUniformBuffer;` (스토리지 플래그 없음) | `updateDescriptorSets()`에서 **검증 레이어 에러**. 스토리지 버퍼로 쓰겠다고 허락받지 않은 버퍼를 연결하려 함 |

검증 레이어가 꺼져 있으면 ①·④가 틀려도 에러 없이 지나갈 수 있어요. 그러면 셰이더가 엉뚱하게 동작하거나 GPU가 멈출 수도 있어요. 그래서 Debug 빌드에서는 검증 레이어를 켜 두는 거예요 (02장).

### 예시 4. 달라도 되는 경우 (모두 정상)

```cpp
// ① 플래그가 더 있음: 스토리지 버퍼로도 쓰고, 복사 명령의 목적지로도 씀 (스테이징 복사용)
bufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer
                 | vk::BufferUsageFlagBits::eTransferDst;

// ③ 종류도, 용량도 더 많음: 지금 셋은 스토리지 버퍼 3개만 쓰지만 여유를 둠
std::array<vk::DescriptorPoolSize, 2> poolSizes{};
poolSizes[0].type = vk::DescriptorType::eStorageBuffer;
poolSizes[0].descriptorCount = 10;   // 3개 쓰고 7개 남음
poolSizes[1].type = vk::DescriptorType::eUniformBuffer;
poolSizes[1].descriptorCount = 2;    // 지금은 안 씀
poolInfo.poolSizeCount = static_cast<std::uint32_t>(poolSizes.size());
poolInfo.pPoolSizes = poolSizes.data();
```

②와 ④는 `eStorageBuffer` 그대로 두면 돼요.

### 예시 5. 칸 종류가 섞인 경우 (질문 6 상황 5와 연결)

binding 3에 유니폼 버퍼(설정값)를 추가하면, 칸마다 네 설정이 이렇게 맞춰져야 해요.

| 칸 | ① 연결할 버퍼의 `usage` | ② layout `descriptorType` | ④ write `descriptorType` |
|---|---|---|---|
| 0 (A) | `eStorageBuffer` 포함 | `eStorageBuffer` | `eStorageBuffer` |
| 1 (B) | `eStorageBuffer` 포함 | `eStorageBuffer` | `eStorageBuffer` |
| 2 (C) | `eStorageBuffer` 포함 | `eStorageBuffer` | `eStorageBuffer` |
| 3 (설정값) | **`eUniformBuffer`** 포함 | **`eUniformBuffer`** | **`eUniformBuffer`** |

③ 풀은 칸 단위가 아니라 **창고 전체**라서 종류별로 합쳐요.

```cpp
poolSizes[0].type = vk::DescriptorType::eStorageBuffer;  poolSizes[0].descriptorCount = 3;  // 0·1·2번 칸
poolSizes[1].type = vk::DescriptorType::eUniformBuffer;  poolSizes[1].descriptorCount = 1;  // 3번 칸
```

이렇게 보면 ①·②·④는 **칸 하나하나**에 대한 약속이고, ③은 **전체 재고**에 대한 약속이라는 차이가 잘 보여요.

---

## 질문 6. — 06~07장 · 상황별 처리

**질문**

> 다음 각 상황에서 **buffers, descriptor set, descriptor pool, descriptor set layout**을 각각 어떻게 처리해야 할지 설명해주세요.

**답변**

먼저 기억할 원칙:

- **descriptor set은 데이터를 담지 않고 버퍼를 가리키기만** 해요 (참조).
- **layout은 칸 구조**(번호·종류·개수)만 정하고, 버퍼 크기나 어떤 버퍼인지는 몰라요.
- **풀은 셋을 떼어 주는 창고**이고, 용량(셋 수, 종류별 descriptor 수)이 정해져 있어요.

### 상황 1 — 같은 버퍼에 새 숫자만 넣고 다시 계산

> A·B 버퍼에 담긴 숫자를 더해 C 버퍼에 결과를 저장했습니다. 이제 A·B에 새로운 숫자를 넣고 같은 계산을 다시 실행하려 합니다.
>
> 데이터의 개수와 자료형은 같으며, 기존 버퍼의 같은 영역을 사용합니다. 무엇을 변경해야 하나요?

**답변**

| 대상 | 처리 |
|---|---|
| buffers | **A·B의 내용만 새로 씀** (`mapMemory` → `memcpy` → `unmapMemory`) |
| descriptor set | 그대로 (이미 A·B·C를 가리키고 있음) |
| descriptor pool | 그대로 |
| descriptor set layout | 그대로 |

셋은 버퍼를 가리킬 뿐이라 버퍼 **안의 값**이 바뀌어도 다시 쓸 필요가 없어요. 단, 이전 계산이 끝난 뒤(fence 대기 후)에 덮어써야 해요. GPU가 아직 A·B를 읽는 중에 바꾸면 결과가 섞여요.

### 상황 2 — A 대신 A′를 쓰기 (기존 연결은 보존하지 않아도 됨)

> A·B를 더하는 대신, 이미 만들어져 있는 A′ 버퍼와 B 버퍼를 더해 C에 저장하려 합니다. A′의 크기와 자료형은 A와 같습니다.
>
> 기존 descriptor set에는 A·B·C가 연결되어 있습니다. 앞으로 A를 사용할 필요는 없으며, 기존 연결 정보를 보존하지 않아도 됩니다. 새 descriptor set을 할당해야 할까요? 어떻게 처리할 수 있나요?

**답변**

| 대상 | 처리 |
|---|---|
| buffers | A′는 이미 있음. A는 더 이상 필요 없으니 GPU가 다 쓴 뒤 정리해도 됨 |
| descriptor set | **새로 할당할 필요 없음.** 기존 셋의 **0번 칸만** A′로 다시 씀 (`WriteDescriptorSet` 하나: `dstBinding = 0`, `pBufferInfo` → A′) 후 `updateDescriptorSets()`. 1·2번(B·C)은 그대로 둠 |
| descriptor pool | 그대로 |
| descriptor set layout | 그대로 (칸 구조가 같음) |

주의: 셋을 다시 쓰는 건 **GPU가 그 셋을 쓰는 작업이 끝난 뒤**에 해야 해요. 그리고 그 셋을 바인딩해 둔 커맨드 버퍼는 셋이 갱신되면 무효가 되므로 다시 녹화해야 해요.

### 상황 3 — 두 계산을 번갈아, 셋 내용을 매번 다시 쓰지 않기

> 다음 두 계산을 번갈아 실행하려 합니다.
>
> ```
> C₀ = A₀ + B₀
> C₁ = A₁ + B₁
> ```
>
> 여섯 버퍼는 모두 준비되어 있지만, descriptor set은 A₀·B₀·C₀를 연결한 하나만 있습니다. 두 계산은 같은 셰이더와 같은 binding 구조를 사용합니다.
>
> **두 묶음의 연결 정보를 모두 보관하고, 계산을 바꿀 때마다 descriptor set의 내용을 다시 쓰지 않으려 합니다.** 기존 pool에는 할당 여유가 있습니다.
>
> 무엇을 추가로 준비해야 하며, 실행할 때는 무엇을 선택해야 하나요?

**답변**

| 대상 | 처리 |
|---|---|
| buffers | 그대로 (여섯 개 모두 준비됨) |
| descriptor set | **같은 layout으로 셋을 하나 더 할당**해서 A₁·B₁·C₁를 써 넣음. 이제 셋 0 = (A₀, B₀, C₀), 셋 1 = (A₁, B₁, C₁) |
| descriptor pool | 그대로 (할당 여유가 있음) |
| descriptor set layout | 그대로 (같은 셰이더·같은 칸 구조라 두 셋 모두 같은 layout을 씀) |

**실행할 때:** 커맨드 버퍼에서 `bindDescriptorSets()`로 **이번에 쓸 셋(0 또는 1)을 골라** 바인딩한 뒤 dispatch해요. 파이썬으로 치면 함수(셰이더)는 그대로 두고 인자 묶음 `args0`, `args1` 중 하나를 골라 `f(*args0)` / `f(*args1)`로 부르는 것과 같아요.

### 상황 4 — pool 용량(셋 3개, StorageBuffer 6개)에서 세 번째 셋 추가

> 하나의 pool을 다음 용량으로 만들었습니다.
>
> ```
> 최대 descriptor set 수: 3
> StorageBuffer 디스크립터 총용량: 6
> ```
>
> 이 pool에서 set 두 개를 할당했고, 각 set에는 입력 둘과 출력 하나를 위한 StorageBuffer 디스크립터가 3개씩 들어 있습니다.
>
> 이제 같은 구조의 세 번째 입력·출력 묶음을 추가하려 합니다. **기존 set 두 개는 그대로 보관해야 하며**, 새 묶음의 버퍼들은 이미 준비되어 있습니다.
>
> 기존 pool에서 set을 하나 더 할당할 수 있나요? 할 수 없다면 무엇을 새로 준비해야 하나요? 기존 layout도 다시 만들어야 하나요?

**답변**

**기존 pool에서는 할당할 수 없어요.**

- 셋 개수 제한: 2개 사용 / 최대 3개 → 1개 여유 있음
- **StorageBuffer descriptor 용량: 3 × 2 = 6개 사용 / 총 6개 → 남은 게 0개**
- 세 번째 셋은 descriptor 3개가 필요하므로 할당이 실패해요 (`eErrorOutOfPoolMemory`). 제한 두 개 중 **하나라도 모자라면** 안 돼요.

| 대상 | 처리 |
|---|---|
| buffers | 그대로 (새 묶음 버퍼는 준비됨) |
| descriptor pool | **새 풀을 하나 더 만듦** (예: 최대 셋 1개 이상, StorageBuffer 3개 이상). 기존 풀을 지우고 크게 다시 만들면 거기서 받은 셋 두 개도 함께 사라지니까 안 돼요 |
| descriptor set | 새 풀에서 셋 1개 할당 → 새 묶음 버퍼 3개를 써 넣음 |
| descriptor set layout | **다시 만들 필요 없음.** layout은 특정 풀에 묶여 있지 않아서, 같은 layout으로 어느 풀에서든 셋을 할당할 수 있어요 |

### 상황 5 — 새 셰이더가 binding 3(UniformBuffer)을 추가로 요구

> 기존 셰이더는 다음 구조를 사용합니다.
>
> ```
> binding 0: StorageBuffer — A
> binding 1: StorageBuffer — B
> binding 2: StorageBuffer — C
> ```
>
> 새 셰이더는 여기에 계산 설정값을 받는 binding을 추가로 요구합니다.
>
> ```
> binding 3: UniformBuffer — 계산 설정값
> ```
>
> 설정값을 담은 버퍼는 이미 준비되어 있고, 기존 pool에는 새 set을 할당할 공간과 필요한 두 타입의 디스크립터 용량이 충분합니다.
>
> 기존 descriptor set에 write만 한 번 더 하면 될까요? 기존 layout과 set을 그대로 사용할 수 있는지, 무엇을 새로 만들어야 하는지 설명해주세요. (Pipeline 설정은 제외합니다)

**답변**

**기존 셋에 write만 한 번 더 하는 것으로는 안 돼요.**

셋의 칸 구조는 **할당할 때 쓴 layout으로 고정**돼요. 기존 layout에는 0·1·2번 칸(StorageBuffer)만 있고 3번 칸이 없어요. 없는 칸에 `dstBinding = 3`으로 쓰는 건 잘못된 사용이에요 (검증 레이어 에러). 칸을 나중에 늘릴 방법도 없어요.

| 대상 | 처리 |
|---|---|
| descriptor set layout | **새로 만듦**: binding 0·1·2 = StorageBuffer, binding 3 = UniformBuffer (모두 `eCompute`). 기존 layout은 바꿀 수 없으니 새 객체가 필요해요 |
| descriptor set | **새 layout으로 새 셋을 할당.** 새 셋은 비어 있으니 **0·1·2번(A·B·C)도 다시 써 넣고** 3번에 설정값 버퍼를 씀. 0·1·2번은 `updateDescriptorSets()`의 두 번째 인자(`vk::CopyDescriptorSet`, 07장에서 `{}`로 비워 둔 것)로 기존 셋에서 복사해 올 수도 있어요 |
| descriptor pool | 그대로 (용량이 충분하다고 했으니 기존 풀에서 할당) |
| buffers | A·B·C는 그대로. 설정값 버퍼는 `usage`에 **`eUniformBuffer`** 가 있어야 해요 |

기존 셰이더도 계속 쓴다면 기존 layout과 셋은 지우지 않고 그대로 두면 돼요. 두 셰이더가 서로 다른 layout·셋을 쓰는 거예요.
