import { test } from "./test.js"

(async () => {
  const wasm = fetch("build/main.wasm");
  const { instance } =
    await WebAssembly.instantiateStreaming(wasm, { env: {
      print: console.log,
      putchar: x => console.log(String.fromCharCode(x)),
    } });

  const canvas = document.getElementById("draw");
  const ctx = canvas.getContext("2d");
  ctx.webkitImageSmoothingEnabled = false;
  ctx.mozImageSmoothingEnabled = false;
  ctx.imageSmoothingEnabled = false;

  canvas.width = (1 << 8);
  canvas.height = (1 << 8);

  /* TODO: call this again on resize? fullscreen canvas? */
  instance.exports.init(canvas.width, canvas.height);

  test(instance.exports);

  /* make an image backed by the WASM module's memory */
  const img = new ImageData(
    new Uint8ClampedArray(
      instance.exports.memory.buffer,
      instance.exports.__heap_base,
      canvas.width * canvas.height * 4
    ),
    canvas.width,
    canvas.height
  );

  /* input handling */
  // window.onkeydown = e => instance.exports.keydown(e.key.charCodeAt(0));

  (function frame() {
    /* generate the pixels */
    instance.exports.draw();

    /* display 'em */
    ctx.putImageData(img, 0, 0);

    /* come back in 16ms */
    requestAnimationFrame(frame);
  })();
})()
