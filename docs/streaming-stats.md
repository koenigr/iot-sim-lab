## Ring buffer

`stats::RingBuffer<T, N>` (`stats/include/stats/ring_buffer.hpp`, header-only)

Fixed-size circular buffer backed by `std::array<T, N>`, capacity `N > 0`, element type `T`. No heap allocation.  
Holds the last `N` sensor measurements.

`ConstIterator` is a forward iterator (`std::forward_iterator_tag`). It stores a pointer to the buffer and a logical index, and provides `operator*`, `operator->`, `operator++` (prefix/postfix), `operator==`, `operator!=`.

The oldest element is not necessarily at array index 0. `head_` is the slot of the next write, `count_` the number of stored elements. Example: `N = 5` after `push(1) ... push(7)`:

```mermaid
flowchart LR
    subgraph data_["data_ (N = 5), physical slots"]
        direction LR
        S0["[0]: 6"] --- S1["[1]: 7"] --- S2["[2]: 3"] --- S3["[3]: 4"] --- S4["[4]: 5"]
    end
    HEAD(["head_ = 2<br/>next push writes here"]) --> S2
    OLD(["begin(): rb[0]<br/>oldest = 3"]) --> S2
    NEW(["rb[4]<br/>newest = 7"]) --> S1
```


## Filters: MA vs. EWMA vs. median

## Welford and numerical stability

`stats/include/stats/welford.hpp`, header-only

Computes mean and variance one sample at a time in O(1) memory, without storing samples.

State: `n`, `mean`, `m2` (sum of squared deviations from the current mean).

Per sample `x`:
```
  n     += 1
  delta = x - mean
  mean  += delta / n
  m2    += delta * (x - mean)
```

Results: `mean`, sample variance `m2 / (n - 1)`, population variance `m2 / n`.

Why not `E[x²] − E[x]²`: it subtracts two large, nearly equal numbers and loses precision to cancellation (e.g. data around 1e9). Welford only works with small deltas and stays numerically stable.

Edge cases: sample variance is NaN for `n < 2`.

In `iot-sim-lab`, Welford runs in the collector to keep a running mean and stddev per sensor stream, e.g. a temperature node. Each new reading gets `z = (x - mean) / stddev`. If `|z|` exceeds the threshold (e.g. 3), it is flagged as an anomaly (a stuck sensor, a spike, a dropped value). It needs `O(1)` memory, no sample buffer, and is numerically stable over long runs. For a stationary signal, this is the baseline for “what is normal”.

## Fixed-Point (Q16.16)

`stats/include/stats/ewma_fixed.hpp`, header-only

Integer-only exponentially weighted moving average, no `float`/`double`.
Smooths a noisy signal with one output per input sample.

$$y_n = y_{n-1} + \alpha \, (x_n - y_{n-1})$$

- Values and alpha are Q16.16 (`int32_t`, LSB = 2^-16, range ±32768).
- alpha in [0, 1.0] (`0 .. 65536`); alpha = 0.25 -> `16384`. Out-of-range values are clamped.
- The product is computed in `int64_t`, rounded half up (+0.5 LSB before `>> 16`).
- No overflow: with alpha in [0, 1] the result always lies between `y` and `x`.
- The first sample initializes the state (`y = x`).
- Error vs. the double version is bounded by about 0.5/alpha LSB (see `ewma_fixed_test.cpp`).

Smoothing a noisy temperature or RSSI reading on a node without an FPU
(e.g. Cortex-M0/M3, STM32F407 without float use in the ISR path).
Small alpha -> smooth but slow; large alpha -> fast but noisy.
Use it as the filtered value for display or control; use Welford (not EWMA)
if you need mean/stddev for anomaly detection.

## Benchmark results
