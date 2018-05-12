A Simple 2D engine inspired by the Ubi-Art Framework (in active development)

Features <br />
* Basic abstraction over Directx11 (core/d3d_render_context.cpp)
* Custom ImGui implementation (core/imgui_impl.cpp)
* Hot-reloading of game code (main.cpp)
* Analytical collision solver (game/scene.cpp)
* Circle swept collision solving (game/scene.cpp)
  * Line segments <br />
  * AABBs <br />
  * Circles <br />
  * Cubic Bezier curves <br />
* 'Collide and slide' algorithm implementation (game/game.cpp)
* Saving and loading of geometry (game/scene.cpp and game/game.cpp)
* Various debug rendering primitives (engine/debug_renderer.cpp)
* In-game editor (press f4 to toggle)
* Dynamic light volume generation (game/game.cpp)
* Custom allocators (engine/allocators.cpp and game/memory.h)
