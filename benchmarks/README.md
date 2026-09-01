# Benchmark gates

Build each benchmark with the same board, F_CPU, compiler and optimization flags. Record `.text/.data/.bss`, cycles/op (or timer ticks), and generated assembly.

Required comparisons:

- GPIO: runtime `digitalWrite`, constant `digitalWrite`, `fastioWrite`, direct PORT
- ADC: compensated `analogRead`, `analogReadFast`, event-trigger/monitor overhead
- uDSC: C/avr-gcc vs uDSC multiply, MAC, divide and dot product
- Wire/RAM: default vs Lean/Tiny profiles
- Timer: software shutdown ISR vs hardware fault shutdown latency

An optimization is accepted only if it fixes correctness, moves repeated work to hardware, or meets the PRD size/performance threshold. Do not claim benchmark wins from source inspection alone.
