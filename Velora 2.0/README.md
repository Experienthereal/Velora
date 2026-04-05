# Velora Programming Language

**C++ speed. Python simplicity. Built-in game engine.**

Velora is a compiled programming language designed for game development and general-purpose coding. It compiles to native machine code through C, delivering C++-level performance with Python-like ease of use.

---

## Quick Start

```vel
main:
    print("Hello from Velora!")
```

```vel
func fibonacci(n: int) -> int:
    if n <= 1:
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)

main:
    for i in 0..10:
        print(f"fib({i}) = {fibonacci(i)}")
```

## Features

| Feature | Velora |
|---------|--------|
| Syntax | Python-like (indentation-based) |
| Speed | C++-level (compiles to native) |
| Memory | Automatic Reference Counting |
| Types | Strong static with inference |
| Variables | `:=` infer, `::` constant |
| Game Engine | Built-in Veloragame |
| Graphics | DirectX / Vulkan / OpenGL |

## Install

### From the Installer (Recommended)

1. Download `VeloraSetup.exe`
2. Double-click it — it asks for admin rights
3. Click OK to install
4. Open any terminal and type: `velora version`

**What the installer does:**
- Copies Velora to `C:\Program Files\Velora\`
- Adds `velora` to the system PATH
- Registers `.vel` files with Windows (right-click → Run/Build/Check)
- Adds a system tray icon
- Starts automatically with Windows

### From Source

Requires [MinGW-w64](https://github.com/brechtsanders/winlibs_mingw) with GCC 12+.

```batch
:: Build just the compiler
build.bat

:: Build the single-file installer EXE
build_installer.bat
```

## Usage

```batch
velora run main.vel         # Compile and run
velora build main.vel       # Compile to .exe
velora check main.vel       # Syntax check only
velora tokens main.vel      # Show token stream
velora new my_project       # Create new project
velora version              # Show version
velora help                 # Show help
```

## Veloragame Example

```vel
Veloragame(backend = Auto, vsync = true):

main:
    window := create Window("My Game", 1280, 720)
    renderer := window.getRenderer()
    player_x := 400.0

    game_loop:
        while window.isOpen():
            if keyboard.isPressed("D"):
                player_x += 5.0
            if keyboard.isPressed("A"):
                player_x -= 5.0

            renderer.clear(#1a1a2e)
            renderer.draw_rect(player_x, 500, 40, 60, color = #4ecdc4)
            renderer.present()
```

## Project Structure

```
compiler/       Velora compiler (vlc) — Lexer, Parser, Analyzer, CodeGen
stdlib/         Standard library — math, io, fs, time, log
veloragame/     Built-in game engine — window, renderer, physics, UI, sound
examples/       Example .vel programs
installer/      Windows installer source
```

## License

MIT License — see [LICENSE](LICENSE)
