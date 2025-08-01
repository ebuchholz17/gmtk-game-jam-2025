#!/bin/sh
mkdir -p ../../build/web/assets

tsc

pushd ../../build/web

clang --target=wasm32 -g -nostdlib -fvisibility=hidden -Wl,--no-entry  -Wl,--export-dynamic -Wl,--import-memory -D _GNG_WEB -Wl,--allow-undefined-file=../../code/web/wasm.syms ../../code/web/web_platform.c -o gng.wasm 

cp ../../code/web/index.html index.html
cp -R ../../assets ./

popd
