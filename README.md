# OS Class Shell

## Compile

With CMake (>= 3.5):
```bash
mkdir build
cmake -S . -B build
cd build
make
```

Without CMake (GCC, etc.):
```bash
mkdir build
gcc src/*.c src/*.h -o build/shell
```

## Execute

```bash
./build/shell
```
