# armorcore

3D engine core for C with embedded V8. ArmorCore is designed for the [Graphics5](https://github.com/Kode/Kinc/tree/master/Backends/Graphics5) api and targets Direct3D12, Vulkan, Metal and WebGPU. *(wip!)*

Based on [Krom](https://github.com/Kode/Krom). Powered by [Kinc](https://github.com/Kode/Kinc).

```bash
git clone --recursive https://github.com/armory3d/armorcore
cd armorcore
```

**Windows**
```bash
# Unpack `v8\libraries\win32\release\v8_monolith.7z` using 7-Zip - Extract Here (exceeds 100MB)
Kinc/make -g direct3d11
# Open generated Visual Studio project at `build\Krom.sln`
# Build for x64 & release
```

**Linux**
```bash
Kinc/make -g opengl --compiler clang --compile
cd Deployment
strip Krom
```

**macOS**
```bash
Kinc/make -g metal
# Open generated Xcode project at `build/Krom.xcodeproj`
# Build
```

**Android** *wip*
```bash
Kinc/make -g opengl android
# Manual tweaking is required for now:
# https://github.com/armory3d/armorcore/blob/master/kfile.js#L136
# Open generated Android Studio project at `build/Krom`
# Build for device
```

**iOS** *wip*
```bash
Kinc/make -g metal ios
# Open generated Xcode project at `build/Krom.xcodeproj`
# Build for device
```

**Windows DXR** *wip*
```bash
# Unpack `v8\libraries\win32\release\v8_monolith.7z` using 7-Zip - Extract Here (exceeds 100MB)
Kinc/make -g direct3d12
# Open generated Visual Studio project at `build\Krom.sln`
# Build for x64 & release
```

**Linux VKRT** *wip*
```bash
Kinc/make -g vulkan --compiler clang --compile
cd Deployment
strip Krom
```

**Windows VR** *wip*
```bash
# Unpack `v8\libraries\win32\release\v8_monolith.7z` using 7-Zip - Extract Here (exceeds 100MB)
Kinc/make -g direct3d11 --vr oculus
# Open generated Visual Studio project at `build\Krom.sln`
# Build for x64 & release
```

**Generating a v8 snapshot file**
```bash
./Krom . --snapshot
# Generates a `krom.bin` file from `krom.js` file
```
