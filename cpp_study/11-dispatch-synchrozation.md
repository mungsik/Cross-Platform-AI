## Buffer vs. Command Buffer

- 버퍼(buffer, `vk::Buffer`) : 데이터를 담음. inputA, inputB, output
- 커멘드 버퍼(command buffer, `vk::CommandBuffer`) : GPU 명령을 담음. 

## fence & memory barrier 

- fence : `vk::raii::Fence`, GPU가 이 제출을 끝냈는지 CPU가 확인하고 기다리는 객체
- memory barrier : 셰이더가 쓴 결과가 CPU 읽기에 보이도록 보장하는 명령

### 이렇게 하는 이유

`submit()은 커멘드 버퍼를 큐에 넘기고 바로 반환함. GPU가 계산을 끝날 때까지 기다리지 않음. 그래서 submit 직후 CPU가 output을 읽으면 아직 빈 값(0)을 읽을 수 있음