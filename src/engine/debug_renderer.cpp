

struct PerSceneConstants {
	Mat4 projection;	
};

struct PerObjectConstants {
	Mat4 model;
	Vec4 color;	
};

struct TexturedVertex {
	Vec3 position;
	Vec2 uv;	
};


struct DebugRenderer {
	Shader geom_shader = {};
	Shader light_shader = {};
	Shader textured_shader = {};
	ShaderLayout geom_layout = {};
	ShaderLayout textured_layout = {};
	ShaderConstant per_scene = {};
	ShaderConstant per_object = {};
	RenderContext *render_context;
	BlendState alpha_blend;
	Sampler textured_sampler = {};
	
	void init(RenderContext *rc) {
		render_context = rc;
		RenderContext::LayoutElement layout[] = {
			{"POSITION", RenderContext::Format::Vec3}
		};
		
		RenderContext::LayoutElement textured_layout_desc[] = {
			{"POSITION", RenderContext::Format::Vec3},
			{"TEXCOORD", RenderContext::Format::Vec2}	
		};
		
		geom_shader = render_context->createShader("geometry");
		geom_layout = render_context->createShaderLayout(layout, ArrayCount(layout), &geom_shader);
		
		light_shader = render_context->createShader("lights");
		textured_shader = render_context->createShader("textured");
		textured_layout = render_context->createShaderLayout(textured_layout_desc, ArrayCount(textured_layout_desc), &textured_shader);
		
		per_scene = render_context->createShaderConstant(sizeof(PerSceneConstants));
		per_object = render_context->createShaderConstant(sizeof(PerObjectConstants));
		
		alpha_blend = render_context->createBlendState();
		
		textured_sampler = render_context->createSampler();
	}
	
	void startGeom(Mat4 cam_transform = Mat4()) {
		render_context->bindShader(&geom_shader);
		PerSceneConstants scene_constants = {cam_transform};
		render_context->updateShaderConstant(&per_scene, &scene_constants);
		render_context->bindShaderConstant(&per_scene, 0, -1);
		const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
		render_context->bindBlendState(&alpha_blend, blend_factor, 0xffffffff);
	}
	
	void drawPolyStrip(Vec3 *vertices, u32 vertex_count, Vec4 color = Vec4(1.0f), Mat4 model = Mat4(), RenderContext::Topology topology = RenderContext::Topology::TriangleStrip) {
		
		VertexBuffer vb = render_context->createVertexBuffer(vertices, sizeof(Vec3), vertex_count);
		
		render_context->bindShaderLayout(&geom_layout);
		render_context->bindVertexBuffer(&vb, 0);
		PerObjectConstants object_constants = {model, color};
		render_context->updateShaderConstant(&per_object, &object_constants);
		render_context->bindShaderConstant(&per_object, 1, 0);
		render_context->sendDraw(topology, vertex_count);
		
		render_context->destroyVertexBuffer(&vb);
	}	
	
	void drawTexturedPolyStrip(TexturedVertex *vertices, u32 vertex_count, Texture2D *texture, Vec4 color = Vec4(1.0f), Mat4 model = Mat4(), RenderContext::Topology topology = RenderContext::Topology::TriangleStrip) {
		
		VertexBuffer vb = render_context->createVertexBuffer(vertices, sizeof(TexturedVertex), vertex_count);
		render_context->bindTexture2D(texture);
		render_context->bindSampler(&textured_sampler);
		render_context->bindShaderLayout(&textured_layout);
		render_context->bindVertexBuffer(&vb, 0);
		PerObjectConstants object_constants = {model, color};
		render_context->updateShaderConstant(&per_object, &object_constants);
		render_context->bindShaderConstant(&per_object, 1, 0);
		render_context->sendDraw(topology, vertex_count);
		// render_context->bindTexture2D(0);
		render_context->destroyVertexBuffer(&vb);
	}	
	
	void drawLine(Vec2 p0, Vec2 p1, f32 thickness = 0.025f, Vec4 color = Vec4(1.0f)) {
		Vec2 n = Vec2::normalize(p1 - p0);
		Vec2 r = Vec2::rPerp(n);
		Vec2 l = Vec2::lPerp(n);
		Vec2 top_left = p1 + l * thickness;
		Vec2 top_right = p1 + r * thickness;
		
		Vec2 bottom_left = p0 + l * thickness;
		Vec2 bottom_right = p0 + r * thickness;
		
		Vec3 vertices[] = {
			Vec3(top_left, 0.0f),
			Vec3(top_right, 0.0f),
			Vec3(bottom_left, 0.0f),
			Vec3(bottom_right, 0.0f),
		};
		
		drawPolyStrip(vertices, ArrayCount(vertices), color);
	}
	
	void drawCircle(Vec2 position, f32 radius, Vec4 color = Vec4(1.0f), int num_splits = 36) {
		
	}
	
	void fillCircle(Vec2 position, f32 radius, Vec4 color = Vec4(1.0f), int num_splits = 36) {
		VertexStack vs(&g_frame_stack);
		
		for(int i = -180; i < 181; i+= Math::ceilToInt(360.0f / num_splits)) {
			Vec3 vert = Vec3();
			Vec3 vert1 = Vec3();
			
			vert.x = Math::sin(Math::toRadians(i)) * radius;
			vert.y = Math::cos(Math::toRadians(i)) * radius;
			
			vert1.x = Math::sin(Math::toRadians(i + 180)) * radius;
			vert1.y = Math::cos(Math::toRadians(i + 180)) * radius;
		
			vs.push(vert);
			vs.push(vert1);
		}
		
		drawPolyStrip(vs.getPointer(), vs.getCount(), color, Mat4::translate(Vec3(position, 0.0f)));
		
		vs.clear();
	}
	
	void drawCubBezier(Vec2 a, Vec2 b, Vec2 c, Vec2 d, f32 thickness = 0.025f, Vec4 color = Vec4(1.0f), int tess_detail = 20) {
		Vec2 previous = Math::cubCurve(a, b, c, d, 0.0f);
		for(int i = 1; i < tess_detail+1; i++) {
			f32 t = (1.0f / (f32)tess_detail) * (f32)i;
			Vec2 p = Math::cubCurve(a, b, c, d, t);
			Vec2 normal = Vec2::lPerp(Vec2::normalize(p - previous));
			// fillCircle(p + normal * 0.5f, 0.1f);
			// fillCircle(p - normal * 0.5f, 0.1f);
			if(i == 1) {
				// fillCircle(previous + normal * 0.5f, 0.1f);
				// fillCircle(previous - normal * 0.5f, 0.1f);
			}
			drawLine(previous, p, thickness, color);
			previous = p;
		}
	}
	
	void drawAABB(AABB box, Vec4 color = Vec4(1.0f, 0.0f, 0.0f, 1.0f)) {
		f32 thickness = 0.025f;
		drawLine(box.min, Vec2(box.max.x, box.min.y), thickness, color);
		drawLine(Vec2(box.max.x, box.min.y), box.max, thickness, color);
		drawLine(box.max, Vec2(box.min.x, box.max.y),  thickness, color);
		drawLine(box.min, Vec2(box.min.x, box.max.y),  thickness, color);
	}
	
	void fillRect(Vec2 center, Vec2 size, Vec4 color =  Vec4(1.0f)) {
		Vec3 hs = Vec3(size * 0.5f, 0.0f);
		Vec3 vertices[] = {
			Vec3(center + Vec2(-hs.x, -hs.y), 0.0f),
			Vec3(center + Vec2(-hs.x, hs.y), 0.0f),
			Vec3(center + Vec2(hs.x, -hs.y), 0.0f),
			Vec3(center + Vec2(hs.x, hs.y), 0.0f),
		};
		
		drawPolyStrip(vertices, 4, color, Mat4());
	}
	
	void fillTexturedRect(Vec2 center, Vec2 size, Texture2D *texture, Vec4 color =  Vec4(1.0f), Vec2 uv_offset = Vec2(), Vec2 uv_scale = Vec2(1.0f), Mat4 world = Mat4()) {
		Vec3 hs = Vec3(size * 0.5f, 0.0f);
		TexturedVertex vertices[] = {
			{Vec3(center + Vec2(-hs.x, -hs.y), 0.0f), uv_offset + Vec2(0.0f, 1.0f) * uv_scale},
			{Vec3(center + Vec2(-hs.x, hs.y), 0.0f), uv_offset + Vec2(0.0f, 0.0f) * uv_scale},
			{Vec3(center + Vec2(hs.x, -hs.y), 0.0f), uv_offset + Vec2(1.0f, 1.0f) * uv_scale},
			{Vec3(center + Vec2(hs.x, hs.y), 0.0f), uv_offset + Vec2(1.0f, 0.0f) * uv_scale},
		};
		
		drawTexturedPolyStrip(vertices, 4, texture, color, world);
	}
};