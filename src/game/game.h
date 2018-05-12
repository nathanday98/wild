#ifndef GAME_H
#define GAME_H
#include <engine/std.h>
#include <SDL2/SDL.h>
#include <core/render_context.h>
#include <game/memory.h>

class InputManager;
struct Assets;

#define GAME_INIT(name) void name(MemoryStore *mem_store, RenderContext *render_context, Assets *assets)
typedef GAME_INIT(GameInitFunc);

GAME_INIT(gameInitStub) {
	
}


#define GAME_UPDATE(name) void name(MemoryStore *mem_store, InputManager *input, f32 delta, SDL_Window *window, Assets *assets)
typedef GAME_UPDATE(GameUpdateFunc);

GAME_UPDATE(gameUpdateStub) {
	
}


#define GAME_RENDER(name) void name(MemoryStore *mem_store, SDL_Window *window, RenderContext *render_context, InputManager *input, Assets *assets)
typedef GAME_RENDER(GameRenderFunc);

GAME_RENDER(gameRenderStub) {
	
}

#endif // GAME_H