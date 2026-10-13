## 00. 용어

| 이름 | 한글 | 뜻 |
|---|---|---|
| descriptor | 디스크립터 | "이 자원은 저 버퍼의 여기부터 몇 바이트까지"라고 셰이더에게 알려 주는 연결 쪽지 한 장 |
| descriptor set | 디스크립터 셋 | 디스크립터 여러 장을 묶은 것. 셰이더를 실행할 때 통째로 넘김 (07장) |
| descriptor set layout | 디스크립터 셋 레이아웃 | 그 묶음의 양식(칸 배치표). "0번 칸에 스토리지 버퍼 1개, ..." (06장) |
| binding | 바인딩 | 양식 안의 칸 번호. 06장에서는 "연결하다"(동사)가 아니라 "자리"(명사) |
| storage buffer | 스토리지 버퍼 | 셰이더가 읽고 쓸 수 있는 버퍼 **종류**. `inputA`/`inputB`/`output`은 그 종류의 실제 버퍼 |
| uniform buffer | 유니폼 버퍼 | (참고) 셰이더가 읽기만 하는 작은 버퍼. 설정값·상수용 |

descriptor는 *describe*(설명하다)에서 온 이름이에요.

## 01. 왜 descriptor가 필요한가

셰이더는 GPU에서 도는 별도 프로그램이라 C++ 변수 `inputA`를 직접 볼 수 없어요. 셰이더 코드에는 번호만 있어요.

```glsl
layout(binding = 0) buffer A { float a[]; };
```

"0번이 실제로 어느 버퍼냐"를 연결해 주는 쪽지가 descriptor예요.

```python
# 06장: layout = 양식 (칸 번호와 종류만, 내용은 비어 있음)
layout = {0: "storage_buffer", 1: "storage_buffer", 2: "storage_buffer"}

# 07장: descriptor set = 양식대로 채운 실제 묶음 (데이터를 복사하지 않고 참조만 함)
descriptor_set = {0: inputA, 1: inputB, 2: output}

shader(descriptor_set)
```

양식과 내용을 나누는 이유: 양식이 같으면 내용(버퍼)만 바꿔 끼울 수 있어요. 함수 시그니처는 그대로 두고 인자만 바꿔 호출하는 것과 비슷해요.

## 02. 누가 무엇을 가지고 있나

| | 가진 것 |
|---|---|
| 버퍼 | 실제 데이터 (입력값, 결과값). 만들 때 크기가 고정됨 (05장) |
| descriptor set layout | 칸 배치 규칙 (몇 번 칸에 어떤 종류가 몇 개, 어느 셰이더가 씀) |
| descriptor set | 각 칸이 **어느 버퍼를 가리키는지** (07장) |
| 컴퓨트 셰이더 | 공식. 어느 칸을 읽고 어느 칸에 쓸지(입력/출력)도 여기서 정해짐 |

layout에 **없는** 것:

- 실제 버퍼 이름 → 그래서 06장 코드에 `inputA`가 안 나옴
- 버퍼 크기 → 같은 layout에 8개짜리 버퍼도, 100만 개짜리 버퍼도 꽂을 수 있음. 버퍼의 몇 바이트를 쓸지(offset, range)는 07장 `bufferInfos`에서 정함
- 입력인지 출력인지 → 0, 1, 2번 모두 똑같이 `eStorageBuffer`

### 빵 공장 비유

| Vulkan | 비유 |
|---|---|
| 컴퓨트 셰이더 | 레시피 (재료 A와 B를 섞어 굽는다). 같은 레시피를 원소마다 동시에 적용 |
| descriptor set layout | 기계의 투입구 배치도 (0번 구멍 밀가루, 1번 설탕, 2번 완성품 출구) |
| descriptor set | 구멍에 실제로 연결한 재료 통 |
| 버퍼 | 재료 통 그 자체 |
| dispatch | "8개 만들어!" 버튼 |

셰이더를 빵틀로 보면 "같은 걸 여러 개 동시에"는 맞지만, 셰이더는 모양이 아니라 계산(가공)을 하므로 레시피에 더 가까워요.

### AI 모델에 비유하면

| 이 튜토리얼 | 모델 추론 |
|---|---|
| `inputA` | 모델 입력 (이미지, 토큰 임베딩) |
| `inputB` | 가중치 |
| `output` | 출력 (logits, 다음 층 활성값) |
| 컴퓨트 셰이더 | 연산 하나 (커널: 덧셈, 행렬곱) |

실제 모델은 버퍼·바인딩이 훨씬 많고, 중간 결과는 CPU로 안 돌아오고 GPU 안(`eDeviceLocal`)에서만 오가요. PyTorch `y = x + w`가 내부에서 해 주는 일을 손으로 하는 셈이에요.

## 03. `vk::DescriptorSetLayoutBinding` 필드

| 필드 | 우리 값 | 뜻 |
|---|---|---|
| `binding` | `i` (0, 1, 2) | 칸 번호. 셰이더의 `layout(binding = N)`과 맞춤 |
| `descriptorType` | `eStorageBuffer` | 칸에 오는 자원 종류 |
| `descriptorCount` | `1` | 칸 하나에 들어가는 **버퍼 개수** (버퍼 안 원소 수가 아님!) |
| `stageFlags` | `eCompute` | 이 칸을 쓰는 셰이더 단계 |

`descriptorCount`가 3이면 칸 하나에 버퍼 3개가 배열로 들어가요 (`bufs[0]`, `bufs[1]`, ...). 같은 종류가 많고 셰이더가 번호로 골라야 할 때 써요. 이 예제는 역할이 다 다르니 "칸 3개 × 각 1개"가 읽기 쉬워요.

Vulkan은 칸 종류와 개수를 기본적으로 미리 다 정해야 해요. 파이프라인을 만들 때 드라이버가 셰이더를 미리 최적화하려고요. (참고: 나중 기능인 descriptor indexing, 일명 bindless로 느슨하게 할 수도 있음)

## 04. 레이아웃 만들기

```cpp
vk::DescriptorSetLayoutCreateInfo layoutInfo{};
layoutInfo.bindingCount = static_cast<std::uint32_t>(bindings.size());
layoutInfo.pBindings = bindings.data();
descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
```

- layout은 **1개**, 그 안의 칸 설명(binding)이 **3개**. `bindingCount = 3`은 "양식 안에 칸이 3개"라는 뜻
- `static_cast<T>(값)` (정적 형 변환): `.size()`는 `std::size_t`(64비트), 필드는 `std::uint32_t`(32비트)라 명시적으로 바꿈. 파이썬 `int(x)` 같은 것
- `.data()`: 배열 첫 칸의 주소. 필드 이름 앞 `p` = pointer

### `pXxx` + `xxxCount` 짝 패턴

C 스타일 API는 배열을 "**첫 칸 주소 + 개수**"로 받아요. 주소에는 끝이 어딘지 정보가 없어서 개수를 따로 줘야 해요.

```python
make_layout(bindings)          # 파이썬: 길이가 따라옴
make_layout(start_address, 3)  # C: "여기서부터 3개 읽어"
```

- 개수를 너무 작게 주면 뒤 칸을 무시하고, 크게 주면 배열 밖 메모리를 읽어 오동작
- 숫자 대신 `bindings.size()`를 써서 자동으로 맞춤
- 같은 패턴: `enabledLayerCount`/`ppEnabledLayerNames`(02장), `queueCreateInfoCount`/`pQueueCreateInfos`(04장)

## 05. 빌드 결과

```
Descriptor set layout created: 3 bindings.
Descriptor set layout destroyed.
```

- 첫 줄: `createDescriptorSetLayout()`이 `initVulkan()`에서 호출됐고 칸 3개로 layout이 만들어짐. 3 = `bindings` 배열 크기
- 둘째 줄: 끝날 때 `vk::raii`가 자동 정리
- 검증 레이어 경고 없음
- cmake의 `missing components: glslangValidator`는 무시해도 됨 (`glslc`는 있음)
