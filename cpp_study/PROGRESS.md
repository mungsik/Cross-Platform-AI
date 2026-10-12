# 진행 기록

다른 컴퓨터에서 이어서 공부할 때 이 파일부터 확인합니다.

## 다른 컴퓨터에서 이어 하기

처음 한 번:

```sh
git clone git@github.com:mungsik/Cross-Platform-AI.git
cd Cross-Platform-AI
git switch seongsik
```

그 다음부터:

```sh
git pull     # 작업 시작 전
git push     # 작업 끝난 후
```

- 작업 브랜치: `seongsik` (내 포크 `origin` = mungsik/Cross-Platform-AI)
- **원본(upstream = Pseudo-Lab)에는 push나 PR 금지.** 원본의 새 장을 받을 때만 `git pull upstream main`
- 회사 컴퓨터에 upstream 등록: `git remote add upstream https://github.com/Pseudo-Lab/Cross-Platform-AI.git`
- 환경(Vulkan SDK, CMake, 컴파일러)은 컴퓨터마다 [00. 환경 설정](../examples/00-env-settings/README.md)을 먼저 확인

## 현재 위치 (2026-10-12 기준)

**05. Buffer와 memory 완료 → 다음: 06. Descriptor set layout**

- 05장: `examples/05-buffers-memory/main.cpp` 빌드·실행 확인 (버퍼 3개 × 32바이트, 메모리 type 3, CPU 기록·확인 통과)
- 05장 후반(`createBuffer()`의 메모리 할당·연결부터 `writeAndVerifyBuffer()`, `createStorageBuffers()`, `initVulkan()` 연결까지)은 Claude가 채움 → 코드 읽으며 복습 필요
- 02장은 콜백 작성 단계에서 멈춘 채 넘어옴 (나중에 돌아가서 마무리)
- 다음: [06. Descriptor set layout](../examples/06-descriptor-layout/README.md)

### 05장 진행 상황

- [x] 버퍼 크기와 보관할 자리 만들기 (`static_assert`, `elementCount`, `bufferSize`, `BufferResource`, `inputA`/`inputB`/`output`)
- [x] 버퍼 객체 만들기 (`vk::BufferCreateInfo`, `eStorageBuffer`, `eExclusive`)
- [x] 버퍼가 요구하는 메모리 확인하기 (`getMemoryRequirements()`, `eHostVisible | eHostCoherent`)
- [x] 사용할 메모리 유형 찾기 (`findMemoryType()`, `1u << i` 비트 검사)
- [x] 메모리 할당하고 버퍼에 연결하기 (`vk::raii::DeviceMemory`, `bindMemory`)
- [x] CPU에서 값 기록하기 / 다시 읽기 (`mapMemory`, `memcpy`, `unmapMemory`)
- [x] 입력과 출력 버퍼 준비하기 (`createStorageBuffers()`)
- [x] 초기화 순서에 연결하기 / 빌드와 실행 (타깃 `buffers_memory_example`)

## 완료한 장

| 장 | 상태 | 비고 |
|---|---|---|
| 00. 환경 설정 | ✅ | `env_check` 빌드·실행 확인 |
| 01. Instance | ✅ | 빌드·실행 확인 (`Instance created.` / `Instance destroyed.`) |
| 02. Validation layers | ⏸ 미완 | 콜백 작성 단계에서 멈춤. 03장으로 먼저 넘어감 |
| 03. Physical devices | ✅ | 빌드·실행 확인 (`Selected physical device: Tesla T4`) |
| 04. Logical device | ✅ | 빌드·실행 확인 (`Logical device created.`) |
| 05. Buffer와 memory | ✅ | 빌드·실행 확인 (`CPU write/readback verified.`) |

## 빌드 명령 (Linux)

```sh
cd examples/<장 폴더>
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target <타깃 이름>   # 01: instance_example, 03: physical_devices, 04: logical_device_example, 05: buffers_memory_example
./build/main
```

타깃 이름은 각 장의 `CMakeLists.txt`에 있는 `add_executable(이름 ...)`에서 확인합니다. **반드시 그 장 폴더에서** 실행합니다 (다른 장 폴더의 `build/`는 그 장 타깃만 앎 → `No rule to make target`).

## 공부 노트

- [01-instance.md](01-instance.md): 포인터, 구조체와 변수, struct와 class, 필드, 파이썬과 C++ 변수의 차이, 멤버 변수와 지역 변수, 타입
- [02-validation-layers.md](02-validation-layers.md): constexpr, `const char*`
- [03-physical-devices.md](03-physical-devices.md): `!`로 실패 먼저 거르기(early return), Context, `vk::raii::Context context;` 읽는 법, `auto`, `std::optional`과 `const ... &`, `std::move()`, 빌드 결과 해석
- [04-logical-device.md](04-logical-device.md): `{}` vs `()`(most vexing parse), 괄호 세 종류, `&`로 주소 넘기기, 큐 우선순위, `createLogicalDevice` 흐름, 큐 패밀리와 `vulkaninfo`로 확인하기, 빌드 결과, **스터디 질문 준비(Queue vs QueueFamily)**
- [05-buffers-memory.md](05-buffers-memory.md): `static_assert`·`constexpr`(constant expression), 비트와 바이트, 멤버와 필드, 버퍼와 메모리 구분, 메모리 요구사항·`eHostVisible`/`eHostCoherent`, enum 값 앞의 `e`, `mapMemory`와 `memcpy`, 빌드 결과

노트에 아직 정리하지 않은, 대화에서 다룬 주제:

- `#include`, 전처리기, `.cpp`/`.hpp`, `-dev` 패키지(헤더 vs 라이브러리)
- `public`/`private`(접근 지정자, 캡슐화), `void`, `int main()`과 종료 코드
- 네임스페이스와 `::`, `vk`/`vk::raii`/RAII
- 중괄호 초기화 `{}`, `auto`, 범위 기반 for, 참조(`&`)와 포인터(`*`)의 차이
- `std::cout`/`std::cerr`, `<<` 연결
- `static` 멤버(클래스에 하나), `constexpr` + `const` 위치
- (03장 대화) `if (조건)` 괄호, `.empty()`, `nullptr`(빈자리), 멤버 변수 생성/정리 순서, `std` 네임스페이스, `uint32_t`(오타 `unit32_t` 주의)
- (03장 대화) `::`는 항상 "~ 안의", 변수 이름은 띄어쓰기 뒤 / 클래스 이름(대문자)과 변수 이름(소문자)
- (03장 대화) auto 쓸 때와 안 쓸 때(멤버 변수엔 auto 불가), `auto&` vs `const auto&`
- CMake: `-S`/`-B`, configure와 build, 타깃(`add_executable`), `CMakeLists.txt` 자동 탐색
- (03장 후반 대화) 함수 매개변수·반환 타입엔 auto 대신 타입 명시, auto 타입은 컴파일러가 헤더 선언으로 앎(VS Code 마우스 오버로 확인)
- (03장 후반 대화) C 스타일 for문 `for (초기; 조건; ++i)`, `&&`(논리) vs `&`(비트) 플래그 검사, `std::nullopt`, `*optional`
- (03장 후반 대화) 지역 변수 수명(블록 `}`에서 정리) vs 멤버 변수, 디버깅용 `std::cout` 찍기
- (04~05장 대화) CPU가 GPU에 일을 시키는 구조와 Queue 활용 흐름(커맨드 버퍼 → submit → fence), 논리 장치, Vulkan이란(Khronos 표준, CUDA·Metal 비교), 크로스 플랫폼 조건과 기기별 최적화, Vulkan으로 AI 모델 실행(llama.cpp, ncnn 등)

## Claude에게 이어서 부탁할 때

> `cpp_study/PROGRESS.md` 읽고 다음 장(06)부터 이어서 도와줘.

- 파이썬은 익숙하고 C++은 처음 배우는 중입니다. 파이썬과 비교하는 설명을 선호합니다.
- 코드는 README를 보며 직접 채웁니다. 정답 코드를 대신 써 주기보다 문법 설명과 실수 지적을 원합니다.
