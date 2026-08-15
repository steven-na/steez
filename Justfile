lib := "steez"
src_dir := "src"
test_dir := "tests"
build_dir := "build"
cc := "cc"
cflags := "-Wall -Wextra -Wpedantic -Werror -std=c23 -fPIC -march=native"
lflags := "-lm -lpthread"
prefix := "/usr/local"
extra_cflags := ""

default:
    @just --list

build: build-static build-shared

gen-version type="release":
    ./scripts/gen-version.sh {{type}}

build-static: (gen-version "release")
    mkdir -p {{build_dir}}
    cd {{build_dir}} && {{cc}} {{cflags}} {{extra_cflags}} -c ../{{src_dir}}/unity.c
    ar rcs {{build_dir}}/lib{{lib}}.a {{build_dir}}/*.o

build-shared: (gen-version "release")
    mkdir -p {{build_dir}}
    cd {{build_dir}} && {{cc}} {{cflags}} {{extra_cflags}} -c ../{{src_dir}}/unity.c
    {{cc}} -shared -o {{build_dir}}/lib{{lib}}.so {{build_dir}}/*.o {{lflags}}

build-release: (gen-version "release")
    mkdir -p {{build_dir}}
    cd {{build_dir}} && {{cc}} {{cflags}} {{extra_cflags}} -O3 -DNDEBUG -c ../{{src_dir}}/unity.c
    ar rcs {{build_dir}}/lib{{lib}}-release.a {{build_dir}}/*.o
    {{cc}} -shared -o {{build_dir}}/lib{{lib}}-release.so {{build_dir}}/*.o {{lflags}}

build-debug: (gen-version "debug")
    mkdir -p {{build_dir}}
    cd {{build_dir}} && {{cc}} {{cflags}} {{extra_cflags}} -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer -c ../{{src_dir}}/unity.c
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

install-vendor dir: build
    #!/usr/bin/env bash
    set -euo pipefail
    dir="{{dir}}"
    dir="${dir%/}"
    vendor_dir="$dir/vendor/{{lib}}"
    mkdir -p "$vendor_dir/include/{{lib}}" "$vendor_dir/lib"
    cp {{src_dir}}/*.h "$vendor_dir/include/{{lib}}/"
    cp {{build_dir}}/lib{{lib}}.a {{build_dir}}/lib{{lib}}.so "$vendor_dir/lib/"
    echo "Installed lib{{lib}} into $vendor_dir"

    target_justfile="$dir/Justfile"
    if [ -f "$target_justfile" ] && grep -q '^cflags :=' "$target_justfile" && grep -q '^lflags :=' "$target_justfile"; then
        echo "Detected ctmplt-layout project at $dir; wiring up build flags"
        inc_flag="-Ivendor/{{lib}}/include"
        static_lib="vendor/{{lib}}/lib/lib{{lib}}.a"
        if ! grep -qF -- "$inc_flag" "$target_justfile"; then
            sed -i "s|^cflags := \"\(.*\)\"|cflags := \"\1 $inc_flag\"|" "$target_justfile"
        fi
        if ! grep -qF -- "$static_lib" "$target_justfile"; then
            sed -i "s|^lflags := \"\(.*\)\"|lflags := \"\1 $static_lib\"|" "$target_justfile"
        fi
        echo "Updated $target_justfile (cflags/lflags)"
    else
        echo "$target_justfile doesn't match the ctmplt bin/lib layout (no cflags:=/lflags:= Justfile); skipping auto-link wiring"
    fi

uninstall-vendor dir:
    rm -rf {{dir}}/vendor/{{lib}}

test-all: (gen-version "debug")
    mkdir -p {{build_dir}}
    {{cc}} {{cflags}} {{extra_cflags}} {{test_dir}}/*.c {{src_dir}}/unity.c -o {{build_dir}}/test -lcriterion {{lflags}}
    ./{{build_dir}}/test

coverage: (gen-version "debug")
    mkdir -p {{build_dir}}/cov-src {{build_dir}}/cov-tests
    cd {{build_dir}}/cov-src && {{cc}} {{cflags}} --coverage -c ../../{{src_dir}}/unity.c
    cd {{build_dir}}/cov-tests && {{cc}} {{cflags}} --coverage -c ../../{{test_dir}}/unity.c
    {{cc}} {{cflags}} --coverage {{build_dir}}/cov-src/*.o {{build_dir}}/cov-tests/*.o -o {{build_dir}}/test-coverage -lcriterion {{lflags}}
    ./{{build_dir}}/test-coverage
    gcovr --root . --filter '{{src_dir}}/' {{build_dir}}/cov-src --html-details {{build_dir}}/coverage.html --print-summary

clean:
    rm -rf {{build_dir}}

compiledb:
    bear -- just build
