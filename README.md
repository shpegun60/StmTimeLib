# StmTimeLib

Header-only timing library for STM32 projects. Provides lightweight interval timers with both compile-time and runtime intervals, one-shot variants, and “virtual” timers backed by SysTick. Designed for zero-overhead inlining, safe unsigned wrap-around, and clear compile-time diagnostics.

## Highlights

- **Unified API**: `StackITimer<Interval, T>` and `StackVTimer<Interval, T>`  
  - `Interval == 0` → dynamic interval (set at runtime)  
  - `Interval != 0` → static interval (known at compile time)
- **One-shot timers**: `OneShotITimer`, `OneShotVTimer` (+ adapters)
- **Policy-driven `now()`**: adapters `ITimeBase`, `VTimeBase`, `OneShotIBase`, `OneShotVBase` call `Policy::now()` internally
- **Unsigned wrap-around correct** arithmetic
- **Compile-time guarantees**:  
  - forbids changing interval in static mode  
  - checks presence of `setInterval()` for dynamic policies  
  - verifies `Policy::now()` exists and returns a convertible type
- **Zero RTTI, no virtuals**, small and inlinable
- **STM32-friendly**: integrates with HAL `uwTick` and SysTick

## Requirements

- **C++17** or newer
- STM32 toolchain (GCC/Clang/ARMCC)
- HAL (for `uwTick`) if you use the provided `Tick` policy and `VTimer` backend

## Directory layout (typical)

```
time/
  interval_policy.h        // StaticIntervalPolicy, DynamicIntervalPolicy
  StackITimer.h
  OneShotITimer.h
  interval/
    ITimeBase.h
    OneShotIBase.h
  virtual/
    VTimer.h               // virtual timers driven by SysTick
    StackVTimer.h
    VTimeBase.h
    OneShotVBase.h
  OneShotVTimer.h
  Time.h                   // Tick policy + ready-made aliases
```

> Paths and header names should match your repo. Adjust includes accordingly.

## Core concepts

### Interval policies

```cpp
// Static interval: known at compile time
template<typename T, T Interval>
struct StaticIntervalPolicy {
  static_assert(Interval > T{0}, "Interval must be > 0");
  static constexpr T getInterval() noexcept { return Interval; }
  StaticIntervalPolicy() = default;
  StaticIntervalPolicy(T) = delete; // no runtime setting
};

// Dynamic interval: settable at runtime
template<typename T>
struct DynamicIntervalPolicy {
  T interval;
  explicit constexpr DynamicIntervalPolicy(T iv = T{}) noexcept
    : interval(normalize(iv)) {}
  constexpr void setInterval(T iv) noexcept { interval = normalize(iv); }
  constexpr T getInterval() const noexcept { return interval; }
private:
  static constexpr T normalize(T iv) noexcept {
    if constexpr (std::is_signed_v<T>) return iv < T{0} ? -iv : iv;
    else return iv;
  }
};
```

### Stack timers (plain interval timers)

#### `StackITimer<Interval = 0u, T = reg>`

- Works with any monotonically increasing `now` counter that fits `T`.
- `T` must be **unsigned integral**. Wrap-around is handled by unsigned arithmetic.
- `Interval == 0` → dynamic; `Interval != 0` → static.

Key API:

```cpp
bool isExpired(T now) const;
T    timeLeft(T now) const;
void next(T now);
void next(T now, T interval);  // only in dynamic mode (and only if policy has setInterval)
T    elapsed(T now) const;
static constexpr bool isAvailable();
```

Compile-time safety:
- `next(now, interval)` is a **static error** in static mode.
- In dynamic mode, `next(now, interval)` is a static error if the policy doesn’t offer `setInterval(T)`.

#### `ITimeBase<Interval, Policy>`

Thin adapter over `StackITimer` that **calls `Policy::now()` internally**.  
Constructors auto-arm the timer on creation.

Key API:

```cpp
bool isExpired() const;
T    timeLeft() const;
void next();               // uses Policy::now()
void next(T interval);     // dynamic only
T    elapsed() const;
```

### One-shot timers

#### `OneShotITimer<Interval = 0u, T = reg>`

Returns `true` exactly once when the interval expires, then stays false until re-armed.

Key API:

```cpp
bool isExpired(T now);     // non-const; flips internal state on first expiry
void start(T now);         // arm from now
void start(T now, T interval); // dynamic only
void next(T now);          // re-arm (requires started=true)
void next(T now, T interval);  // dynamic only
void stop();
bool isStopped() const;
```

Adapters:

- `OneShotIBase<Interval, Policy>` uses `Policy::now()` internally.  
  Constructors auto-start:
  - dynamic: `start(now, initial_interval)`
  - static:  `start(now)`

### Virtual timers (SysTick-driven)

#### `VTimer`

A lightweight list of countdown timers decremented in SysTick context.  
You should call the library’s tick hook from your SysTick handler.

Integration options:

```cpp
// Option A: explicit hook from your handler
extern "C" void SysTick_Handler(void) {
  HAL_IncTick();          // your HAL tick
  VTimer_SysTickHook();   // call library hook to service virtual timers
}

// Option B: provide a weak HAL_SYSTICK_Callback override (if library offers it)
// Prefer Option A to avoid conflicts with other code.
```

#### `StackVTimer<Interval = 0u, T = reg>`

Combines `VTimer` backend with interval policy.  
`next(now)` translates to `VTimer::next(policyInterval)` and stores `now` for `elapsed(now)`.

Key API:

```cpp
void next(T now);
void next(T now, T interval);  // dynamic only
T    elapsed(T now) const;
static constexpr bool isAvailable();
```

#### `VTimeBase<Interval, Policy>`

Adapter over `StackVTimer` that calls `Policy::now()` internally.

Key API:

```cpp
void next();
T    elapsed() const;
```

#### `OneShotVTimer` and `OneShotVBase`

One-shot semantics on top of `StackVTimer`.  
`OneShotVBase<Interval, Policy>` adapts to `Policy::now()`.

## Ready-made policy and aliases (`Time.h`)

```cpp
// HAL uwTick policy
class Tick {
public:
  using type_t = u32;                 // your typedef for uint32_t
  static inline type_t now() noexcept { return uwTick; }
  static constexpr bool isAvailable() noexcept { return true; }
};

// Handy aliases
template<auto Interval = 0u>
using TickITimer   = ITimeBase<Interval, Tick>;
template<auto Interval = 0u>
using OneShotITick = OneShotIBase<Interval, Tick>;

template<auto Interval = 0u>
using TickVTimer   = VTimeBase<Interval, Tick>;
template<auto Interval = 0u>
using OneShotVTick = OneShotVBase<Interval, Tick>;
```

> Ensure `extern "C" volatile uint32_t uwTick;` is visible (via HAL headers or your own declaration).

## Quick start

### Static interval, plain stack timer

```cpp
#include "time/StackITimer.h"

using Tmr100ms = StackITimer<100u, uint32_t>;

void loop(uint32_t now_ms) {
  static Tmr100ms t;      // interval baked in at compile time
  if (t.isExpired(now_ms)) {
    t.next(now_ms);       // re-arm
    // do work
  }
}
```

### Dynamic interval, plain stack timer

```cpp
#include "time/StackITimer.h"

using Tmr = StackITimer<0u, uint32_t>;   // dynamic
Tmr t(50);                               // initial interval = 50

void loop(uint32_t now_ms) {
  if (t.isExpired(now_ms)) {
    t.next(now_ms, 100);                 // change interval at runtime
  }
}
```

### One-shot timer with policy-based now()

```cpp
#include "time/Time.h" // Tick + adapters

OneShotITick<> shot(200);  // dynamic one-shot, initial interval = 200

void loop() {
  // auto-armed in ctor if you used the adapter variant that starts in ctor
  if (shot.isExpired()) {
    // fires exactly once
  }
}
```

### Virtual timer (SysTick-driven)

```cpp
#include "time/virtual/StackVTimer.h"
#include "time/Time.h"

// call VTimer_SysTickHook() from SysTick_Handler (see Integration above)

TickVTimer<50> vt; // static 50-tick “virtual” timer

void loop() {
  vt.next(Tick::now());                 // starts countdown in VTimer
  while (!vt.elapsed(Tick::now())) {    // or check timeLeft() via StackITimer path
    // spin or sleep…
  }
}
```

## Type and semantic rules

- `T` must be **unsigned integral** (e.g., `uint16_t`, `uint32_t`, `uint64_t`).  
- Unsigned wrap-around semantics are relied upon; never cast to signed for comparisons.  
- For **static interval**, the compile-time constant must fit into `T`.  
- For dynamic policies:
  - `setInterval(T)` is required to use `next(now, interval)`; otherwise compile-time error.
  - Intervals are normalized to non-negative (for signed policies where applicable).

## ISR & concurrency notes

- Plain stack timers (`StackITimer`) are lock-free by design if you:
  - update `lastTime` only in main context,
  - compute `now` in a single-word type.
- For `VTimer` backend:
  - All container mutations happen under an IRQ guard inside library code.
  - Your `next(delay)` that only stores a single-word value is safe without guard if you call it only after `isExpired() == true` (counter is zero in ISR path).

## Error messages you may see

- `"Cannot set interval on a static <...>Timer"`  
  You called `next(now, interval)` on a static timer.

- `"selected policy does not provide setInterval(T)"`  
  Your dynamic policy lacks `setInterval`.

- `"Policy must provide static now() convertible to type_t"`  
  Adapters require `Policy::now()`.

## Build

Just include the headers in your project. No sources to compile.  
Enable C++17 or newer, optimize with `-O2` or above.

Example flags:

```
-std=c++17 -O2 -ffunction-sections -fdata-sections -Wall -Wextra -Werror
```

## FAQ

**Q: Can I use `uint16_t` as the time base?**  
A: Yes. Wrap-around occurs more frequently, but arithmetic stays correct.

**Q: Do I need to protect `next(delay)` with IRQ guard?**  
A: Not if you call it only when `isExpired()==true` and the counter is single-word. Otherwise, protect.

**Q: Why unsigned only?**  
A: To make wrap-around and comparisons well-defined and fast.

**Q: How do I hook SysTick for virtual timers?**  
A: Call `VTimer_SysTickHook()` from your SysTick handler (see snippet above).

## License

See `LICENSE` in the repository.
