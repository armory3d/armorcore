# armorcore

3D engine core for C with JS scripting. ArmorCore targets Direct3D12, Vulkan, Metal and WebGPU. Browser support is handled by compiling C sources into WebAssembly.

Powered by [Kinc](https://github.com/Kode/Kinc) - low-level hardware abstraction library.

*(wip)*

```bash
git clone --recursive https://github.com/armory3d/armorcore
cd armorcore
```

**Windows (x64)**
```bash
Kinc/make -g direct3d12
# Open generated Visual Studio project at `build\Armory.sln`
# Build for x64 & release
```

**Linux (x64)**
```bash
Kinc/make -g vulkan --compiler clang --compile
```

**macOS (arm64)**
```bash
Kinc/make -g metal
# Open generated Xcode project at `build/Armory.xcodeproj`
# Build
```

**Android (arm64)**
```bash
Kinc/make -g vulkan android
# Manual tweaking is required for now:
# https://github.com/armory3d/armorcore/blob/main/kfile.js#L134
# Open generated Android Studio project at `build/Armory`
# Build
```

**iOS (arm64)**
```bash
Kinc/make -g metal ios
# Open generated Xcode project at `build/Armory.xcodeproj`
# Build
```
