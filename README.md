An implementation of an efficient, multi-threaded, and self-cleaning meeting room booking service using three different languages: **C++20, Rust 1.74** and **C# 11**. It uses *interval trees* to efficiently lookup the meeting schedules and a separate cleaning thread that removes past meetings. The interface is safe to use in a *multi-threaded* environment.

Each project resides in it's own folder so switch to `/rust`, `/cpp` or `/csharp` respectively before executing the operations below.

# Building
## C++
- use `C++ Test Driver build` task

### emscripten bindings
```bash
source ./emsdk/emsdk_env.sh

cd ../cpp/build
emcc -lembind ../src/main.cpp   -O2   -s MODULARIZE=1   -o mrooms.js    -pthread   -sENVIRONMENT=web,worker
   -s PTHREAD_POOL_SIZE=5   -sEXPORT_NAME=mrooms   -s TOTAL_MEMORY=256MB   -I ../include/    -std=c++20

cp mrooms* ../../ui/static/
```

## C#
- use `C# Test Driver build` task

## Rust
```bash
cargo build --release
```

# Running tests
## C++
- use `C++ UnitTests build` task
   - use `-fsanitize=thread` for multi-threading debugging
   - use `-pg` for building profiling binary

## Rust
```bash
cargo test -- --nocapture
```

## C# 
- use `C# UnitTests build` task


# Profiling 
## C++
1. Add `-pg` to compiler options
2. Run test driver binary which generates `gmon.out`
   ```bash
   /build/meeting_rooms
   ```
3. Run `gprof`
   ```bash
   gprof meeting_rooms gmon.out > /tmp/profile.txt
   python3 /tmp/graphdot.py -s -e 5 /tmp/<name>.txt | dot -Tpng -o output.png
   ```

# Rust
```bash
perf record -F99 --call-graph dwarf target/profiling/rust_vs_cpp
```

# C#
```bash
sudo perf record -F99 --call-graph dwarf dotnet exec MeetingRooms/Service/bin/Debug/net7.0/service.dll
sudo chown alinmx perf.data
```

# Profiling general
## How to convert perf.data to firefox profiler file format
```bash
cd <folder_containing_perf.data>
perf script -F +pid > /tmp/test.perf
```