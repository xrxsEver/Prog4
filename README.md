# Pengo

A remake of the 1982 arcade game **Pengo**, built on a small engine I wrote myself on top of Minigin
[Minigin](https://github.com/avadae/minigin) and
[SDL3](https://www.libsdl.org/). It's a DAE *Programming 4* exam project: the point isn't just the
game, it's building the engine with the patterns from Robert Nystrom's
*[Game Programming Patterns](https://gameprogrammingpatterns.com/)*.

Three levels loaded from data, three modes (single-player, co-op, versus), full controller support and
a navigable menu.

## Design

The codebase is split hard down the middle:

- **`Engine/`** — reusable framework. The game loop, object/component model, input, rendering, scenes,
  audio, events. It knows nothing about Pengo.
- **`Game/`** — everything Pengo-specific: the maze, ice blocks, Sno-Bees, the HUD, game modes.

Keeping game concepts out of the engine is what makes the engine an engine and not just "the game in two
folders." Everything lives in the `dae` namespace.

## Patterns

These are the design / game-programming patterns the engine is built on, and where to look for each:

- **Game Loop** — `Engine/Core/Minigin.cpp`. A fixed-timestep accumulator: deterministic `FixedUpdate()`
  steps for logic/physics, then one variable-rate `Update()`, then render. Delta time is clamped so a
  hitch can't blow up the simulation. The same loop drives both the desktop build and the web
  (Emscripten) build.
- **Update Method** — every `GameObject` and `Component` gets `Update(deltaTime)` each frame, so each
  entity advances its own behaviour (animation, movement, timers) without the loop knowing the details.
- **Command** — `Engine/Core/Command.h`. Input is bound to command objects, never handled inline. Game
  commands live in `Game/Commands/` (Move, Push, AddScore, LoseLife) and are bound through the
  `InputManager`. The same command works from keyboard or gamepad, which is what makes rebinding and
  two-player input trivial.
- **Observer** — `Engine/Events/` (`Subject` / `Observer` / `Event`). The score and lives HUD *observe*
  the player instead of polling it: when the player scores or dies, it notifies, and the display reacts.
- **Component** — `Engine/Core/GameObject`. Objects are composed of components rather than built from an
  inheritance tree. A `GameObject` owns its components and a parent/child transform hierarchy; behaviour
  is added by attaching components (render, text, the player character, the maze, HUD readouts).
- **State** — `Game/Characters/PengoState.{h,cpp}`. Pengo is a small state machine — Idle / Moving /
  Pushing / Dying — where each state handles input and returns the next state. The death flow lives
  entirely in the Dying state instead of being scattered through `if` checks.

A few more in use: **Service Locator** for the global sound system and collision grid
(`Engine/Audio/ServiceLocator.h`), and **Singleton** for the handful of true engine-wide managers
(`Renderer`, `SceneManager`, `ResourceManager`).

## Build & run

**Windows** — open the root folder in Visual Studio 2026 or VS Code (both recognise it as a CMake
project), or configure from the command line:

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

The executable looks for assets in `./Data/`; CMake copies `Data/` (and the SDL DLLs) next to the exe
as a post-build step.

**Web (Emscripten)** — with emsdk active:

```
mkdir build_web && cd build_web
emcmake cmake ..
emmake ninja
python -m http.server   # then open http://localhost:8000
```
