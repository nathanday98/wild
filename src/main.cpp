#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <stdio.h>
#include <string.h>
#include <engine/std.h>
#include <engine/timer.cpp>
#include <stdlib.h>
#include <engine/math.cpp>
#include <string>
#include <game/game.h>
#include <core/platform.h>
#include <imgui/imgui.h>
#include <engine/input.cpp>
#include <core/render_context.h>
#include <core/d3d_render_context.cpp>
#include <core/imgui_impl.cpp>
#define STB_IMAGE_IMPLEMENTATION
#include <core/stb_image.h>
#include <game/assets.cpp>
#include <game/memory.h>

struct GameCode {
	void *game_code_dll;
	GameInitFunc *init;
	GameUpdateFunc *update;
	GameRenderFunc *render;
	FileTime last_write_time;
	bool is_valid;
};

internal_func GameCode loadGameCode(const char *source_dll_name, const char *temp_dll_name) {
	GameCode result = {};
	Platform::copyFile((char *)source_dll_name, (char *)temp_dll_name);
	result.last_write_time = Platform::getLastWriteTime((char *)source_dll_name);
	result.game_code_dll = SDL_LoadObject(temp_dll_name);
	
	if(result.game_code_dll) {
		result.init = (GameInitFunc *)SDL_LoadFunction(result.game_code_dll, "gameInit");
		result.update = (GameUpdateFunc *)SDL_LoadFunction(result.game_code_dll, "gameUpdate");
		result.render = (GameRenderFunc *)SDL_LoadFunction(result.game_code_dll, "gameRender");
		
		result.is_valid = (result.init != 0 && result.update != 0 && result.render != 0);
	}
	
	if(!result.is_valid) {
		result.init = gameInitStub;
		result.update = gameUpdateStub;
		result.render = gameRenderStub;	
	}
	
	return result;
}

internal_func void unloadGameCode(GameCode *game_code) {
	if(game_code->game_code_dll) {
		SDL_UnloadObject(game_code->game_code_dll);
		game_code->game_code_dll = 0;
	}
	
	game_code->is_valid = false;
	game_code->init = gameInitStub;
	game_code->update = gameUpdateStub;
	game_code->render = gameRenderStub;	
}

int main(int arg_count, char *args[]) {
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) { 
		printf("Couldn't initialize SDL\n");
	}
	
	char *base_path = SDL_GetBasePath();
	std::string game_dll_name = std::string(base_path) + "game.dll";
	std::string temp_game_dll_name = std::string(base_path) + "game_temp.dll";
	
	u32 window_width = 1600;
	u32 window_height = 900;
	SDL_Window *window = SDL_CreateWindow("Dungeon crawler thing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_RESIZABLE);
	if(!window) {
		error("Couldn't create window");
	}
	s32 refresh_rate = 60;
	RenderContext render_context;
	render_context.init(window_width, window_height, refresh_rate, window);
	
	ImGuiImpl::init(window);
	
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = 0;
	ImGuiStyle &style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	
	Timer frame_timer = Timer();
	bool running = true;
	
	MemoryStore mem_store = {};
	mem_store.game_memory = {0, Megabytes(8)};
	mem_store.asset_memory = {0, Megabytes(8)};
	mem_store.frame_memory = {0, Megabytes(2)};
	
	u64 mem_total_size = 0;
	for(int i = 0; i < MEMORY_STORE_COUNT; i++) {
		mem_total_size += mem_store.blocks[i].size;
	}
	
	mem_store.memory = (void *)SDL_malloc(mem_total_size);
	memset(mem_store.memory, 0, mem_total_size); // NOTE(nathan): this might need removing for performance??
	u8 *byte_walker = (u8 *)mem_store.memory;
	for(int i = 0; i < MEMORY_STORE_COUNT; i++) {
		mem_store.blocks[i].memory = (void *)byte_walker;
		byte_walker += mem_store.blocks[i].size;
	}
	
	
	
	
	f32 target_seconds_per_frame = 1.0f / (f32)refresh_rate;
	f32 delta = target_seconds_per_frame;

	Assets *game_assets = (Assets *)mem_store.asset_memory.memory;

	{
		int tex_w, tex_h, tex_comps;
		u8 *tex_data = stbi_load("data/textures/map.png", &tex_w, &tex_h, &tex_comps, 0);
		
		game_assets->map = render_context.createTexture2D(tex_data, (u32)tex_w,  (u32)tex_h, RenderContext::Format::u32_unorm);
		stbi_image_free(tex_data);
	}
	
	{
		int tex_w, tex_h, tex_comps;
		u8 *tex_data = stbi_load("data/textures/tileset.png", &tex_w, &tex_h, &tex_comps, 0);
		
		game_assets->tileset = render_context.createTexture2D(tex_data, (u32)tex_w,  (u32)tex_h, RenderContext::Format::u32_unorm);
		stbi_image_free(tex_data);
	}
	
	{
		int tex_w, tex_h, tex_comps;
		u8 *tex_data = stbi_load("data/textures/player.png", &tex_w, &tex_h, &tex_comps, 0);
		
		game_assets->player_texture = render_context.createTexture2D(tex_data, (u32)tex_w,  (u32)tex_h, RenderContext::Format::u32_unorm);
		stbi_image_free(tex_data);
	}

	const char *level_files[] = {
		"test",
		"hub"
	};
	
	game_assets->levels = level_files;
	game_assets->max_levels = ArrayCount(level_files);
	
		
	GameCode game_code = loadGameCode(game_dll_name.c_str(), temp_game_dll_name.c_str());
	game_code.init(&mem_store, &render_context, game_assets);
	
	InputManager input(window);
	// SDL_RenderSetLogicalSize(renderer, 400, 225);
	// SDL_RenderSetScale(renderer,4 ,4);
	while(running) {
		frame_timer.start();
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			ImGuiImpl::processEvent(&e);
			switch(e.type) {
				case SDL_QUIT: running = false; break;
				case SDL_CONTROLLERDEVICEREMOVED: {
					printf("Controller removed\n");
				} break;
				case SDL_CONTROLLERDEVICEADDED: {
					printf("Controller Added\n");
					input.connectController();
				} break;
				case SDL_KEYDOWN: {
					switch(e.key.keysym.scancode) {
						case SDL_SCANCODE_ESCAPE: running = false; break;
					}
				} break;
				
				case SDL_WINDOWEVENT: {
					switch(e.window.event) {
						case SDL_WINDOWEVENT_SIZE_CHANGED: {
							s32 new_width = e.window.data1;
							s32 new_height = e.window.data2;
							render_context.resizeBuffer(new_width, new_height);
						} break;
					}
				} break;
			}
		}
		
		ImGuiImpl::newFrame(window, &render_context);
		
		FileTime new_dll_write_time = Platform::getLastWriteTime((char *)game_dll_name.c_str());
		if(Platform::compareFileTime(&new_dll_write_time, &game_code.last_write_time) != 0) {
			unloadGameCode(&game_code);
			printf("Reloading game code\n");
			game_code = loadGameCode(game_dll_name.c_str(), temp_game_dll_name.c_str());
		}
		
		input.processKeys();
		f32 debug_window_height = 0.0f;
		ImGui::SetNextWindowPos(ImVec2(5, 5));
		ImGui::Begin("Debug Stuff", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize); {
			ImGui::Text("Frame time: %.3f", delta * 1000.0f);
			ImGui::Text("Frame size: %d, %d", window_width, window_height);
			debug_window_height = ImGui::GetWindowHeight();
			ImGui::End();
		}
		
		if(input.isKeyPressedOnce(Button::Fullscreen)) {
			u32 fullscreen_mode = SDL_WINDOW_FULLSCREEN_DESKTOP;
			if(ContainsBits(SDL_GetWindowFlags(window), fullscreen_mode))  {
				SDL_SetWindowFullscreen(window, 0);
			} else {
				SDL_SetWindowFullscreen(window, fullscreen_mode);
			}
		}
		
		game_code.update(&mem_store, &input, delta, window, game_assets);
		f32 r = 51.0f / 255.0f;
		render_context.clear(Vec4(r, r, r, 1.0f).xyzw);
		
		game_code.render(&mem_store, window, &render_context, &input, game_assets);
		
		ImGui::Render();
		ImGuiImpl::render(&render_context);
		
		render_context.present();
		
		
		
		u64 work_counter = frame_timer.getWallClock();
		f32 work_seconds_elapsed = frame_timer.getSecondsElapsed();
		
		f32 seconds_elapsed_for_frame = work_seconds_elapsed;
		if(seconds_elapsed_for_frame < target_seconds_per_frame)
		{
			u32 sleep_ms = (u32)((1000.0f * (target_seconds_per_frame - seconds_elapsed_for_frame)) - 1);
			if(sleep_ms > 0)
				SDL_Delay(sleep_ms);
			while(seconds_elapsed_for_frame < target_seconds_per_frame)
			{
				seconds_elapsed_for_frame = frame_timer.getSecondsElapsed();
			}
		}
		else
		{
			// missed frame
			printf("Missed frame\n");
		}
		u64 end_counter = frame_timer.getWallClock();
		delta = frame_timer.getSecondsElapsed();
		SDL_SetWindowTitle(window, formatString("%.3fms/frame", delta * 1000.0f));
		input.endFrame();
	}
	
	unloadGameCode(&game_code);
	ImGuiImpl::shutdown();
	render_context.uninit();
	SDL_DestroyWindow(window);
	
	SDL_Quit();
	return 0;
}