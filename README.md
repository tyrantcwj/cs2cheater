# iTyc-cs2

CS2 外部 overlay 学习项目。预编译包请从 [Releases](https://github.com/tyrantcwj/cs2cheater/releases) 下载。

## 使用

1. 先启动 Counter-Strike 2
2. 运行 `iTyc-cs2.exe`
3. 内存后端建议先选 `1 (WinAPI)`；模式 3 需要管理员权限
4. 快捷键：小键盘 `1` 菜单 / `2` 总开关 / `3` 退出

## 编译

需要 Visual Studio 2022/2026 Build Tools（C++ 桌面开发）与 CMake 3.20+。

```bat
cd cs2ext
cmake -B build -G "Visual Studio 18 2026" -A x64
cmake --build build --config Release
```

产物：`cs2ext/build/Release/iTyc-cs2.exe`
