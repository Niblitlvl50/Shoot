# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

The project uses CMake with Visual Studio 2022 on Windows. The `build/` directory is already configured.

**Configure (first time or after CMakeLists changes):**
```
cd build
cmake ../ -DCMAKE_BUILD_TYPE=Debug
```

**Build all targets:**
```
cmake --build build -j8
```

**Build a specific target:**
```
cmake --build build --target game_exe -j8
cmake --build build --target editor -j8
cmake --build build --target game_test_exe -j8
cmake --build build --target unittest -j8
```

**Run tests:**
```
./bin/game_test_exe.exe     # Game-level tests (src/tests/)
./bin/unittest.exe           # Engine-level tests (Mono1/tests/)
```

Run a single GTest: append `--gtest_filter=TestSuiteName.TestName`.

**Run binaries** (from repo root — working directory matters for resource loading):
```
./bin/<Config>/game_exe.exe
./bin/<Config>/editor.exe
./bin/<Config>/animator.exe res/sprites/alien.sprite
```

Outputs go to `bin/<Config>/` where `<Config>` is `Debug` or `Release` depending on the CMake build type. No separate install step is needed.

## Scripts

All scripts are in `scripts/` and require Python 3.

**Bake sprite atlas** (run after adding/changing images in `res/images/`):
```
python scripts/bake_sprites.py
```
Packs all PNGs from `res/images/` into `res/sprite_atlas.png` (2048×1024) and regenerates `.sprite` files in `res/sprites/`. Requires `bin/Debug/spritebaker` to be built.

**Embed sprites as C headers** (run after baking if sprites are embedded in code):
```
python scripts/embed_all_sprites.py
```
Walks `res/sprites/` and generates a `.h` file next to each `.sprite` file containing the data as a `constexpr const char*`.

`generate_icons_file.py` is macOS-only (uses `sips`/`iconutil`) for generating `.icns` app icons.

## Repository Structure

```
Mono1/          Custom engine (submodule) — the "mono" static library
  src/          Engine source; included as "Mono1/src" and "Mono1/third_party"
  tests/        Engine unit tests (GTest)
  third_party/  SDL2, Chipmunk2D, imgui, gtest, etc.
src/
  Game/         Game logic — compiled into game_lib static library
  Editor/       Level editor executable (links game_lib + mono)
  Animator/     Sprite animator executable (links mono)
  tests/        Game unit tests (GTest, links game_lib + mono)
  SpriteBaker/  Standalone sprite atlas baking tool
res/            Runtime assets: worlds, entities, sprites, sounds, fonts, configs
build/          CMake build tree (Visual Studio 2022 solution: shoot.sln)
```

## Architecture

### Engine Layer (Mono1)

`mono::Engine` owns the main loop and runs one `mono::IZone` at a time by calling `Accept(IUpdater)` and `Accept(IRenderer)` each frame.

**SystemContext** (`Mono1/src/SystemContext.h`) is the service locator for all game systems. Systems are registered with `CreateSystem<T>(args...)` and retrieved with `GetSystem<T>()`. Each frame, SystemContext calls `Update()` → `PostUpdate()` → `Sync()` on every registered system in creation order.

**IGameSystem** (`Mono1/src/IGameSystem.h`) is the interface all systems implement. Lifecycle: `Begin()` → per-frame `Update()`/`PostUpdate()`/`Sync()` → `Reset()` on zone unload → `Destroy()` before destructor.

**Core engine systems** (all in `Mono1/src/`):
- `EntitySystem` — entity/component management; loads entities from `.entity` files via `SpawnEntity()`
- `TransformSystem` — position/rotation (matrix) and bounding box for every entity, indexed by entity ID
- `PhysicsSystem` — Chipmunk2D wrapper; syncs physics body positions back to TransformSystem each frame
- `TriggerSystem` — event bus keyed by `hash::Hash("trigger_name")`; supports shape, area, time, counter, relay, and animation-notify triggers
- `SpriteSystem`, `TextSystem`, `ParticleSystem`, `RenderSystem`, `LightSystem` — rendering
- `EventHandler` — type-safe pub/sub for input and application events (keyboard, mouse, controller)

### Game Layer (src/Game/)

`game::CreateGameSystems()` (`src/Game/GameSystems.h`) wires all game-specific systems into the SystemContext before any zone loads.

**GameZone** (`src/Game/Zones/GameZone.h`) is the main scene. It reads a `.world` file (via `ReadWorldComponentObjects()`), spawns all entities, and creates an `IGameMode`.

**IGameMode** (`src/Game/GameMode/`) — `Begin()`/`End()` hooks for mode-specific logic. Modes are constructed by hash in `GameModeFactory`:
- `"start_menu_mode"` → `StartMenuGameMode`
- `"horde_mode"` → `HordeGameMode`
- `"package_delivery_mode"` → `PacketDeliveryGameMode`

**EntityLogicSystem** (`src/Game/Entity/EntityLogicSystem.h`) owns all `IEntityLogic` instances. Each enemy or interactive object registers a logic object (created by `EntityLogicType` enum → controller class mapping).

**IEntityLogic** (`src/Game/Entity/IEntityLogic.h`) is the base for all per-entity behaviours — enemy AI controllers, player logic, reactive props. Subclasses implement `Update()` and optionally `Draw()`. Enemy controllers (in `src/Game/Enemies/`) use the template `StateMachine<StateId, UpdateContext>` (`src/Game/StateMachine.h`) for state-driven AI.

### Component System

Components are identified by hashed names (constants in `src/Game/Entity/Component.h`, e.g. `TRANSFORM_COMPONENT`, `SPRITE_COMPONENT`, `HEALTH_COMPONENT`). Each component type registers create/release/update/enable function pointers with `IEntityManager::RegisterComponent()` — implementations live in `src/Game/Entity/ComponentFunctions.cpp` and `GameComponentFuncs.cpp`.

Entity data is stored in fixed-size pools per system. `mono::ActiveVector<T>` (`Mono1/src/Util/ActiveVector.h`) provides sparse indexed storage with O(1) activation checks.

### Key Conventions

**Hashing** — `hash::Hash(const char*)` (FNV-1a, `Mono1/src/System/Hash.h`) is used everywhere: component IDs, trigger names, game mode names, attribute keys. The same string always produces the same `uint32_t`. Register debug strings with `hash::HashRegisterString()` so they appear in logs.

**Entity IDs** — `mono::INVALID_ID = (uint32_t)-1`. Entity IDs are dense indices into pre-allocated arrays; systems assume IDs never exceed the capacity passed at creation.

**Collision categories** — defined in `src/Game/CollisionConfiguration.h` as bitmasks: `PLAYER`, `PLAYER_BULLET`, `ENEMY`, `ENEMY_BULLET`, `PROPS`, `PICKUPS`, `STATIC`, `PACKAGE`, `PLAYER_ONLY`. Each category has a corresponding mask controlling what it collides with.

**Render layers** — `game::LayerId` (`src/Game/RenderLayers.h`): `BACKGROUND` through `GAMEOBJECTS_UI` are affected by the lighting pass; `UI`, `UI_OVERLAY`, `UI_DEBUG` are not.

**Navigation** — `game::NavigationSystem` builds a navmesh from physics geometry and exposes `FindPath(start, end)` returning an A* path. Enemy controllers use `PathBehaviour` (`src/Game/Behaviour/PathBehaviour.h`) to follow paths using physics body velocity.
