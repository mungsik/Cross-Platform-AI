## 1. invocation, workgroup, dispatch가 각각 무엇인가요? (CUDA에서 무엇과 대응되나요?)

| Vulkan 용어 | 한국어 | 뜻 | CUDA 대응 |
|---|---|---|---|
| invocation | 인보케이션 | 셰이더 `main()`이 한 번 실행되는 단위입니다. 이 예제에서는 원소 하나를 맡습니다. | thread (스레드) |
| workgroup | 워크그룹 | invocation 묶음입니다. 크기는 셰이더의 `local_size_x`로 정합니다. | thread block (블록) |
| dispatch | 디스패치 | workgroup 개수를 넘겨 셰이더를 실행시키는 명령입니다. 커맨드 버퍼에 기록됩니다. | kernel launch `kernel<<<grid, block>>>` |

셰이더 내장 변수(built-in variable)는 CUDA 변수와 이렇게 대응합니다.

| GLSL | CUDA |
|---|---|
| `local_size_x` (셰이더에 적음) | `blockDim.x` (실행할 때 지정) |
| `dispatch(groupCount, 1, 1)`의 `groupCount` | `gridDim.x` |
| `gl_LocalInvocationID` | `threadIdx` |
| `gl_WorkGroupID` | `blockIdx` |
| `gl_GlobalInvocationID` | `blockIdx * blockDim + threadIdx` |
| `shared` 변수 / `barrier()` | `__shared__` / `__syncthreads()` |

이 예제에서는 `local_size_x = 64`, 원소가 8개이므로 `groupCount = (8 + 63) / 64 = 1`입니다. invocation 64개 중 8개만 계산하고 나머지는 `if (index >= params.count) return;`에서 바로 끝납니다.

CUDA와 다른 점은 블록 크기를 정하는 위치입니다. CUDA는 실행할 때 블록 크기를 지정하지만, Vulkan은 셰이더 안에 적습니다. 그래서 파이프라인을 만들 때 이미 고정됩니다.

## 2. subgroup(warp)이란? 크기는 어디서 정하나?

- subgroup은 workgroup 안에서 하드웨어가 실제로 한 번에 같은 명령을 동시에 실행하는 invocation 묶음
- workgroup 크기는 개발자가 셰이더의 `local_size_x`로 정함
- subgroup 크기는 GPU 하드웨어와 드라이버가 정합니다(물리적 묶음). 개발자는 값을 조회만 합니다.

| GPU | 일반적인 subgroup 크기 |
|---|---|
| NVIDIA (우리 T4 포함) | 32 |
| AMD | 64 (RDNA는 32/64) |
| Intel | 8 / 16 / 32 |
| 모바일 (Mali, Adreno 등) | 4~128로 다양함 |

조회 방법은 두 가지입니다.

- C++: `vk::PhysicalDeviceSubgroupProperties::subgroupSize` (`getProperties2()`로 얻음)
- 셰이더: `gl_SubgroupSize` (`GL_KHR_shader_subgroup_basic` 확장 필요)

Vulkan 1.3(`VK_EXT_subgroup_size_control`)부터는 하드웨어가 지원하는 범위 안에서 크기를 요청할 수 있습니다. 그래도 범위 자체는 하드웨어가 정합니다.

T4에서 `local_size_x = 64`이면 workgroup 하나가 warp 2개(32 × 2)로 실행됩니다. 그래서 보통 workgroup 크기를 subgroup 크기의 배수(32, 64, 128 등)로 잡습니다. 배수가 아니면 마지막 warp의 일부 레인(lane)이 놀게 됩니다.

## 3. 펜스, 배리어, 세마포어란? 배리어와 세마포어는 어떻게 다른가?

| 이름 | 한국어 | 누가 누구를 기다리나 | 범위 |
|---|---|---|---|
| fence | 펜스 | CPU가 GPU 제출이 끝나기를 기다림 | `submit` 한 번 전체 |
| pipeline barrier | 파이프라인 배리어 | GPU 명령이 같은 큐의 앞 명령을 기다림 | 커맨드 버퍼 안의 명령 사이 |
| semaphore | 세마포어 | GPU의 한 `submit`이 다른 `submit`을 기다림 | 제출 단위, 큐 사이도 가능 |

### 배리어와 세마포어의 차이

| | 배리어 | 세마포어 |
|---|---|---|
| 기록 위치 | 커맨드 버퍼 안 (`cmd.pipelineBarrier`) | `submit` 정보 (`waitSemaphores` / `signalSemaphores`) |
| 연결하는 대상 | 같은 큐에서 앞 명령 → 뒤 명령 | 앞 `submit` → 뒤 `submit` (다른 큐 사이도 가능) |
| 대상 지정의 세밀함 | 버퍼 범위·단계·접근 종류까지 지정 | 제출 단위 전체를 신호(signal)로 연결 |
| 예 | 13장 디스패치 1 → 디스패치 2 (`intermediate`) | 컴퓨트 큐 결과 → 그래픽스 큐 렌더링, 스왑체인 이미지 획득 → 렌더링 |

파이썬 비유로 보면 다음과 같습니다.

- 배리어: 한 함수 안에서 `t = add(a, b)` 다음에 `out = add(t, b)`가 `t`를 다 쓴 뒤에 읽도록 순서를 보장합니다.
- 세마포어: 두 작업(프로세스)이 `Event`로 "끝났다" 신호를 주고받습니다. CPU는 관여하지 않습니다.
- 펜스: 메인 스레드가 `future.result()`로 결과가 나올 때까지 기다립니다.

이 튜토리얼은 큐 하나에 `submit`도 한 번이라 세마포어가 필요 없었습니다. 참고로 Vulkan 1.2의 timeline semaphore는 CPU에서도 기다리거나 신호를 줄 수 있어서 펜스 역할 일부를 겸합니다.

## 4. 배리어에서 PipelineStage와 AccessMask란?

배리어는 두 가지를 보장해야 하고, 각각을 다른 값으로 지정합니다.

| 이름 | 한국어 | 지정하는 것 | 보장하는 것 |
|---|---|---|---|
| PipelineStage (`srcStageMask` / `dstStageMask`) | 파이프라인 단계 | **언제**: 어떤 단계의 작업이 끝나야 어떤 단계가 시작할 수 있는지 | 실행 의존성(execution dependency): 순서 |
| AccessMask (`srcAccessMask` / `dstAccessMask`) | 접근 마스크 | **무엇을**: 어떤 종류의 메모리 쓰기를 어떤 종류의 읽기에 보여 줄지 | 메모리 의존성(memory dependency): 캐시를 비우고 무효화해서 값이 보이게 함 |

PipelineStage만 맞으면 순서는 지켜집니다. 하지만 앞 단계가 쓴 값이 아직 그 단계의 캐시에 남아 있으면 뒤 단계가 이전 값을 읽을 수 있습니다. AccessMask가 "이 쓰기를 → 이 읽기에 보이게 하라"를 지정해서 이 문제를 막습니다.

- `src` = 앞쪽(기다림을 당하는 쪽), `dst` = 뒤쪽(기다리는 쪽)
- stage는 `pipelineBarrier()`의 인자로 넘기고, access는 `BufferMemoryBarrier` 구조체에 넣습니다.
