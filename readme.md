# armorcore

3D engine core for C with JS scripting. ArmorCore targets Direct3D12, Vulkan, Metal and WebGPU.

Powered by [Kinc](https://github.com/Kode/Kinc) - low-level hardware abstraction library.

```bash
git clone https://github.com/armory3d/armorcore
cd armorcore
```

**Windows (x64)**
```bash
./make --graphics direct3d12
# Open generated Visual Studio project at `build\Armory.sln`
# Build for x64 & release
```

**Linux (x64)**
```bash
./make --graphics vulkan --compile
```

**macOS (arm64)**
```bash
./make --graphics metal
# Open generated Xcode project at `build/Armory.xcodeproj`
# Build
```

**Android (arm64)**
```bash
./make --graphics vulkan --target android
# Open generated Android Studio project at `build/Armory`
# Build
```

**iOS (arm64)**
```bash
./make --graphics metal --target ios
# Open generated Xcode project at `build/Armory.xcodeproj`
# Build
```

**wasm**
```bash
./make --graphics webgpu --target wasm
```
