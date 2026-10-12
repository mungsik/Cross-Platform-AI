## 1. 포인터 쓰는 이유

Q. 아 그럼 문자라 직접 전달하지 못하니까 포인터로 사용한거고, applicationVersion 같은건 숫자니까 바로 전달한거야?

A. 네, 정확히 이해하셨습니다. 한 가지만 보충하면, 핵심은 "문자냐 숫자냐"보다 "크기가 정해져 있느냐" 입니다.

### 구조체의 칸은 크기가 고정되어 있어야 함

| 필드 | 데이터 | 크기 | 저장 방식 |
|---|---|---|---|
| `applicationVersion` | 숫자 | 항상 4바이트 (`uint32_t`) | 칸에 값을 직접 넣음 |
| `pApplicationName` | 문자열 | `"01-instance"`는 11글자, `"My Super App"`은 12글자… 제각각 | 칸에는 주소(8바이트)만, 글자는 다른 곳에 |

숫자는 크기가 항상 같아서 칸에 바로 들어갑니다. 문자열은 길이가 정해져 있지 않아서 칸에 넣을 수 없으니 "글자는 저기 있음"이라는 주소를 대신 넣는 것입니다. 주소는 무엇을 가리키든 항상 8바이트입니다(64비트 시스템 기준).

### 버전은 숫자 하나로 "압축"한 것

VK_MAKE_API_VERSION(0, 1, 0, 0)은 숫자 네 개처럼 보이지만, 사실 정수 하나로 합쳐 주는 매크로입니다.

```
variant(0) | major(1) | minor(0) | patch(0)
→ 비트 단위로 이어 붙여서 정수 하나: 4194304
```

그래서 4바이트 칸 하나에 직접 들어갈 수 있습니다. 버전을 문자열 "1.0.0"로 줬다면 그것도 포인터가 필요했을 겁니다. Vulkan은 그걸 피하려고 버전을 정수로 표현합니다.

### 같은 원리가 앞으로도 반복됨

```cpp
createInfo.pApplicationInfo = &appInfo;   // 구조체도 크기가 크니 주소로 연결
createInfo.enabledExtensionCount = 0;     // 개수는 숫자 → 직접
createInfo.ppEnabledExtensionNames = ...; // 문자열 목록 → 주소
```

고정 크기의 작은 값은 직접 넣고, 크기가 제각각이거나 큰 데이터는 주소로 연결한다. Vulkan 구조체는 전부 이 규칙으로 되어 있습니다.

## 2. 구조체? 변수? 

엄밀히 말하면 구조체는 "타입"이고, appInfo는 그 타입으로 만든 "변수" 입니다.

```cpp
vk::ApplicationInfo   appInfo{};
//   구조체 (타입)      변수
```

### int와 비교하면 쉬움

| | 타입 (설계도) | 변수 (실제로 만들어진 것) |
|---|---|---|
| 기본 타입 | `int` | `int x = 3;`의 `x` |
| 구조체 | `vk::ApplicationInfo` | `vk::ApplicationInfo appInfo{};`의 `appInfo` |

int가 "정수 하나를 담는 틀"이라면, vk::ApplicationInfo는 "이름, 버전, 엔진 이름… 여러 칸을 묶은 틀" 입니다. 이 틀로 실제 변수를 만들면, 그 변수 안에 칸들이 생깁니다.

```cpp
vk::ApplicationInfo a{};   // 같은 틀로
vk::ApplicationInfo b{};   // 여러 개를 만들 수도 있음
a.pApplicationName = "A";  // 각각 독립된 칸을 가짐
b.pApplicationName = "B";
```

> 구조체 = 여러 값을 묶은 사용자 정의 타입

## 3. struct? class? 

| 층 | 무엇을 정하나 | 해당하는 것 |
|---|---|---|
| ① 타입을 만드는 키워드 | 새 타입을 정의함 | `struct`, `class` |
| ② 접근 지정자 | 그 타입 안의 멤버를 누가 쓸 수 있는지 | `public`, `private` |

- 필드만 있고 숨길 게 없으면 → 보통 struct로 씁니다 (ApplicationInfo)
- 숨길 내부가 있으면 → 보통 class로 쓰고, 그 안을 public:/private:로 나눕니다 (ComputeApplication)

## 4. 필드 

필드(field)는 구조체나 클래스 안에 들어 있는 변수 하나하나를 말합니다. 앞에서 "칸"이라고 표현했던 것이 바로 필드입니다.

```cpp
struct ApplicationInfo
{
    const char* pApplicationName;    // ← 필드
    uint32_t    applicationVersion;  // ← 필드
    const char* pEngineName;         // ← 필드
    uint32_t    engineVersion;       // ← 필드
    uint32_t    apiVersion;          // ← 필드
};
```

## 5. 파이썬 변수는 객체를 가리키는 참조이고, C++의 변수는 객체 그 자체임

같은 코드를 파이썬과 C++에서 실행해 보면 결과가 다릅니다.

### 파이썬: 같은 것을 가리킴

```python
a = ApplicationInfo()
a.api_version = 13

b = a              # b도 "같은 객체"를 가리킴 (복사 아님)
b.api_version = 99

print(a.api_version)   # 99  ← a도 바뀜!
```

```
a ──┐
    ├──▶ [ api_version: 99 ]   객체는 하나뿐
b ──┘
```

파이썬 변수는 객체에 붙인 이름표입니다. b = a는 같은 객체에 이름표를 하나 더 붙이는 것이라서 b로 바꾸면 a에서도 바뀐 게 보입니다.

### C++: 복사본이 생김

```cpp
vk::ApplicationInfo a{};
a.apiVersion = 13;

vk::ApplicationInfo b = a;   // a의 내용을 "복사"해서 새 객체 b를 만듦
b.apiVersion = 99;

std::cout << a.apiVersion;   // 13  ← a는 그대로!
```

```
a: [ apiVersion: 13 ]     ← 상자 하나
b: [ apiVersion: 99 ]     ← 별도의 상자 (처음엔 a 내용을 복사해 옴)
```

C++ 변수는 상자 그 자체입니다. b = a는 a 상자의 내용물을 복사해서 b 상자에 넣는 것이라서 둘은 서로 독립적입니다.

### C++에서 파이썬처럼 "같은 것"을 가리키려면?

그때 쓰는 게 앞에서 배운 포인터입니다. (참조 &라는 방법도 있는데, 나중에 나오면 설명하겠습니다.)

```cpp
vk::ApplicationInfo a{};
vk::ApplicationInfo* p = &a;   // a의 "주소"를 저장 → a를 가리킴
p->apiVersion = 99;            // 포인터로 필드 접근할 때는 -> 사용

std::cout << a.apiVersion;     // 99  ← a가 바뀜
```

## 6. 멤버 변수 vs 지역 변수

차이는 어디에 선언했느냐입니다. 그에 따라 얼마나 오래 사느냐가 달라집니다.

| | 멤버 변수 | 지역 변수 |
|---|---|---|
| 선언 위치 | 클래스 안, 함수 밖 | 함수 안 |
| 예 | `context`, `instance` | `appInfo`, `extensions` |
| 수명 | 객체(`app`)가 살아 있는 동안 | 그 함수가 끝나면 사라짐 |
| 쓸 수 있는 곳 | 그 클래스의 모든 멤버 함수 | 그 함수 안에서만 |

### 파이썬으로 비교

```python
class ComputeApplication:
    def __init__(self):
        self.context = Context()     # 멤버 변수 (self.가 붙음)
        self.instance = None

    def create_instance(self):
        app_info = ApplicationInfo() # 지역 변수 (self. 없음)
```

self.가 붙은 것이 멤버 변수, 안 붙은 것이 지역 변수입니다. C++에서는 self.를 쓰지 않는 대신 선언 위치로 구분합니다.

### 이 코드에서 왜 이렇게 나눴나?

- instance → 멤버 변수: createInstance()가 끝난 뒤에도 계속 써야 합니다. 다음 장들에서 GPU를 찾을 때도 필요합니다. 지역 변수로 만들면 함수가 끝나는 순간 RAII로 해제되어 버립니다.
- appInfo → 지역 변수: 인스턴스를 만들 때 한 번만 쓰고 더는 필요 없습니다.

앞에서 "run()은 반환값 없이 결과를 객체 안에 남긴다"고 했던 그 "객체 안"이 바로 멤버 변수입니다.

## 7. 타입

```
타입 (type)
├─ 기본 타입:   int, char, bool, uint32_t, ...   ← C++에 원래 있는 것
├─ 구조체:      struct로 만든 타입                ← 사용자가 정의
├─ 클래스:      class로 만든 타입                 ← 사용자가 정의
└─ 그 외:       포인터(char*), 배열 등
```

### 지금 코드에 나온 것들로 분류하면

| 이름 | 타입의 종류 |
|---|---|
| `int` (`main`의 반환형) | 기본 타입 |
| `uint32_t` (`apiVersion` 필드) | 기본 타입 (부호 없는 32비트 정수) |
| `const char*` (`pApplicationName` 필드) | 포인터 |
| `vk::ApplicationInfo` | 구조체 (`struct`로 정의됨) |
| `vk::ExtensionProperties` | 구조체 |
| `vk::raii::Context`, `vk::raii::Instance` | 클래스 (`class`로 정의됨) |
| `ComputeApplication` | 클래스 (직접 정의) |
| `std::vector<...>` | 클래스 (표준 라이브러리) |
