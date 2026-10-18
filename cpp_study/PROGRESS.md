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
- **원본(upstream = Pseudo-Lab)에는 push나 PR 금지.** 원본의 새 장을 받을 때만 `git fetch upstream` 후 merge (2026-10-14 기준 08~13장은 `main`이 아니라 `upstream/compute-pipeline-tutorials` 브랜치에 있음)
- upstream `.gitignore`가 허용 목록(allowlist) 방식이라, `cpp_study/`·`question/`을 허용하는 줄을 `.gitignore` 맨 끝에 추가해 둠 → upstream merge 때 이 줄이 지워지지 않았는지 확인
- 회사 컴퓨터에 upstream 등록: `git remote add upstream https://github.com/Pseudo-Lab/Cross-Platform-AI.git`
- 환경(Vulkan SDK, CMake, 컴파일러)은 컴퓨터마다 [00. 환경 설정](../examples/00-env-settings/README.md)을 먼저 확인

## GCP 인스턴스 `vulkan-tutorial` (b·c 존에 하나씩, 2026-10-18 기준 c 사용 중)

같은 이름의 인스턴스가 b 존(`asia-northeast3-b`)과 c 존(`asia-northeast3-c`)에 하나씩 있습니다. 둘 다 n1-standard-4 + T4이고, T4 재고가 없으면(STOCKOUT) 켜지지 않아서 켜지는 쪽을 씁니다.

- 2026-10-13: b 존 재고 없음 → 스냅샷 `vulkan-tutorial-snap-20261013`으로 c 존에 새로 만듦
- 2026-10-17: c 존 재고 없음 → b 존 옛 인스턴스를 켜서 사용 (코드는 `git pull`로 최신화)
- 2026-10-18: c 존 인스턴스에서 09장 작업 (IP·`~/.ssh/config` 상태는 접속할 때 다시 확인)
- **두 디스크는 따로입니다.** push 안 한 파일은 다른 존 인스턴스에 없음 → 작업 끝나면 꼭 push

**존을 바꿀 때 할 일** (켜는 쪽 존 이름으로)

1. 켜기: `gcloud compute instances start vulkan-tutorial --zone=asia-northeast3-<존>`
2. 외부 고정 IP `34.47.80.216`을 켜는 쪽으로 옮기기. **IP가 없으면 인터넷이 안 됨** → `git pull`이 멈추고 VS Code가 "Copying VS Code Server to host with scp"에서 멈춤

   ```sh
   gcloud compute instances delete-access-config vulkan-tutorial --zone=asia-northeast3-<안 쓰는 존> --access-config-name=external-nat
   gcloud compute instances add-access-config vulkan-tutorial --zone=asia-northeast3-<켜는 존> --access-config-name=external-nat --address=34.47.80.216
   ```

3. `~/.ssh/config`의 `ProxyCommand` 줄 `--zone=` 바꾸기 (Mac·회사 컴퓨터 둘 다)
4. 호스트 키가 존마다 달라서 경고가 뜨면 `ssh-keygen -R vulkan-tutorial` 후 다시 접속
5. VM에서 `git pull`

현재(2026-10-18): b 존 재고 없어 c 존 사용. IP는 c 존에 붙어 있음, 집 Mac `~/.ssh/config`도 c 존. 회사 컴퓨터는 마지막 설정 존 확인 후 다르면 3·4번 하기

## 현재 위치 (2026-10-18 기준)

**09. Compute pipeline 완료 → 다음: [10. Command buffers](../examples/10-command-buffers/README.md)**

- 09장: `examples/09-compute-pipeline/main.cpp` 빌드·실행 확인 (`Shader module created: 1720 bytes.` / `Compute pipeline created.` / `Resources destroyed.`, 검증 레이어 경고 없음, Debug 빌드)
- 09장: `createPipelineLayout()`의 푸시 상수 범위·`layoutInfo`까지는 직접 채움 (오타 `pPushConstatnRanges` → `pPushConstantRanges` 수정). 레이아웃 생성·`return true`, 멤버 2개, `createComputePipeline()` 전체, `initVulkan()` 연결은 Claude가 채움 → 코드 읽으며 복습 필요
- 09장 실행 결과 터미널 캡처 제출하기 (README 요구사항)
- `cpp_study/09-compute-pipeline.md`는 아직 빈 파일 (아래 "09장 대화" 항목을 정리할 곳)
- VS Code에서 `.comp` 색이 안 입혀짐: SSH 호스트에 GLSL 확장이 없음 → `slevesque.shader`를 "Install in SSH"로 설치 예정

- 08장: `examples/08-compute-shader/main.cpp` 빌드·실행 확인 (`Shader module created: 1720 bytes.`, 검증 레이어 경고 없음, Debug 빌드)
- 08장: `<fstream>`, `shaderModule` 멤버, `createShaderModule()`의 `seekg(0)`까지는 직접 채움. `read()`부터 모듈 생성·`return true`와 `initVulkan()` 연결은 Claude가 채움 → 코드 읽으며 복습 필요 (`reinterpret_cast<char*>`, `codeSize`는 바이트·`pCode`는 `code.data()`)
- 08장 실행 결과 터미널 캡처 제출하기 (README 요구사항)
- VM에 `glslang-tools` 설치함 (VS Code GLSL Lint 확장용 `glslangValidator`). c 존에도 10-18 설치

- 07장: `examples/07-descriptor-sets/main.cpp` 빌드·실행 확인 (풀 max sets 1 / StorageBuffer 3개, 바인딩 0·1·2가 각각 다른 버퍼 핸들, range 32 bytes, 검증 레이어 경고 없음)
- 07장: `<vector>`, 멤버, `createDescriptorPool()`은 직접 채움. `createDescriptorSets()` 전체와 `initVulkan()` 연결은 Claude가 채움 → 코드 읽으며 복습 필요 (특히 `pSetLayouts = &layout`, `for (auto& ...)`의 `&`, `pBufferInfo = &bufferInfos[i]`)
- 06·07장 실행 결과 터미널 캡처 제출하기 (README 요구사항)
- 06장: `examples/06-descriptor-layout/main.cpp` 빌드·실행 확인 (`Descriptor set layout created: 3 bindings.`, 검증 레이어 경고 없음)
- 06장은 README 보며 직접 채움 (바인딩 3개 반복문, `DescriptorSetLayoutCreateInfo`, `initVulkan()` 연결)
- 05장 후반(`createBuffer()`의 메모리 할당·연결부터 `writeAndVerifyBuffer()`, `createStorageBuffers()`, `initVulkan()` 연결까지)은 Claude가 채움 → 코드 읽으며 복습 필요
- 02장은 콜백 작성 단계에서 멈춘 채 넘어옴 (나중에 돌아가서 마무리)
- 다음: 08 Compute shader → 09 Compute pipeline → 10 Command buffers → 11 Dispatch와 동기화 → 12 Staging buffers → 13 Multiple dispatches (11장에서 첫 GPU 벡터 덧셈 완성)

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
| 08. Compute shader | ✅ | 빌드·실행 확인 (`Shader module created: 1720 bytes.`). `read()` 이후는 Claude가 채움 |
| 09. Compute pipeline | ✅ | 빌드·실행 확인 (`Compute pipeline created.`). 레이아웃 생성 이후는 Claude가 채움 |

## 빌드 명령 (Linux)

```sh
cd examples/<장 폴더>
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target <타깃 이름>   # 01: instance_example, 03: physical_devices, 04: logical_device_example, 05: buffers_memory_example, 06: descriptor_layout_example, 07: descriptor_sets_example, 08: compute_shader_example, 09: compute_pipeline_example
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
- [08-compute-shader.md](08-compute-shader.md): `add.comp`(GLSL compute shader), 바깥 반복문 대신 병렬 실행, Invocation·Workgroup(`local_size_x`), 남는 invocation과 범위 검사, CUDA 대응표, push constant, SPIR-V, 8장 요약
- 그림 정리 (브라우저로 열기): [vulkan-overview.html](vulkan-overview.html) 00~07장 총정리(전체 지도·검증 레이어·큐 패밀리·버퍼/메모리·RAII 정리 순서·C++ 문법 모음), [descriptor-visual.html](descriptor-visual.html) 06·07장 디스크립터 단계별 그림

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
- (2026-10-16 디스크립터·메모리 복습 대화) binding = descriptor set 안의 번호 붙은 자리 vs descriptorType(storage 등) 구분, descriptor = buffer 위치 정보(offset/range, 데이터 복사 아님) vs push constant(값 자체, 08장 `params.count`), descriptor set = binding 묶음(set·binding 2단계 번호, bindDescriptorSets로 통째로 연결), pool의 maxSets·type별 descriptorCount ≥ 필요량(layout↔set 칸 수는 자동, layout↔셰이더는 개발자가 맞춤), `eFreeDescriptorSet`가 필요한 이유(raii 개별 반납), binding 수는 계산이 정하고 GPU 한계는 천장(T4 storage 1,048,576 / 최소 보장 4, `maxBoundDescriptorSets` 32는 set 수), buffer(정의)·memory(공간)·data 구분과 CPU(map) vs GPU(descriptor) 접근 경로, HostVisible(map 가능)·HostCoherent(flush/invalidate 불필요, 캐시 불일치)·HostCached, T4 메모리 타입 3번(캐시 없음)·4번(cached), 버퍼 값은 언제든 수정·크기는 고정(재생성+복사, 2배 성장, 쪼개기, OOM 시 CPU 메모리), 큰 할당 하나를 sub-allocation(VMA)·재사용, Vulkan이 객체를 분리한 이유(explicit API), buffer와 queue의 관계(command buffer가 descriptor set으로 간접 참조, `sharingMode`), queue는 GPU 쪽(CPU는 handle만), submit=시작·dispatch=기록
- (2026-10-17 08장 대화) `add.comp`는 README 설명용으로 이미 완성된 파일(`main.cpp`에 넣지 않음, CMake가 `glslc`로 `.spv` 빌드), GLSL에도 for/while 있음(바깥 반복문만 병렬 실행이 대신), workgroup 크기는 셰이더·개수는 C++(10장 dispatch), 셰이더 종류(vertex·fragment·compute, workgroup은 compute 전용), AI 연산은 언어·비전 모두 compute(fragment는 화면 그리기·WebGL 우회·CUDA 이전), `add.comp` 수정 가능하지만 binding·push constant·`local_size_x`·타입은 C++과 맞춰야 함, 08장에선 셰이더를 아직 실행하지 않음
- (2026-10-17 08장 후반 대화) 디스크립터 바인딩에 맞춘 버퍼 선언(`set`/`binding` 번호로 연결, 이름 무관, `readonly`/`writeonly`, `std430` stride, `values[]` 런타임 크기 배열), push constant(command buffer에 값 자체를 적음, 약 128바이트, 읽기 전용) vs 버퍼(값이 있는 GPU 메모리)·디스크립터(위치 정보), 버퍼가 필요한 이유(크기·결과 쓰기·GPU에 남음), `count`를 push constant로 넘기는 이유(`.length()` 대안), SPIR-V(공통 바이너리, `.pyc` 비유, 32비트 워드), `ADD_SHADER_PATH`는 CMake `target_compile_definitions`(`-D`)가 정의, 1바이트 = 8비트·`sizeof`는 바이트, `uint32_t`(u=unsigned, _t=type), `code(n)`은 함수가 아니라 변수 정의 + 생성자 인자, 지역 변수 vs 멤버 변수(`code` vs `shaderModule`), `std::vector`(실행 중 크기) vs `std::array`(컴파일 시 크기), `static_cast`는 키워드, `minus.comp`처럼 연산만 바꾼 셰이더 가능(CMake 경로 수정 필요, 10장 이후 추천)
- (2026-10-18 09장 대화) 파이프라인 레이아웃(리소스 구조 = 함수 시그니처) vs 컴퓨트 파이프라인(셰이더 + 레이아웃, 생성 시 고정·셰이더마다 하나), `DescriptorSets` 복수형(할당 API가 배열, 프레임별·셋 번호별), `{}` 값 초기화(선언이 아니라 초기화, `updateDescriptorSets(writes, {})`의 `{}`는 빈 배열), 푸시 상수 `count`가 필요한 이유(인보케이션 64개 vs 원소 8개, 범위 검사), 원소 수(개수) vs 바이트 크기, `local_size_x = 64`는 기본값이 아니라 셰이더에서 정한 값, 인보케이션 하나 = 원소 하나, `groupCount = (count + 63) / 64`를 실행 중에 계산(자동 아님, 개발자 코드가 구함), `maxComputeWorkGroupCount` X 최소 65,535, 디스패치(워크그룹 수를 받는 실행 명령, 커맨드 버퍼에 기록), 원소 = 가중치 값 하나(fp16/int8), 원소별 덧셈은 잔차 연결·bias, matmul은 출력 원소당 인보케이션, `*raii` vs 포인터 역참조(`operator*` 오버로딩, `&layout`이 진짜 포인터), 셋 0 = `pSetLayouts[0]`(배열 인덱스 = 셋 번호, 0인 이유), `setLayout` = descriptor set layout 줄임, 셰이더 단계(컴퓨트는 단계 하나, 구조체를 그래픽스와 공유), 1720바이트 = SPIR-V 파일 크기(`OpDecorate`·타입·이름이 대부분, `spirv-dis`로 확인)

## Claude에게 이어서 부탁할 때

> `cpp_study/PROGRESS.md` 읽고 10장 `examples/10-command-buffers` README부터 이어서 도와줘.

- 파이썬은 익숙하고 C++은 처음 배우는 중입니다. 파이썬과 비교하는 설명을 선호합니다.
- 코드는 README를 보며 직접 채웁니다. 정답 코드를 대신 써 주기보다 문법 설명과 실수 지적을 원합니다.
