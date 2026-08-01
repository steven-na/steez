lib := "steez"
src_dir := "src"
test_dir := "tests"
build_dir := "build"
cc := "cc"
cflags := "-Wall -Wextra -Wpedantic -Werror -std=c23 -fPIC"
lflags := "-lm"
prefix := "/usr/local"

default:
    @just --list

build: build-static build-shared

build-static:
    mkdir -p {{build_dir}}
    cd {{build_dir}} && {{cc}} {{cflags}} -c ../{{src_dir}}/*.c
    ar rcs {{build_dir}}/lib{{lib}}.a {{build_dir}}/*.o

build-shared:
    mkdir -p {{build_dir}}
    cd {{build_dir}} && {{cc}} {{cflags}} -c ../{{src_dir}}/*.c
    {{cc}} -shared -o {{build_dir}}/lib{{lib}}.so {{build_dir}}/*.o {{lflags}}

build-release:
    mkdir -p {{build_dir}}
    cd {{build_dir}} && {{cc}} {{cflags}} -O3 -DNDEBUG -c ../{{src_dir}}/*.c
    ar rcs {{build_dir}}/lib{{lib}}-release.a {{build_dir}}/*.o
    {{cc}} -shared -o {{build_dir}}/lib{{lib}}-release.so {{build_dir}}/*.o {{lflags}}

build-debug:
    mkdir -p {{build_dir}}
    cd {{build_dir}} && {{cc}} {{cflags}} -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer -c ../{{src_dir}}/*.c
    ar rcs {{build_dir}}/lib{{lib}}-debug.a {{build_dir}}/*.o
    {{cc}} -shared -fsanitize=address,undefined -o {{build_dir}}/lib{{lib}}-debug.so {{build_dir}}/*.o {{lflags}}

install: build
    sudo mkdir -p {{prefix}}/include/{{lib}} {{prefix}}/lib
    sudo cp {{src_dir}}/*.h {{prefix}}/include/{{lib}}/
    sudo cp {{build_dir}}/lib{{lib}}.a {{build_dir}}/lib{{lib}}.so {{prefix}}/lib/
    sudo ldconfig

uninstall:
    sudo rm -rf {{prefix}}/include/{{lib}}
    sudo rm -f {{prefix}}/lib/lib{{lib}}.a {{prefix}}/lib/lib{{lib}}.so
    sudo ldconfig

test-all:
    mkdir -p {{build_dir}}
    {{cc}} {{cflags}} {{test_dir}}/*.c {{src_dir}}/*.c -o {{build_dir}}/test -lcriterion {{lflags}}
    ./{{build_dir}}/test

coverage:
    mkdir -p {{build_dir}}/cov-src {{build_dir}}/cov-tests
    cd {{build_dir}}/cov-src && {{cc}} {{cflags}} --coverage -c ../../{{src_dir}}/*.c
    cd {{build_dir}}/cov-tests && {{cc}} {{cflags}} --coverage -c ../../{{test_dir}}/*.c
    {{cc}} {{cflags}} --coverage {{build_dir}}/cov-src/*.o {{build_dir}}/cov-tests/*.o -o {{build_dir}}/test-coverage -lcriterion {{lflags}}
    ./{{build_dir}}/test-coverage
    gcovr --root . --filter '{{src_dir}}/' {{build_dir}}/cov-src --html-details {{build_dir}}/coverage.html --print-summary

clean:
    rm -rf {{build_dir}}

compiledb:
    bear -- just build
