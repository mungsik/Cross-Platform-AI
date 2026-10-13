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

## GCP 인스턴스 `vulkan-tutorial` (2026-10-13 존 이전)

b 존(`asia-northeast3-b`)에 T4 재고가 없어(STOCKOUT) 켜지지 않아서, 스냅샷으로 **c 존(`asia-northeast3-c`)**에 같은 이름으로 다시 만들었습니다.

- 그대로: 머신(n1-standard-4 + T4), 디스크 내용(홈, SDK, 저장소), 외부 고정 IP `34.47.80.216`, 서비스 계정
- 바뀜: 존 b → c, 내부 IP `10.178.0.12` → `10.178.0.13`, SSH 호스트 키
- 켜기: `gcloud compute instances start vulkan-tutorial --zone=asia-northeast3-c`
- 남은 리소스: b 존 옛 인스턴스·디스크(꺼짐), 스냅샷 `vulkan-tutorial-snap-20261013` → 정리 여부 미정

**회사 컴퓨터에서 처음 접속할 때 할 일** (집 Mac은 완료)

- [ ] `~/.ssh/config`의 `ProxyCommand` 줄 `--zone=asia-northeast3-b` → `--zone=asia-northeast3-c`
- [ ] 호스트 키 경고가 뜨면 `ssh-keygen -R vulkan-tutorial` 후 다시 접속
- [ ] `gcloud compute ssh`를 쓰면 `--zone=asia-northeast3-c`로 지정

## 현재 위치 (2026-10-13 기준)

**07. Descriptor pool과 sets 완료 → 다음: 08장 (저장소에 아직 없음, `git pull upstream main`으로 확인)**

- 07장: `examples/07-descriptor-sets/main.cpp` 빌드·실행 확인 (풀 max sets 1 / StorageBuffer 3개, 바인딩 0·1·2가 각각 다른 버퍼 핸들, range 32 bytes, 검증 레이어 경고 없음)
- 07장: `<vector>`, 멤버, `createDescriptorPool()`은 직접 채움. `createDescriptorSets()` 전체와 `initVulkan()` 연결은 Claude가 채움 → 코드 읽으며 복습 필요 (특히 `pSetLayouts = &layout`, `for (auto& ...)`의 `&`, `pBufferInfo = &bufferInfos[i]`)
- 06·07장 실행 결과 터미널 캡처 제출하기 (README 요구사항)
- 06장: `examples/06-descriptor-layout/main.cpp` 빌드·실행 확인 (`Descriptor set layout created: 3 bindings.`, 검증 레이어 경고 없음)
- 06장은 README 보며 직접 채움 (바인딩 3개 반복문, `DescriptorSetLayoutCreateInfo`, `initVulkan()` 연결)
- 05장 후반(`createBuffer()`의 메모리 할당·연결부터 `writeAndVerifyBuffer()`, `createStorageBuffers()`, `initVulkan()` 연결까지)은 Claude가 채움 → 코드 읽으며 복습 필요
- 02장은 콜백 작성 단계에서 멈춘 채 넘어옴 (나중에 돌아가서 마무리)
- 다음: 08장이 올라오면 이어서 (셰이더 모듈 → 파이프라인 → 커맨드 버퍼·dispatch → 제출·대기 → `output` 읽기 순서로 예상)

## 완료한 장

| 장 | 상태 | 비고 |
|---|---|---|
| 00. 환경 설정 | ✅ | `env_check` 빌드·실행 확인 |
| 01. Instance | ✅ | 빌드·실행 확인 (`Instance created.` / `Instance destroyed.`) |
| 02. Validation layers | ⏸ 미완 | 콜백 작성 단계에서 멈춤. 03장으로 먼저 넘어감 |
| 03. Physical devices | ✅ | 빌드·실행 확인 (`Selected physical device: Tesla T4`) |
| 04. Logical device | ✅ | 빌드·실행 확인 (`Logical device created.`) |
| 05. Buffer와 memory | ✅ | 빌드·실행 확인 (`CPU write/readback verified.`) |
| 06. Descriptor set layout | ✅ | 빌드·실행 확인 (`Descriptor set layout created: 3 bindings.`) |
| 07. Descriptor pool과 sets | ✅ | 빌드·실행 확인 (`Descriptor sets updated: 1`). 후반은 Claude가 채움 |

## 빌드 명령 (Linux)

```sh
cd examples/<장 폴더>
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target <타깃 이름>   # 01: instance_example, 03: physical_devices, 04: logical_device_example, 05: buffers_memory_example, 06: descriptor_layout_example, 07: descriptor_sets_example
./build/main
```

타깃 이름은 각 장의 `CMakeLists.txt`에 있는 `add_executable(이름 ...)`에서 확인합니다. **반드시 그 장 폴더에서** 실행합니다 (다른 장 폴더의 `build/`는 그 장 타깃만 앎 → `No rule to make target`).

## 스터디 질문

- [1주차 질문](../question/week1-assignments-Q.md) / [질문+답변 한 파일](../question/week1-assignments-A.md): Queue·QueueFamily, 버퍼·메모리·`bindMemory`, 메모리 속성(T4 `vulkaninfo` 근거), map/memcpy/unmap, 네 가지 타입 설정(예시 5개), descriptor 상황 1~5
- 답변 중 메모리 장벽, 커맨드 버퍼 재녹화, `CopyDescriptorSet`, 질문 5 예시 3(틀렸을 때 결과)은 아직 튜토리얼에서 안 배운 내용 → Vulkan 사양 기준으로 Claude가 정리, 직접 실행 확인은 안 함

## 공부 노트

- [01-instance.md](01-instance.md): 포인터, 구조체와 변수, struct와 class, 필드, 파이썬과 C++ 변수의 차이, 멤버 변수와 지역 변수, 타입
- [02-validation-layers.md](02-validation-layers.md): constexpr, `const char*`
- [03-physical-devices.md](03-physical-devices.md): `!`로 실패 먼저 거르기(early return), Context, `vk::raii::Context context;` 읽는 법, `auto`, `std::optional`과 `const ... &`, `std::move()`, 빌드 결과 해석
- [04-logical-device.md](04-logical-device.md): `{}` vs `()`(most vexing parse), 괄호 세 종류, `&`로 주소 넘기기, 큐 우선순위, `createLogicalDevice` 흐름, 큐 패밀리와 `vulkaninfo`로 확인하기, 빌드 결과, **스터디 질문 준비(Queue vs QueueFamily)**
- [05-buffers-memory.md](05-buffers-memory.md): `static_assert`·`constexpr`(constant expression), 비트와 바이트, 멤버와 필드, 버퍼와 메모리 구분, 메모리 요구사항·`eHostVisible`/`eHostCoherent`, enum 값 앞의 `e`, `mapMemory`와 `memcpy`, 빌드 결과
- [06-descriptor-layout.md](06-descriptor-layout.md): descriptor·descriptor set·layout·binding 용어, 누가 무엇을 가지나(버퍼/layout/set/셰이더), layout에 없는 것(버퍼 이름·크기·입출력), 빵 공장·AI 모델 비유, `descriptorCount`, `pXxx`+`xxxCount` 짝 패턴, `static_cast`, 빌드 결과
- [07-descriptor-sets.md](07-descriptor-sets.md): `*` 구분 팁 (작성 중)

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
- (07장 대화) `#include <vector>`와 `std::vector`(파이썬 list, 길이 가변) vs `std::array`(고정), descriptor pool(셋을 떼어 주는 창고, 칸당 count vs 풀 전체 count), Vulkan 핸들(번호표, opaque, `vk::Buffer` vs `vk::raii::Buffer`), `*`의 세 가지(포인터 역참조·`*optional`·`*raii`, 연산자 오버로딩), descriptor는 "설명서"가 아니라 자원 위치 쪽지(LayoutBinding = 칸 규칙과 구분), 만들기 → 연결하기 → 계산기 조립 → 실행 큰 흐름
- (스터디 질문 대화) 큐 개수는 하드웨어가 정하고 우리는 그 안에서 패밀리·개수를 고름, 버퍼/메모리 분리는 명시적 API 공통 특징(VMA), 모델 버퍼 크기는 "필요한 최대치로 미리 잡고 일부만 쓰기"(가중치·KV 캐시·중간 결과, offset/range, 넘치면 새로 만들어 옮김), Vulkan은 주로 추론용(학습은 CUDA 생태계), T4 한계값(`maxStorageBufferRange` 약 4GiB, `minStorageBufferOffsetAlignment` 16), `getMemoryRequirements()`의 size/alignment/memoryTypeBits, offset(시작 위치), device memory와 DeviceLocal(외장 GPU=VRAM, 내장 GPU=공유 RAM), `mapMemory()` 장점·단점(스테이징 버퍼와 비교), `void*`(타입 없는 포인터, 타입은 우리 코드와 셰이더가 약속), 메모리 매핑(주소 범위 연결), `memcpy`(memory copy, 받는 쪽 먼저, 주소+크기), `.data()`(첫 칸 주소), `unmapMemory()`(값 전송이 아니라 연결 해제, 다시 map하려면 필수)

## Claude에게 이어서 부탁할 때

> `cpp_study/PROGRESS.md` 읽고 다음 장(08)부터 이어서 도와줘.

- 파이썬은 익숙하고 C++은 처음 배우는 중입니다. 파이썬과 비교하는 설명을 선호합니다.
- 코드는 README를 보며 직접 채웁니다. 정답 코드를 대신 써 주기보다 문법 설명과 실수 지적을 원합니다.
