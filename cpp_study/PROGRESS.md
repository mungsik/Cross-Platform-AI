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

**03. Physical devices → "물리적 디바이스 목록 가져오기" 섹션 마무리 → "API 버전 확인하기"**

- 파일: `examples/03-physical-devices/main.cpp`
- 가이드: `examples/03-physical-devices/README.md` 30줄부터 (다음 섹션은 73줄)
- 02장은 콜백 작성 단계에서 멈춘 채 03장으로 넘어옴 (03장 `main.cpp`에는 02장 완성 코드가 이미 들어 있음)

### 03장 진행 상황

- [x] 장치 선택 준비하기: `physicalDevice`, `computeQueueFamilyIndex` 멤버, `<cstdint>`/`<optional>`/`<utility>` 추가
- [ ] **물리적 디바이스 목록 가져오기** ← 거의 끝. 아래 "고칠 것" 먼저
  - [x] `pickPhysicalDevice()` 틀, `enumeratePhysicalDevices()`, `empty()` 검사
  - [x] `for (auto& candidate : physicalDevices)` 반복문
- [ ] API 버전 확인하기 (`getProperties()`, `continue`)
- [ ] 컴퓨트 큐 패밀리 찾기 (`findComputeQueueFamily`, `std::optional`)
- [ ] 조건을 만족하는 장치 저장하기 (`std::move(candidate)`)
- [ ] 선택한 장치 정보 확인하기
- [ ] 초기화 순서에 연결하기 (`initVulkan()`에 `pickPhysicalDevice()` 추가)
- [ ] 빌드와 실행

### 다음에 시작할 때 먼저 고칠 것

1. **에러 출력과 `return false;`가 `for` 반복문 안에 들어가 있음.** README 66줄: 반복문의 닫는 중괄호 `}` **아래**(반복문 밖)로 옮겨야 함. 지금은 첫 GPU를 보자마자 실패로 끝남.
2. 에러 메시지 오타: `pyhsical` → `physical`

## 완료한 장

| 장 | 상태 | 비고 |
|---|---|---|
| 00. 환경 설정 | ✅ | `env_check` 빌드·실행 확인 |
| 01. Instance | ✅ | 빌드·실행 확인 (`Instance created.` / `Instance destroyed.`) |
| 02. Validation layers | ⏸ 미완 | 콜백 작성 단계에서 멈춤. 03장으로 먼저 넘어감 |
| 03. Physical devices | 🔄 진행 중 | 위 체크리스트 참고 |

## 빌드 명령 (Linux)

```sh
cd examples/<장 폴더>
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target <타깃 이름>   # 01: instance_example
./build/main
```

타깃 이름은 각 장의 `CMakeLists.txt`에 있는 `add_executable(이름 ...)`에서 확인합니다.

## 공부 노트

- [01-instance.md](01-instance.md): 포인터, 구조체와 변수, struct와 class, 필드, 파이썬과 C++ 변수의 차이, 멤버 변수와 지역 변수, 타입
- [02-validation-layers.md](02-validation-layers.md): constexpr, `const char*`
- [03-physical-devices.md](03-physical-devices.md): `!`로 실패 먼저 거르기(early return), Context, `vk::raii::Context context;` 읽는 법, `auto`

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

## Claude에게 이어서 부탁할 때

> `cpp_study/PROGRESS.md` 읽고 03장 "다음에 시작할 때 먼저 고칠 것"부터 이어서 도와줘.

- 파이썬은 익숙하고 C++은 처음 배우는 중입니다. 파이썬과 비교하는 설명을 선호합니다.
- 코드는 README를 보며 직접 채웁니다. 정답 코드를 대신 써 주기보다 문법 설명과 실수 지적을 원합니다.
