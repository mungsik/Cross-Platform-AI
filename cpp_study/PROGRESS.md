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

## 현재 위치 (2026-10-11 기준)

**02. Validation layers → "메시지를 받을 콜백 작성하기" 섹션**

- 파일: `examples/02-validation-layers/main.cpp`
- 가이드: `examples/02-validation-layers/README.md` 64줄부터

### 02장 진행 상황

- [x] 사용할 레이어 이름 정하기: `static constexpr const char* validationLayer`, `#include <cstring>`
- [x] 레이어가 설치되어 있는지 확인하기: `checkValidationLayerSupport()` 완성
- [ ] **메시지를 받을 콜백 작성하기** ← 여기부터. `debugCallback` 틀만 넣었고 본문은 비어 있음
  - 다음 할 일: 본문에 심각도/종류 출력 → 식별자/메시지 출력 → `return VK_FALSE;`
- [ ] 어떤 메시지를 받을지 설정하기
- [ ] 인스턴스 생성 전에 지원 여부 검사하기
- [ ] 레이어와 확장을 인스턴스에 연결하기
- [ ] 인스턴스 생성 후에도 메시지 받기
- [ ] 콜백으로 확인용 메시지 보내기
- [ ] 초기화와 실행 순서 연결하기
- [ ] 빌드와 실행

> 지금 상태로 빌드하면 `debugCallback`에 `return`이 없어서 경고가 납니다. 위 단계를 채우면 해결됩니다.

## 완료한 장

| 장 | 상태 | 비고 |
|---|---|---|
| 00. 환경 설정 | ✅ | `env_check` 빌드·실행 확인 |
| 01. Instance | ✅ | 빌드·실행 확인 (`Instance created.` / `Instance destroyed.`) |
| 02. Validation layers | 🔄 진행 중 | 위 체크리스트 참고 |

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

노트에 아직 정리하지 않은, 대화에서 다룬 주제:

- `#include`, 전처리기, `.cpp`/`.hpp`, `-dev` 패키지(헤더 vs 라이브러리)
- `public`/`private`(접근 지정자, 캡슐화), `void`, `int main()`과 종료 코드
- 네임스페이스와 `::`, `vk`/`vk::raii`/RAII
- 중괄호 초기화 `{}`, `auto`, 범위 기반 for, 참조(`&`)와 포인터(`*`)의 차이
- `std::cout`/`std::cerr`, `<<` 연결
- `static` 멤버(클래스에 하나), `constexpr` + `const` 위치
- CMake: `-S`/`-B`, configure와 build, 타깃(`add_executable`), `CMakeLists.txt` 자동 탐색

## Claude에게 이어서 부탁할 때

> `cpp_study/PROGRESS.md` 읽고 02장 "메시지를 받을 콜백 작성하기"부터 이어서 도와줘.

- 파이썬은 익숙하고 C++은 처음 배우는 중입니다. 파이썬과 비교하는 설명을 선호합니다.
- 코드는 README를 보며 직접 채웁니다. 정답 코드를 대신 써 주기보다 문법 설명과 실수 지적을 원합니다.
