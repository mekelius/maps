# Maps programming language

## How to compile

**!!! untested on a clean machine (if these work without issue please notify me so I can remove this) !!!**

1. install cmake 4
    - if you use snap there's one for it

2. install clang and llvm (version 19)
    - There's a script on their site that installs both with the correct suffix
    - CMakeLists.txt assumes clang and llvm can be found suffixed with "-19", if that's not the case, you might need to do some symlinking or tweak CMakeLists.txt

3. install libedit 
    - for example: ```sudo apt install libedit-dev```
    - libedit is only needed for targets mapsci and e2e_tests. If you build just the target mapsc this isn't needed

4. (optional) if you want to run the tests:
    - ```pip install --user lit```

5. configure the build directory

```
mkdir build
cmake -B build # (-DCMAKE_BUILD_TYPE=Release except there's no releases yet)
```

6. build 
```
cmake --build build             # compiler, repl and verifier
cmake --build build -t mapsc    # just the compiler (in case you don't have libedit)
cmake --build build -t tests    # build and run the tests
```

This produces 3 executables:
    - compiler: ```./build/mapsc```
    - repl: ```./build/mapsci```
    - verifier: ```./build/mapsc-verify```
        - the verifier is a tool intended to run some checks to catch incorrect compilations. It's pretty bare bones at the moment, but should prove useful once it's fleshed out. To use it, just give it a list of source files to check, and it will test whether the frontend is able to parse and process them and whether the results are consistent. It will print out a "reverse parse" for each file that you can compare to the original.

## Dependency versions used

- general
    - cmake: 4.0.0
    - clang: 19.1.7
    - llvm 19

- for the REPL:
    - libedit 3.1

- for unit and integration tests:
    - doctest 2.4.11 (included in the repo)

- for e2e tests
    - lit 18.1.8
