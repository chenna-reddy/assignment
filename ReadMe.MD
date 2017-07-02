# Assignment

### Compilation

```
cmake -H. -Bbuild
cmake --build build -- -j3
```

## Game

### Usage

```
game [Number of Games]
```

## Concurrent Queue

### Problems
1. `pop` should not return `const T&` instead it must return `T`.
By the time consumer take a copy of return value of pop, some other producer might have override value at that location.
This should fix the problem for Single Producer and Single Consumer.
2. 




