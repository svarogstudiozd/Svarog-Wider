# Svarog Wider

Stereo widening plug-in built with [JUCE 8](https://juce.com).

## Downloading builds

GitHub Actions builds the plug-ins automatically:

- Every push to `main` / every pull request: open the **Actions** tab, pick the
  latest *Build Plugins* run and download the `Svarog-Wider-macOS` and
  `Svarog-Wider-Windows` artifacts.
- Pushing a tag (e.g. `git tag v0.1.0 && git push origin v0.1.0`): the zips are
  attached to a GitHub **Release**.

| Platform | Formats |
| -------- | ------- |
| macOS | AU + VST3, universal (Apple Silicon + Intel), ad-hoc signed |
| Windows | VST3, 64-bit |

### Installing

| Platform | Destination |
| -------- | ----------- |
| macOS AU | `~/Library/Audio/Plug-Ins/Components` |
| macOS VST3 | `~/Library/Audio/Plug-Ins/VST3` |
| Windows VST3 | `C:\Program Files\Common Files\VST3` |

The macOS builds are not notarized. If macOS refuses to load them after
downloading, remove the quarantine flag:

```sh
xattr -dr com.apple.quarantine "/path/to/Svarog Wider.vst3"
xattr -dr com.apple.quarantine "/path/to/Svarog Wider.component"
```

## Building locally

JUCE is vendored in `lib/JUCE`, so no extra dependencies are needed besides
CMake 3.22+ and a C++17 compiler (Xcode on macOS, Visual Studio on Windows).

```sh
# macOS: AU + VST3 + Standalone
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel

# macOS universal binary (arm64 + x86_64)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"

# Windows (Visual Studio generator)
cmake -B build -A x64
cmake --build build --config Release --parallel
```

Build output lands in `build/src/SvarogWider_artefacts/Release/`.

By default the plug-ins are also copied into your system plug-in folders after
building. Pass `-DSVAROG_WIDER_COPY_PLUGINS_AFTER_BUILD=OFF` to disable that
(this is what CI does, so the build tree stays self-contained).
