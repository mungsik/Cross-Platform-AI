## command buffer

- GPU에게 시킬 명령(command)를 미리 기록해 두는 객체
- 일을 바로 실행하지 않고, 목록으로 적어 둔 뒤 **Queue**에 한꺼번에 submit(제출)

## command pool

- 명령 기록에 필요한 메모리를 모아 관리하고, 커멘드 버퍼를 할당하는 객체
- 같은 풀의 기록용 메모리를 재사용하거나 함께 초기화할 수 있도록 할당 단위를 분리한 것

## Primary Command Buffer

| 레벨 | 이름 | 큐에 직접 제출 | 실행 방법 |
|---|---|---|---|
| `ePrimary` | 프라이머리 (primary, 주) | 가능 | `queue.submit()` |
| `eSecondary` | 세컨더리 (secondary, 보조) | 불가능 | 프라이머리 안에서 `executeCommands()`로 호출 |

- primary
    - 큐에 바로 제출하는 command buffer. 
    - 하나만 쓰는게 아님. 몇 개든 만들 수 있음. 각각이 독립적으로 제출 가능한 작업 묶음임.
        - 예를 들어, 작업 종류별로, 데이터 업로드용·계산용 프라이머리를 나눔
- secondary
    - 혼자서는 제출할 수 없음. primary command가 `vkCmdExecuteCommands`로 불러서 실행함
    - 멀티스레드 기록할 때 사용. 스레드마다 하나씩 기록하고, 메인 스레드가 프라이머리 하나에 모아서 제출
    - 재사용할 때 사용. 바뀌지 않는 명령 묶음을 세컨더리로 한 번 기록해두고 매 프레임 프라이머리에서 호출.

## Command Buffer Lifecycle

- **begin recording** 이란 것은 command buffer를 **명령을 받아 적는 상태**로 바꾸는 것
- `vkCmd*` : `vkCmd`로 시작하는 모든 함수

```text
allocate          begin()             end()               submit()
────────▶ initial ────────▶ recording ────────▶ executable ────────▶ pending
                             ▲                                         │
                             │ 여기서만 vkCmd* 가능          GPU 실행 완료 시
                             │                                         ▼
                                                              executable로 복귀
```

| 상태 | 의미 | 할 수 있는 일 |
|---|---|---|
| `initial` | 할당 직후, 비어 있음 | `begin()` |
| `recording` | 기록 중 | `bindPipeline`, `dispatch` 등 `vkCmd*` 호출 |
| `executable` | 기록 완료 | 큐에 제출 |
| `pending` | 제출되어 GPU가 실행 중 | 기다리기 (수정 금지) |
