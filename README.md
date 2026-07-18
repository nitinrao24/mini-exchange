# Mini Exchange in C++

An in-memory single-symbol limit order book and matching engine implementing
price-time priority. Built as a quant-dev portfolio project.

## Current status

Stage 1 complete: OrderBook primitives with full unit test coverage.

## Build

```bash
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

## Test

```bash
cd build && ctest --output-on-failure
```
