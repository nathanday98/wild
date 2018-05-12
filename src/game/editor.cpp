struct Handle {
	Vec2 position;	
	f32 radius;	
	
	static f32 DefRadius;
};

f32 Handle::DefRadius = 0.2f;

struct EditorState {
	bool enabled;
	bool was_enabled;
	int selected_handle_index;
	std::vector<Handle> handles;
	Camera camera;
	Vec2 start_mouse_pos;
	
	void init() {
		selected_handle_index = -1;
		enabled = false;
		was_enabled = false;
	}
	
	void onShow(GameState *game) {
		camera = game->camera;
	}
	
	void onHide(GameState *game) {
		for(int i = 0; i < game->scene.static_aabbs.size(); i++) {
			AABB &collider = game->scene.static_aabbs[i];
			if(collider.min.x > collider.max.x) Swap(collider.min.x, collider.max.x);
			if(collider.min.y > collider.max.y) Swap(collider.min.y, collider.max.y);
		}
	}
	
	
	Vec2 useHandle( InputManager *input, Vec2 position, f32 radius = Handle::DefRadius, Vec2 offset = Vec2(), Vec2 axis_lock = Vec2(1.0f, 1.0f)) {
		Vec2 delta = camera.getWorldMouseDelta();
		Circle hc = {position + offset, radius};
		int i = (int)handles.size();
		if(input->isMouseButtonDownOnce(0) && hc.pointInside(camera.world_mouse_pos)) {
			selected_handle_index = i;
		}
		
		if(selected_handle_index == i) {
			if(!input->isMouseButtonDown(0)) selected_handle_index = -1;
			position += delta * axis_lock;
		}
		handles.push_back({position + offset, radius});
		return position;
	}
	
	// NOTE(nathan): must be called in render if you want the lines to be drawn - maybe make the lines immediate mode??
	Vec2 usePosHandle(InputManager *input, Vec2 position, GameState *game) {
		position = useHandle(input, position);
		Vec2 r = Vec2(2.0f, 0.0f);
		Vec2 u = Vec2(0.0f, 2.0f);
		position = useHandle(input, position, Handle::DefRadius, r, Vec2::right());
		position = useHandle(input, position, Handle::DefRadius, u, Vec2::up());
		game->debug_renderer.drawLine(position, position + r);
		game->debug_renderer.drawLine(position, position + u);
		return position;
	}
	
	void update(InputManager *input, f32 delta, SDL_Window *window, GameState *game, Assets *game_assets) {
		camera.beginFrame(window, input);
		handles.clear();
		
		f32 e_width = 200.0f;
		f32 margin = 5.0f;
		ImGui::SetNextWindowPos(ImVec2(camera.window_dimensions.x - (e_width + margin), margin));
		ImGui::SetNextWindowSize(ImVec2(e_width, camera.window_dimensions.y - margin * 2.0f));
		ImGui::Begin("Editor", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize); {
			
			if(ImGui::Button("Clear Level")) game->scene.clear();
			const char *filename = "data/levels/hub.lvl";
			if(ImGui::Button("Save Level")) {
				SceneSaveHeader header = {};
				header.static_aabb_count = (u32)game->scene.static_aabbs.size();
				header.static_aabb_size = (u32)sizeof(AABB);
				header.static_line_count = (u32)game->scene.static_lines.size();
				header.static_line_size = (u32)sizeof(Line);
				header.static_circle_count = (u32)game->scene.static_circles.size();
				header.static_circle_size = (u32)sizeof(Circle);
				header.static_bez_count = (u32)game->scene.static_beziers.size();
				header.static_bez_size = (u32)sizeof(Bezier);
				header.player_start = game->scene.player_start;
				header.data_offset = (u32)sizeof(header);
				
				
				
				void *file = Platform::openFileForWriting(filename);
				
				Platform::writeToFile(file, &header, (s32)sizeof(header));
				s32 aabb_size = (s32)sizeof(AABB);
				for(int i = 0; i < game->scene.static_aabbs.size(); i++) {
					AABB &aabb = game->scene.static_aabbs[i];
					Platform::writeToFile(file, &aabb, aabb_size);
				}
				
				s32 line_size = (s32)sizeof(Line);
				for(int i = 0; i < game->scene.static_lines.size(); i++) {
					Line &line = game->scene.static_lines[i];
					Platform::writeToFile(file, &line, line_size);
				}
				
				s32 circle_size = (s32)sizeof(Circle);
				for(int i = 0; i < game->scene.static_circles.size(); i++) {
					Circle &circle = game->scene.static_circles[i];
					Platform::writeToFile(file, &circle, circle_size);
		
				}
				
				s32 bez_size = (s32)sizeof(Bezier);
				for(int i = 0; i < game->scene.static_beziers.size(); i++) {
					Bezier &bez = game->scene.static_beziers[i];
					Platform::writeToFile(file, &bez, bez_size);
				}
				
				Platform::closeOpenFile(file);
				
			}
			
			for(s32 i = 0; i < game_assets->max_levels; i++) {
				std::string path = "data/levels/" + std::string(game_assets->levels[i]) + ".lvl";
				
				if(ImGui::Button((std::string("Load ") + game_assets->levels[i]).c_str())) {
					FileData fd = Platform::readEntireFile(path.c_str());
					game->scene.loadFromFile(&fd);
				}	
			}
			
			if(ImGui::Button("Reset camera position")) {
				camera.position = Vec2();
				game->camera.position = camera.position;
			}
			
			if(ImGui::Button("Reset player")) {
				game->spawnPlayer();
			}
			
			if(ImGui::Button("Create AABB")) {
				game->scene.addStaticBox({camera.position - Vec2(0.5f), camera.position + Vec2(0.5f)});
			}
			
			if(ImGui::Button("Create Circle")) {
				game->scene.addStaticCircle(camera.position, 2.0f);
			}
			
			if(ImGui::Button("Create bezier curve")) {
				game->scene.addStaticBezier(camera.position + Vec2(-3.0f, -3.0f), camera.position + Vec2(-3.0f, 3.0f), camera.position + Vec2(3.0f, 3.0f), camera.position + Vec2(3.0f, -3.0f))	;
			}
			
			if(ImGui::Button("Create Point light")) {
				game->lights.push_back({camera.position, 5.0f, 360.0f, 0.0f, 360});
			}
			
			
			
			ImGui::End();
		}
		
		if(input->isMouseButtonDown(2)) {
			if(input->isMouseButtonDownOnce(2)) {
				start_mouse_pos = camera.world_mouse_pos;
			}
			Vec2 d = camera.world_mouse_pos - start_mouse_pos;
			
			game->camera.position -= d;
			camera.position -= d;
		}
		
		
	}
	
	void render(SDL_Window *window, RenderContext *render_context, InputManager *input, GameState *game) {
		game->debug_renderer.startGeom(camera.getVP());
		
		
		
		game->player_pos = usePosHandle(input, game->player_pos, game);
		// game->player_pos = useHandle(input, player_up, Handle::DefRadius, Vec2(), Vec2::right()) - Vec2::right() * 2.0f;
		
		game->scene.player_start = usePosHandle(input, game->scene.player_start, game);
		game->debug_renderer.drawAABB({game->scene.player_start - Vec2(0.5f), game->scene.player_start + Vec2(0.5f)}, Vec4(0.0f, 1.0f, 0.0f, 1.0f));
		
		for(int i = 0; i < game->lights.size(); i++) {
			game->lights[i].position = usePosHandle(input, game->lights[i].position, game);
		}
		
		for(int i = 0; i < game->scene.static_beziers.size(); i++) {
			Bezier &bez = game->scene.static_beziers[i];
			Vec2 old0 = bez.points[0];
			Vec2 old1 = bez.points[3];
			bez.points[0] = useHandle(input, bez.points[0]);
			bez.points[3] = useHandle(input, bez.points[3]);
			
			game->debug_renderer.drawLine(bez.points[0], bez.points[1]);
			game->debug_renderer.drawLine(bez.points[2], bez.points[3]);
				
			bez.points[1] += bez.points[0] - old0;
			bez.points[2] += bez.points[3] - old1;
			
			bez.points[1] = useHandle(input, bez.points[1]);
			bez.points[2] = useHandle(input, bez.points[2]);
		}
		
		for(int i = 0; i < game->scene.static_circles.size(); i++) game->debug_renderer.fillCircle(game->scene.static_circles[i].center, game->scene.static_circles[i].radius,Vec4(1.0f, 0.0f, 0.0f, 1.0f));
		
		for(int i = 0; i < game->scene.static_circles.size(); i++) {
			Circle &collider = game->scene.static_circles[i];
			collider.center = usePosHandle(input, collider.center, game);
			Vec2 rpos = collider.center + Vec2(Math::cos(1.0f), Math::sin(1.0f)) * collider.radius;
			rpos = useHandle(input, rpos);
			game->debug_renderer.drawLine(rpos, collider.center);
			collider.radius = Vec2::length(collider.center - rpos);
		}
		
		for(int i = 0; i < game->scene.static_aabbs.size(); i++) {
			AABB &collider = game->scene.static_aabbs[i];
			Vec2 center = (collider.min + collider.max) * 0.5f;
			f32 width = collider.max.x - collider.min.x;
			f32 height = collider.max.y - collider.min.y;
			Vec2 new_center = useHandle(input, center, 0.4f);
			collider.min += new_center - center;
			collider.max += new_center - center;
			collider.min = useHandle(input, collider.min);
			collider.max = useHandle(input, collider.max);
			
			Vec2 top_left = useHandle(input, Vec2(collider.min.x, collider.max.y));
			collider.min.x = top_left.x;
			collider.max.y = top_left.y;
			
			Vec2 bottom_right = useHandle(input, Vec2(collider.max.x, collider.min.y));
			collider.max.x = bottom_right.x;
			collider.min.y = bottom_right.y;
			
			collider.max.x = useHandle(input, Vec2(collider.max.x, collider.min.y + height*0.5f)).x;
			collider.min.x = useHandle(input, Vec2(collider.min.x, collider.min.y + height*0.5f)).x;
			collider.min.y = useHandle(input, Vec2(collider.min.x + width*0.5f, collider.min.y)).y;
			collider.max.y = useHandle(input, Vec2(collider.min.x + width*0.5f, collider.max.y)).y;
			
		}
		
		for(int i = 0; i < handles.size(); i++) {
			Handle &h = handles[i];
			Vec4 color = Vec4(1.0f);
			if(selected_handle_index == i) color = Vec4(1.0f, 0.0f, 0.0f, 1.0f);
			game->debug_renderer.fillCircle(h.position, h.radius, color);
		}
		
		
		
		camera.endFrame();
	}
};