## 1. add.comp

### add.comp

- `add.comp`에서 `comp`는 **compute shader**를 뜻함
- `add.comp`는 C++이 아니라 **GLSL**(셰이더 언어)로 작성됨. 빌드할 때 `glslc`가 SPIR-V(`add.comp.spv`)로 변환함

### GPU에서 반복을 표현하는 방식

- GLSL에도 `for`, `while` 반복문은 있음. 다른 점은 **배열 전체를 훑는 바깥 반복문을 셰이더에 쓰지 않는다**는 것
    - Python 예시

    ```python
    for index in range(count):
        result[index] = a[index] + b[index]
    ```

    - compute shader 예시
        - 반복문의 몸통만 씀. 그러면 GPU가 그 몸통을 여러 개 복사해서 동시(병렬)에 실행함
        - 바깥 반복문의 역할을 병렬 실행이 대신함. `gl_GlobalInvocationID.x`가 반복 변수 `index` 역할

    ```glsl
    void main()
    {
        uint index = gl_GlobalInvocationID.x; // 나는 몇 번째 복사본인가?
        if (index >= params.count) return;    // 배열 범위 밖이면 아무것도 안 함
        resultBuffer.values[index] = inputA.values[index] + inputB.values[index];
    }
    ```

### Invocation과 Workgroup

- 복사본 하나(실행 1회)를 **Invocation(인보케이션)** 이라고 함
- GPU는 invocation을 하나씩 띄우지 않고 묶음 단위로 띄움. 이 묶음을 **Workgroup(워크그룹)** 이라고 함

```glsl
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in; // workgroup 하나에 invocation을 몇 개 넣을까
```

- x 방향으로 64개, y와 z 방향으로 1개씩. 즉, 2D(이미지)나 3D(볼륨 데이터)가 아닌 1D
- workgroup의 **크기**는 셰이더가 정함(`local_size_x`). 즉, 크기가 고정된 묶음 단위. 이 셰이더는 64개 단위로만 띄울 수 있음
- workgroup의 **개수**는 C++ 쪽에서 정함(10장, dispatch)
- 남는 invocation은 `if (index >= params.count) return;`에서 바로 끝남

| 배열 길이 | 필요한 워크그룹 수 | 실제로 실행되는 인보케이션 | 일하는 것 / 바로 return |
|---|---|---|---|
| 8 | 1 | 64 | 8 / 56 |
| 64 | 1 | 64 | 64 / 0 |
| 200 | 4 | 256 | 200 / 56 |

### CUDA와 compute shader

| Vulkan 컴퓨트 셰이더 | CUDA |
|---|---|
| 인보케이션 (Invocation) | 스레드 (Thread) |
| 워크그룹 (Workgroup) | 블록 (Block) |
| `local_size_x` | 블록 크기 (`blockDim.x`) |
| `gl_GlobalInvocationID.x` | `blockIdx.x * blockDim.x + threadIdx.x` |

## push constant 

- push : cpu가 값을 Gpu 쪽으로 밀어넣는다는 뜻.
- constant : 셰이더가 실행되는 동안 값이 바뀌지 않는다는 뜻. 셰이더는 읽기만 가능.

Vulkan에서 cpu는 gpu에게 바로 일을 시키지 않음. 할 일 목록인 **command buffer** 를 먼저 작성하고, 그 목록을 통째로 GPU에 제출함. `push constant`는 이 목록에 값 자체를 같이 적어 넣는 것. 

**커맨드 버퍼 (GPU에게 줄 작업 목록)**

| 순서 | 명령 | 비고 |
|---|---|---|
| 1 | 이 셰이더(`add.comp`)를 사용해라 | |
| 2 | 버퍼는 디스크립터 셋 0번을 써라 | 디스크립터가 버퍼가 "어디 있는지"만 알려 줌 |
| 3 | 푸시 상수: `count = 8` | 값 자체가 목록 안에 들어 있음 |
| 4 | 실행해라 (dispatch) | |

- 버퍼 방식은 '데이터가 저기 있으니 가서 읽어'라고 위치를 알려주는 방식
    - **버퍼(buffer)**: 값이 실제로 들어 있는 GPU 메모리 공간
    - **디스크립터(descriptor)**: 셰이더에게 그 버퍼가 어디 있는지 알려 주는 정보
- push constant는 '값은 이거야' 라고 내용을 직접 건내는 방식. 

## SPIR-V(Standard Portable Intermediate Representation)

- 셰이더를 GPU에 넘기기 위한 공통 바이너리 형식. GLSL 같은 사람이 읽는 셰이더 코드를 컴파일하면 이 형식이 나옴 
- GPU는 제조자마다 실제 기계어가 다름. 그래서 Vulkan은 셰이더를 두 단계로 나눠서 변환함

```text
add.comp (GLSL, 사람이 읽는 코드)
   │  ① glslc  ← 빌드할 때, 내 컴퓨터에서
   ▼
add.comp.spv (SPIR-V, 공통 형식)
   │  ② GPU 드라이버  ← 프로그램 실행 중에, vk::ShaderModule을 쓸 때
   ▼
실제 GPU 기계어 (NVIDIA용 / AMD용 / ...)
```


## 8장 요약

**목표**: 덧셈 셰이더(`add.comp`)를 빌드하고, C++에서 읽어서 셰이더 모듈(`vk::ShaderModule`)을 만드는 것. GPU 실행은 아직 안 함

### 1. 셰이더 코드 (`add.comp`, GLSL)

| 부분 | 역할 |
|---|---|
| `layout(local_size_x = 64) in;` | workgroup 하나에 invocation 64개 |
| `layout(set = 0, binding = 0/1/2, std430) buffer` | 7장에서 연결한 버퍼를 **번호로** 받음. 이름은 상관없음 |
| `readonly` / `writeonly` | 입력은 읽기만, 출력은 쓰기만 |
| `std430` | `float` 간격 4바이트. C++ 배열과 메모리 배치를 맞춤 |
| `float values[]` | 런타임 크기 배열. 길이는 연결된 버퍼가 정함 |
| `layout(push_constant) ... count` | push constant로 실제 원소 수를 받음 |
| `main()` | 반복문 없이 원소 하나만 계산. `gl_GlobalInvocationID.x`가 인덱스 역할 |
| `if (index >= params.count) return;` | 범위를 넘는 invocation은 바로 종료 |

### 2. 데이터 전달 방식

| | 버퍼 + 디스크립터 | push constant |
|---|---|---|
| 담기는 것 | 버퍼: 실제 값이 있는 GPU 메모리 / 디스크립터: 그 위치 정보 | 값 자체가 command buffer 안에 들어감 |
| 크기 | 거의 무제한 | 약 128바이트 |
| 셰이더가 쓰기 | 가능 | 불가능 |
| 용도 | 입력·출력 배열 | 원소 수 같은 작은 설정값 |

### 3. 빌드: GLSL → SPIR-V

```text
add.comp ──glslc(빌드 시)──▶ add.comp.spv ──드라이버(실행 시)──▶ GPU 기계어
```

- SPIR-V는 GPU 제조사와 상관없는 공통 바이너리(Python의 `.pyc`와 비슷). 32비트 워드 단위로 구성됨
- CMake가 빌드할 때 자동 변환하고, `.spv` 경로를 `ADD_SHADER_PATH` 매크로로 C++에 넘겨줌(`CMakeLists.txt`의 `target_compile_definitions`)

### 4. C++ 작업 (`main.cpp`)

1. `#include <fstream>` 추가
2. 멤버 변수 `vk::raii::ShaderModule shaderModule = nullptr;` 추가 (9장에서도 써야 해서 멤버로 둠)
3. `createShaderModule()` 작성

    ```text
    std::ifstream (ate | binary)로 열기 → tellg()로 크기 확인
    → 크기가 0보다 크고 4의 배수인지 검사
    → std::vector<std::uint32_t> code 준비 → seekg(0) → read()
    → ShaderModuleCreateInfo (codeSize는 바이트, pCode는 code.data())
    → vk::raii::ShaderModule 생성
    ```

    - `code`는 지역 변수. 모듈을 만들고 나면 필요 없어서 함수가 끝날 때 자동으로 사라짐
4. `initVulkan()` 끝에 `&& createShaderModule()` 연결

### 5. 결과

```text
Shader module created: 1720 bytes.
Resources destroyed.
```

- 모듈 생성 성공, 검증 레이어 오류 없음. 덧셈 결과는 10장에서 확인

### 다음 단계

| 장 | 내용 |
|---|---|
| 9 | 셰이더 모듈 + 디스크립터 셋 레이아웃 + push constant 크기를 묶어 **컴퓨트 파이프라인** 생성 |
| 10 | command buffer에 명령 기록, `count = 8` 전달, dispatch 실행, 결과 확인 |
