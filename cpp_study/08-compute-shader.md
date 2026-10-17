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
