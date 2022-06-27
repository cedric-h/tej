cd build

clang \
  --target=wasm32 \
  -emit-llvm \
  -c \
  -S \
  ../main.c

llc \
  -march=wasm32 \
  -filetype=obj \
  main.ll

wasm-ld \
  --no-entry \
  --export-all \
  -o main.wasm \
  main.o
