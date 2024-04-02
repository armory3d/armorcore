
**Wasm (Linux, macOS or WSL)**
```bash
../../Kinc/make --from ../.. wasm --compile
# Copy resulting armorcore.wasm file to Deployment
# Copy index.html to Deployment
# Copy https://github.com/Kode/Kinc/tree/main/Backends/System/Wasm/JS-Sources to Deployment
# Todo:
# start.js has hard-coded .wasm file name: https://github.com/Kode/Kinc/blob/main/Backends/System/Wasm/JS-Sources/start.js#L48
# memory.c has hard-coded size: https://github.com/Kode/Kinc/blob/main/miniClib/memory.c#L6
```
