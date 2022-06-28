cd build

zig build-lib \
  --export=init \
  --export=keydown \
  --export=draw \
  -dynamic -target wasm32-freestanding ../main.c

#  clang \
#    --target=wasm32 \
#    -mbulk-memory \
#    -O3 \
#    -flto \
#    -nostdlib \
#    -Wl,--no-entry \
#    -Wl,--export-all \
#    -Wl,--lto-O3 \
#    -Wl,--allow-undefined \
#    -o main.wasm \
#    ../main.c
