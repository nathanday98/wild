namespace ImGuiImpl {
	static double time = 0.0f;
	static bool mouse_pressed[3] = { false, false, false };
	static float mouse_wheel = 0.0f;
	
	static ShaderConstant im_matrix;
	static Shader im_shader;
	static ShaderLayout im_layout;
	static RasterState im_raster_state;
	static BlendState im_blend_state;
	static DepthStencilState im_depth_stencil_state;
	static Texture2D im_font_texture;
	static Sampler im_font_sampler;

	struct VERTEX_CONSTANT_BUFFER
	{
		float        mvp[4][4];
	};
	
	static void render(RenderContext *ctx) {
		ImDrawData *draw_data = ImGui::GetDrawData();
		int index_count = 0;
		int vertex_count = 0;
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			
			vertex_count += cmd_list->VtxBuffer.Size;
			index_count += cmd_list->IdxBuffer.Size;
		}
		
		ImDrawVert *vertices = (ImDrawVert *)malloc(vertex_count * sizeof(ImDrawVert));
		ImDrawIdx *indices = (ImDrawIdx *)malloc(index_count * sizeof(ImDrawIdx));
		
		ImDrawVert* vtx_dst = (ImDrawVert*)vertices;
		ImDrawIdx* idx_dst = (ImDrawIdx*)indices;
		for (int n = 0; n < draw_data->CmdListsCount; n++) {
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtx_dst += cmd_list->VtxBuffer.Size;
			idx_dst += cmd_list->IdxBuffer.Size;
		}
		
		VertexBuffer vb = ctx->createVertexBuffer(vertices, sizeof(ImDrawVert), vertex_count);
		VertexBuffer ib = ctx->createVertexBuffer(indices, sizeof(ImDrawIdx), index_count, RenderContext::BufferType::Index);

		PlatformRenderState *render_state = ctx->saveRenderState();

		ctx->bindShader(&im_shader);

		// Setup orthographic projection matrix into our constant buffer
		{
			
			float L = 0.0f;
			float R = ImGui::GetIO().DisplaySize.x;
			float B = ImGui::GetIO().DisplaySize.y;
			float T = 0.0f;
			float mvp[4][4] =
			{
				{ 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
				{ 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
				{ 0.0f,         0.0f,           0.5f,       0.0f },
				{ (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
			};
			
			ctx->updateShaderConstant(&im_matrix, mvp);
			ctx->bindShaderConstant(&im_matrix, 0, -1);
		}
		
		ctx->setViewport(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y, 0.0f, 1.0f);

		// Bind shader and vertex buffers
		unsigned int stride = sizeof(ImDrawVert);
		unsigned int offset = 0;
		
		ctx->bindShaderLayout(&im_layout);
		ctx->bindVertexBuffer(&vb, 0);
		ctx->bindIndexBuffer(&ib, sizeof(ImDrawIdx) == 2 ? RenderContext::Format::u16 : RenderContext::Format::u32);
		
		
		
		ctx->bindSampler(&im_font_sampler);
		
		// Setup render state
		const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
		ctx->bindBlendState(&im_blend_state, blend_factor, 0xffffffff);
		
		ctx->bindDepthStencilState(&im_depth_stencil_state);
		ctx->bindRasterState(&im_raster_state);

		// Render command lists
		int vtx_offset = 0;
		int idx_offset = 0;
		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList* cmd_list = draw_data->CmdLists[n];
			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
				if (pcmd->UserCallback)
				{
					pcmd->UserCallback(cmd_list, pcmd);
				}
				else
				{
					ctx->bindTexture2D(&im_font_texture);
					
					ctx->setClipRect(pcmd->ClipRect.x, pcmd->ClipRect.y, pcmd->ClipRect.z, pcmd->ClipRect.w);
					// VertexBuffer vb = ctx->createVertexBuffer(cmd_list->VtxBuffer.Data, sizeof(ImDrawVert), cmd_list->VtxBuffer.Size);
					// VertexBuffer ib = ctx->createVertexBuffer(cmd_list->IdxBuffer.Data, sizeof(ImDrawIdx), cmd_list->IdxBuffer.Size, RenderContext::BufferType::Index);
					
					
					// ctx->bindVertexBuffer(&vb, 0);
					// ctx->bindIndexBuffer(&ib, sizeof(ImDrawIdx) == 2 ? RenderContext::Format::u16 : RenderContext::Format::u32);
					ctx->sendDrawIndexed(RenderContext::Topology::TriangleList, pcmd->ElemCount, vtx_offset, idx_offset);
					
				}
				idx_offset += pcmd->ElemCount;
			}
			vtx_offset += cmd_list->VtxBuffer.Size;
		}
		
		free(vertices);
		free(indices);
		
		ctx->destroyVertexBuffer(&vb);
		ctx->destroyVertexBuffer(&ib);
		
		ctx->reloadRenderState(render_state);
		ctx->destroyRenderState(render_state);
	}
	
	static void createFontsTexture(RenderContext *ctx) {
		 // Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
		
		im_font_texture = ctx->createTexture2D(pixels, width, height, RenderContext::Format::u32_unorm);
		im_font_sampler = ctx->createSampler();
	}
	
	static void invalidateDeviceObjects() {
		
	}
	
	static void createDeviceObjects(RenderContext *ctx) {
		if (im_font_sampler.sampler)
			invalidateDeviceObjects();

		// By using D3DCompile() from <d3dcompiler.h> / d3dcompiler.lib, we introduce a dependency to a given version of d3dcompiler_XX.dll (see D3DCOMPILER_DLL_A)
		// If you would like to use this DX11 sample code but remove this dependency you can: 
		//  1) compile once, save the compiled shader blobs into a file or source code and pass them to CreateVertexShader()/CreatePixelShader() [preferred solution]
		//  2) use code to detect any version of the DLL and grab a pointer to D3DCompile from the DLL. 
		// See https://github.com/ocornut/imgui/pull/638 for sources and details.

		// Create the vertex shader
		im_shader = ctx->createShader("imgui");

		RenderContext::LayoutElement layout_elements[] = {
			{"POSITION", RenderContext::Format::Vec2},
			{"TEXCOORD", RenderContext::Format::Vec2},
			{"COLOR", RenderContext::Format::u32_unorm},
		};
		
		im_layout = ctx->createShaderLayout(&layout_elements[0], ArrayCount(layout_elements), &im_shader);

		im_matrix = ctx->createShaderConstant(sizeof(VERTEX_CONSTANT_BUFFER));


		im_blend_state = ctx->createBlendState();
		
		im_raster_state = ctx->createRasterState(true, true);

		// Create depth-stencil State
		im_depth_stencil_state = ctx->createDepthStencilState();

		createFontsTexture(ctx);
	}
	
	
	static void shutdown() {
		invalidateDeviceObjects();
		ImGui::Shutdown();
	}
	
	static const char *GetClipboardText(void *) {
		return SDL_GetClipboardText();
	}
	
	static void setClipboardText(void *, const char *text) {
		SDL_SetClipboardText(text);
	}
	
	static void init(SDL_Window *window) {
		ImGuiIO& io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;                     // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
		io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
		io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
		io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
		io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
		io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
		io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
		io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
		io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
		io.KeyMap[ImGuiKey_A] = SDLK_a;
		io.KeyMap[ImGuiKey_C] = SDLK_c;
		io.KeyMap[ImGuiKey_V] = SDLK_v;
		io.KeyMap[ImGuiKey_X] = SDLK_x;
		io.KeyMap[ImGuiKey_Y] = SDLK_y;
		io.KeyMap[ImGuiKey_Z] = SDLK_z;
		
	#ifdef _WIN32
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(window, &wmInfo);
		io.ImeWindowHandle = wmInfo.info.win.window;
	#else
		(void)window;
	#endif
		
		
		io.SetClipboardTextFn = setClipboardText;
		io.GetClipboardTextFn = GetClipboardText;
		// io.ClipboardUserData = NULL;
	}
	
	static void processEvent(SDL_Event *event) {
		ImGuiIO& io = ImGui::GetIO();
		switch (event->type)
		{
		case SDL_MOUSEWHEEL:
			{
				if (event->wheel.y > 0)
					mouse_wheel = 1;
				if (event->wheel.y < 0)
					mouse_wheel = -1;
				return;
			}
		case SDL_MOUSEBUTTONDOWN:
			{
				if (event->button.button == SDL_BUTTON_LEFT) mouse_pressed[0] = true;
				if (event->button.button == SDL_BUTTON_RIGHT) mouse_pressed[1] = true;
				if (event->button.button == SDL_BUTTON_MIDDLE) mouse_pressed[2] = true;
				return;
			}
		case SDL_TEXTINPUT:
			{
				io.AddInputCharactersUTF8(event->text.text);
				return;
			}
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			{
				int key = event->key.keysym.sym & ~SDLK_SCANCODE_MASK;
				io.KeysDown[key] = (event->type == SDL_KEYDOWN);
				io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
				io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
				io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
				io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
				return;
			}
		}
		return;
	}
	
	static void newFrame(SDL_Window *window, RenderContext *ctx) {
		if (!im_font_sampler.sampler) createDeviceObjects(ctx);

		ImGuiIO& io = ImGui::GetIO();

		// Setup display size (every frame to accommodate for window resizing)
		int w, h;
		int display_w, display_h;
		SDL_GetWindowSize(window, &w, &h);
		display_w = w;
		display_h = h;
		io.DisplaySize = ImVec2((float)w, (float)h);
		io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

		// Setup time step
		u32	local_time = SDL_GetTicks();
		double current_time = local_time / 1000.0;
		io.DeltaTime = time > 0.0 ? (float)(current_time - time) : (float)(1.0f / 60.0f);
		time = current_time;

		// Setup inputs
		// (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
		int mx, my;
		u32  mouseMask = SDL_GetMouseState(&mx, &my);
		if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)
			io.MousePos = ImVec2((float)mx, (float)my);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
		else
			io.MousePos = ImVec2(-1, -1);

		io.MouseDown[0] = mouse_pressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
		io.MouseDown[1] = mouse_pressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
		io.MouseDown[2] = mouse_pressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
		mouse_pressed[0] = mouse_pressed[1] = mouse_pressed[2] = false;

		io.MouseWheel = mouse_wheel;
		mouse_wheel = 0.0f;

		// Hide OS mouse cursor if ImGui is drawing it
		SDL_ShowCursor(io.MouseDrawCursor ? 0 : 1);

		// Start the frame
		ImGui::NewFrame();
	}
}