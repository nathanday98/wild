#include <game/game.h>
#include <engine/std.h>
#include <stdlib.h>
#include <stdio.h>
#include <engine/math.cpp>
#include <imgui/imgui.h>
#include <engine/input.cpp>
#include <float.h>
#include <string.h>
#include <vector>
#include <engine/allocators.cpp>

global_variable StackAllocator g_frame_stack;

#include <game/scene.cpp>
#include <engine/debug_renderer.cpp>
#include <game/fastnoise.h>
#include <game/fastnoise.cpp>
#include <time.h>

#include <game/assets.cpp>

#define CHUNK_W 32
#define CHUNK_H 32

struct Camera {
	Vec2 position;
	Vec2 half_size;
	Vec2i window_dimensions;
	Vec2 world_mouse_pos;
	Vec2 world_mouse_prev_pos;
	
	Mat4 getVP() {
		// return Mat4::perspective(60.0f, (f32)window_dimensions.x / (f32)window_dimensions.y, 0.01f, 1000.0f) * Mat4::translate(Vec3(-position, 0.0f));
		
		return 	Mat4::ortho(half_size.x * 2.0f, half_size.y * 2.0f, -1.0f, 1.0f) * Mat4::translate(Vec3(-position, 0.0f));
	}
	
	Vec2 screenToWorld(Vec2i screen) {
		Vec2 wd = v2iToV2(window_dimensions);
		Vec2 s  = v2iToV2(screen);
		Vec2 unit = s / wd;
		Vec2 result = (unit * half_size * 2.0f) - half_size;
		return Vec2(result.x, -result.y) + position;
	}
	
	void beginFrame(SDL_Window *window, InputManager *input) {
		int window_width, window_height;
		SDL_GetWindowSize(window, &window_width, &window_height);
		window_dimensions = Vec2i(window_width, window_height);
		world_mouse_pos = screenToWorld(input->getMousePosition());
	}
	
	void endFrame() {
		world_mouse_prev_pos = world_mouse_pos;
	}
	
	Vec2 getWorldMouseDelta() { return world_mouse_pos - world_mouse_prev_pos; }
};

struct Bullet {
	Vec2 position;
	Vec2 direction;	
	f32 timer;
};

struct Zombie {
	Vec2 position;
	Vec2 direction;	
	u32 scene_index;
};

struct Light {
	Vec2 position;
	f32 length;
	f32 angle_spread;
	f32 angle_start;	
	s32 ray_count;
};

#define BULLET_POOL_MAX 256
#define ZOMBIE_POOL_MAX 256

struct GameState {
	Vec2 player_pos;
	Vec2 player_end;
	Vec2 player_vel;
	DebugRenderer debug_renderer;
	f32 timer;
	Scene scene;
	bool jumping;
	bool grounded;
	f32 gravity_vel;
	Vec2 reticule_pos;
	Vec2 player_normal;
	Vec2 player_ret;
	Vec2 target_normal;
	std::vector<Light> lights;
	Camera camera;
	f32 fire_timer;
	u8 bullet_masks[BULLET_POOL_MAX];
	Bullet bullet_pool[BULLET_POOL_MAX];
	Vec2 world_mouse_pos;
	Vec2 world_mouse_prev_pos;
	int frame_index;
	f32 frame_timer;

	Vec2 worldCollider(Vec2 pos, f32 radius, Vec2 vel, bool &hit, Vec2 &normal, bool stick = false) {
		Vec2 dest = pos + vel;
		Plane first_plane = {};
		hit = false;
		if(!stick) {
			for(int i = 0; i < 3; i++) {
				CollisionStats stats = scene.sweptQuery(pos, radius, vel);		
				if(stats.hit) {
					normal = stats.sliding_plane.normal;
					hit = true;
				}
				
				if(!stats.hit) return dest;
				pos = stats.near_point;
				if(i == 0) {
					f32 long_radius = radius + EPSILON;
					first_plane = stats.sliding_plane;
					dest -= (first_plane.distance(dest) - long_radius) * first_plane.normal;
					vel = dest - pos;
				} else if (i == 1) {
					Plane second_plane = stats.sliding_plane;
					Vec2 crease = Vec2::normalize(first_plane.normal + second_plane.normal);
					f32 dis = Vec2::dot(dest - pos, crease);
					vel = dis * crease;
					dest = pos + vel;
					// Vec2 crease = Vec2::cross()
				}
			}
		} else {
			CollisionStats stats = scene.sweptQuery(pos, radius, vel);		
			if(stats.hit) {
				normal = stats.sliding_plane.normal;
				hit = true;
			}
			
			if(!stats.hit) return dest;
			pos = stats.near_point;
		}
		
		return pos;	
	}

	f32 calcGravity(f32 max_jump_height, f32 time_to_jump_apex, f32 &jump_velocity) {
		f32 gravity = -(2 * max_jump_height) / Math::pow(time_to_jump_apex, 2);
		jump_velocity = Math::abs(gravity) * time_to_jump_apex;
		return gravity;
	}

	void spawnPlayer() {
		player_vel = Vec2();
		player_pos = scene.player_start;
		player_ret = Vec2(1.0f, 0.0f);
	}
	
	void loadLevel(Assets *assets, int asset_level_index) {
		std::string path = "data/levels/" + std::string(assets->levels[asset_level_index]) + ".lvl";
		FileData fd = Platform::readEntireFile(path.c_str());
		scene.loadFromFile(&fd);
		spawnPlayer();
		camera.position = player_pos;
	}
	
	void init(RenderContext *render_context, Assets *assets) {
		camera.half_size = Vec2(32.0f, 18.0f);
		frame_index = 0;
		
	    debug_renderer.init(render_context);
	    timer = 0.0f;
	    
	    loadLevel(assets, 1);
	    	    
	    grounded = false;
	    jumping = false;
	    gravity_vel = 0.0f;
	    fire_timer = 0.0f;
	    
	}
	
	void fireBullet(Vec2 position, Vec2 direction) {
		for(u32 i = 0; i < BULLET_POOL_MAX; i++) {
			if(bullet_masks[i] == 0) {
				bullet_masks[i] = 1;
				f32 r = Math::randFloat() * 2.0f - 1.0f;
				f32 d = Math::randFloat() * -2.0f + 1.0f;
				direction.x += r * 0.1f;
				direction.y += d * 0.1f;
				direction = Vec2::normalize(direction);
				bullet_pool[i] = {position, direction, 0.0f};
				return;
			}
		}
	}
	
	void update(InputManager *input, f32 delta, SDL_Window *window) {
		camera.beginFrame(window, input);
		
		f32 move_speed = 3.0f;
		player_vel += move_speed * input->getLeftAxis();
		
		
		f32 max_jump_height = 6.0f;
		f32 time_to_jump_apex = 0.25f;
		f32 jump_velocity = 0.0f;
		f32 gravity = calcGravity(max_jump_height, time_to_jump_apex, jump_velocity);	
		timer += delta;	
		frame_timer += delta;
		
		if(input->getJump() && grounded) {
			jumping = true;
			grounded = false;
			player_vel.y += jump_velocity;
		} 
		
		if(input->getDive()) {
			// gravity *= 4.0f;
		}
		
		f32 gmod = 1.0f;
		
		if(frame_timer >= 0.1f) {
			frame_timer = 0.0f;
			frame_index++;
		}
		if(frame_index > 8) frame_index = 0;
		
		// player_vel.y += gravity * gmod * delta;
		
		Vec2 right_axis = input->getRightAxis();
		if(right_axis.x > 0.5f || right_axis.x < -0.5f) {
			player_ret = Vec2::normalize(Vec2(right_axis.x, 0.0f));
		}
		// Vec2 ret_vec = Vec2::normalize(camera.world_mouse_pos - player_pos);
		
		Vec2 p_right = Vec2::rPerp(player_normal);
		
		reticule_pos = player_pos + Vec2::normalize(p_right * player_ret.x * 1.f);
		
		fire_timer -= delta;
		if(fire_timer <= 0.0f) fire_timer = 0.0f;
		if(input->getFire() && fire_timer <= 0.0f) {
			printf("Firing bullet\n");
			// fireBullet(reticule_pos, player_ret);
			player_vel.x += -player_ret.x * 20.0f;
			fire_timer = 0.1f;
		}
		
		player_vel *= 0.76f;
		
		
		bool hit = false;
		player_pos = worldCollider(player_pos, 0.5f, player_vel * delta, hit, target_normal);
		
		// player_pos = worldCollider(player_pos, 0.5f, player_vel * delta, hit, target_normal);
		
		
		
		
		
		
		
		camera.position += (player_pos - camera.position) * 10.0f * delta;
		
		
		
		
		// debug_renderer.drawLine(previous_pos, light_pos, 0.025f, Vec4(0.0f, 1.0f, 0.0f, 1.0f));
		
		// for(int i = 0; i < 4; i++) {
		// 	bez_points[i] = useHandle(this, input, bez_points[i]);
		// }
		
		// for(int i = 0; i < 2; i++) {
		// 	line_points[i] = useHandle(input, line_points[i]);
		// }
	}
	
	void render(SDL_Window *window, RenderContext *render_context, InputManager *input, Assets *assets) {
		// camera.position = Vec2();
		render_context->bindShader(&debug_renderer.textured_shader);
		
		debug_renderer.fillTexturedRect(Vec2(), Vec2(assets->map.width * 0.1f, assets->map.height * 0.1f), &assets->map);
		
		debug_renderer.startGeom(camera.getVP());
		
		
		AABB collider = {Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f)};
		collider = collider;
		
		// debug_renderer.fillCircle(player_pos, 0.5f, Vec4(0.0f, 0.0f, 0.0f, 0.5f), 36);
		
		
		// debug_renderer.fillCircle(new_pos, circle_radius, Vec4(1.0f, 0.0f, 0.0f, 1.0f), 36);
		
		static f32 angle;
		
		debug_renderer.drawLine(Vec2(), Vec2::up() * 2.0f, 0.025f, Vec4(0.0f, 0.0f, 1.0f, 1.0f));
		debug_renderer.drawLine(Vec2(), Vec2::rotate(Vec2::up(), Math::toRadians(angle)) * 2.0f, 0.025f, Vec4(0.0f, 0.0f, 1.0f, 1.0f));
		
		for(int i = 0; i < scene.static_aabbs.size(); i++) debug_renderer.drawAABB(scene.static_aabbs[i]);
		for(int i = 0; i < scene.static_lines.size(); i++) debug_renderer.drawLine(scene.static_lines[i].start, scene.static_lines[i].end, 0.025f, Vec4(1.0f, 0.0f, 0.0f, 1.0f));
		
		for(int i = 0; i < scene.static_beziers.size(); i++) debug_renderer.drawCubBezier(scene.static_beziers[i].a, scene.static_beziers[i].b, scene.static_beziers[i].c, scene.static_beziers[i].d, 0.025f, Vec4(1.0f), scene.static_beziers[i].tesselation);
			
		for(int i = 0; i < scene.static_circles.size(); i++) debug_renderer.fillCircle(scene.static_circles[i].center, scene.static_circles[i].radius,Vec4(1.0f, 0.0f, 0.0f, 1.0f));
			
		
		Vec2 normal = Vec2(player_normal.x, Math::abs(player_normal.y));
		debug_renderer.drawLine(player_pos, reticule_pos, 0.1f, Vec4(0.0f, 1.0f, 0.0f, 1.0f));
		
		// debug_renderer.drawLine(player_pos, player_pos + normal, 0.025f, Vec4(0.0f, 0.0f, 1.0f, 1.0f));
		
		
		// debug_renderer.drawCubBezier(bez_points[0], bez_points[1], bez_points[2], bez_points[3]);
		
		// debug_renderer.fillCircle(world_mouse_pos, 0.1f);
		
		for(u32 i = 0; i < BULLET_POOL_MAX; i++) {
			u8 mask = bullet_masks[i];
			Bullet bullet = bullet_pool[i];
			if(mask == 1) {
				
				debug_renderer.drawLine(bullet.position, bullet.position + bullet.direction * 0.75f, 0.25f, Vec4(1.0f, 0.0f, 1.0f, 1.0f));
			}
		}
		
		
		
		
		
		// render_context->bindShader(&debug_renderer.light_shader);
		VertexStack vs(&g_frame_stack);
		for(int l = 0; l < lights.size(); l++) {
		
			Light &light = lights[l];
		
			int light_ray_count = light.ray_count;
			f32 angle_spread = light.angle_spread;
			f32 ray_length = light.length;
			f32 angle_diff = angle_spread / (f32)light_ray_count;
			Vec2 previous_pos = light.position;
			Vec2 start_dir = Vec2(0.0f, 1.0f);
			f32 start_angle = 180.0f;
			
			for(int i = 0; i <= light_ray_count; i++) {
				Vec2 dir = Vec2::normalize(Vec2(Math::cos(Math::toRadians(start_angle + (i * angle_diff))), Math::sin(Math::toRadians( start_angle + (i * angle_diff)))));
				
				f32 fraction = 1.0f;
				CollisionStats cs = scene.sweptQuery(light.position, 0.00001f, dir * ray_length);
				if(cs.hit) {
					
					fraction = cs.fraction;
				}
				Vec2 new_pos = light.position + dir * fraction * ray_length;
				debug_renderer.drawLine(light.position, new_pos);
				
				// if(cs.hit) {
				// 	Vec2 nn = cs.sliding_plane.normal;
				// 	debug_renderer.drawLine(cs.near_point, cs.near_point + nn);	
					
				// 	f32 theta = Vec2::angle(-dir, nn);
				// 	printf("%.3f\n", Math::toDegrees(theta));					
				// 	Vec2 bounce = Vec2::rotate(nn, Math::toRadians(45));
				// 	CollisionStats new_cs = scene.sweptQuery(new_pos, 0.00001f, bounce * ray_length);
				// 	f32 bounce_fraction = 1.0f;
				// 	if(new_cs.hit) {
				// 		bounce_fraction = new_cs.fraction;
				// 	}
					
					
				// 	debug_renderer.drawLine(new_pos, new_pos + bounce * ray_length);	
				// }
				
				
				// debug_renderer.drawLine(previous_pos, new_pos, 0.025f, Vec4(0.0f, 1.0f, 0.0f, 1.0f));
				
				vs.push(Vec3(light.position, 0.0f));
				vs.push(Vec3(new_pos, 0.0f));
				vs.push(Vec3(previous_pos, 0.0f));
								
				
				previous_pos = new_pos;
			}
			debug_renderer.drawPolyStrip(vs.getPointer(), vs.getCount(), Vec4(light.position, 1.0f), Mat4(), RenderContext::Topology::TriangleList);
			vs.clear();
		}
		
		debug_renderer.drawLine(player_pos, camera.world_mouse_pos);
		// debug_renderer.fillCircle(camera.world_mouse_pos, 1.0f);
		s32 seg_index = Math::floorToInt((22.5f + -Math::toDegrees(Vec2::angle(Vec2::up(), -Vec2::normalize(camera.world_mouse_pos - player_pos)))) / 45.0f);
		
		
		f32 d_scale = -Math::sign(seg_index);
		if(d_scale == 0) d_scale = 1;
		s32 anim_index = Math::abs(seg_index);
		
		// ImGui::Text("anim index %d", anim_index);
		// ImGui::Text("scale  %f", d_scale);
		
		f32 d_angle = 22.5f;
		for(int i = 0; i < 8; i++) {
			// debug_renderer.drawLine(player_pos, player_pos + Vec2::rotate(Vec2::up(), Math::toRadians(d_angle)) * 4.0f);
			d_angle += 45.0f;
		}
		
		debug_renderer.fillCircle(player_pos, 1.0f, Vec4(0.0f, 0.0f, 0.0f, 0.5f));
		
		render_context->bindShader(&debug_renderer.textured_shader);
		
		Vec2 p_size = Vec2(48.0f * 0.1f, 48.0f * 0.1f);
		f32 dx = 1.0f / assets->player_texture.width;
		f32 dy = 1.0f / assets->player_texture.height;
		
		
		Vec2 uv_offset = Vec2(dx * ((48.0f * Math::clamp(-d_scale, 0, 1)) + 48.0f * frame_index), dy * 48.0f * ((anim_index * 3) + 2));
		Vec2 uv_scale = Vec2(dx * d_scale * 48.0f, dy * 48.0f);
		debug_renderer.fillTexturedRect(player_pos + Vec2(0.0f, p_size.y*0.25f), p_size, &assets->player_texture, Vec4(1.0f), uv_offset, uv_scale, Mat4::scale(Vec3(1.0f, 1.0f, 1.0f)));
		
		
		
		// debug_renderer.fillCircle(scene.static_circles[0].closestPoint(player_pos), 0.5f, Vec4(0.0f, 0.0f, 0.0f, 1.0f), 36);
		camera.endFrame();
	}
};

#ifdef PP_EDITOR
#define EDITOR_ONLY(stuff) stuff
#else
#define EDITOR_ONLY(stuff)
#endif // PP_EDITOR

#include <game/editor.cpp>

GAME_INIT(gameInit) {
	
	g_frame_stack.initialize(mem_store->frame_memory.memory, mem_store->frame_memory.size);
	
	u64 memory_size = sizeof(GameState);
	EDITOR_ONLY(memory_size += sizeof(EditorState);)
	
	Assert(memory_size <= mem_store->game_memory.size);	
	GameState *game = (GameState *)mem_store->game_memory.memory;
	
	game->init(render_context, assets);
	
	EDITOR_ONLY(
		EditorState *editor = (EditorState *)((u8 *)mem_store->game_memory.memory + sizeof(GameState));
		editor->init();
	)
}

GAME_UPDATE(gameUpdate) {
	GameState *game = (GameState *)mem_store->game_memory.memory;
	
	
	g_frame_stack.clear();
	
	EDITOR_ONLY(
	EditorState *editor = (EditorState *)((u8 *)mem_store->game_memory.memory + sizeof(GameState));
	if(input->isKeyPressedOnce(Button::ToggleEditor)) {
		editor->enabled = !editor->enabled;
	}
	
	if(editor->enabled) {
		if(!editor->was_enabled) editor->onShow(game);
		editor->update(input, delta, window, game, assets);
	} else {
		if(editor->was_enabled) editor->onHide(game);
		) 
	
		game->update(input, delta, window);
		EDITOR_ONLY(
	}
	
	editor->was_enabled = editor->enabled;
	)
}



GAME_RENDER(gameRender) {
	GameState *game = (GameState *)mem_store->game_memory.memory;
	
	game->render(window, render_context, input, assets);
	
	EDITOR_ONLY(
		EditorState *editor = (EditorState *)((u8 *)mem_store->game_memory.memory + sizeof(GameState));
		if(editor->enabled) editor->render(window, render_context, input, game);
	)
}