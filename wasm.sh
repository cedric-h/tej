cd build

# doesn't seem to work in Chrome, rip

 clang \
   --target=wasm32 \
   -mbulk-memory \
   -O3 \
   -flto \
   -nostdlib \
   -Wl,--no-entry \
   -Wl,--export-all \
   -Wl,--lto-O3 \
   -Wl,--allow-undefined \
   -o main.wasm \
   ../main.c
