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
2. It's not a good practice to return `mEmpty` from `pop` when there is nothing to pop.
Its better to return `false` and pass value by reference through arguments. 
3. In `push` function, `busyWaitForPush` is invoked without guard. Its possible that two producers find there is space,
and end up inserting one element each, which would overwrite non-consumed location.
 
 

### What steps would you take to find the bug?
1. Code review 
2. Its very unlikely that multi-threading bugs can be found using debugger. Following test scenarios shall help finding few issues:  
    a. More producers than number of threads on hardware
    b. Less producers than number of threads on hardware
    c. Introducing sleep in Producers to emulate slow and fast producers
    d. Introducing sleep in Consumer to emulate slow and fast consumer
3. Reconciliation between producer and consumer


### If you can find the bug, what fix would you suggest and Improvements
1. Instead of using Mutex using Atomic Operations with CAS
2. Consumer can consume elements in batch (Remove all elements at once)
3. Spin Strategy can be parameterized instead of using busy spin always (To make it more generic)
4. As this only works with Single Consumer, there should be a way of validating consumer, just to avoid mis-use.