#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include <engine/std.h>
#include <SDL2/SDL.h>
#include <string>
#include <core/platform.h>

struct PlatformPacket;
struct PlatformRenderState;

struct Shader {
	void *vertex_shader;
	void *pixel_shader;
	FileData vertex_file, pixel_file;	
};

struct ShaderLayout {
	void *layout;	
};

struct ShaderConstant {
	void *buffer;	
};

struct VertexBuffer {
	void *buffer;
	u32 vertex_size;	
};

struct RasterState {
	void *state;
};

struct BlendState {
	void *state;	
};

struct DepthStencilState {
	void *state;	
};

struct Texture2D {
	void *texture;
	u32 width;
	u32 height;
};

struct Sampler {
	void *sampler;	
};

struct RenderContext {
	PlatformPacket *pp;
	
	enum class Format {
		Vec2,
		Vec3,
		u32_unorm,
		u32,
		u16,
		MAX
	};
	
	static u32 format_values[Format::MAX];
	static u32 format_sizes[Format::MAX];
	
	struct LayoutElement {
		char *name;
		Format format;
	};
	
	enum class Topology {
		TriangleStrip,
		TriangleList,
		MAX
	};
	
	static u32 topologies[Topology::MAX];
	
	enum class BufferType {
		Vertex,
		Index,
		MAX
	};
	
	static u32 buffer_types[BufferType::MAX];
	
	virtual void setClipRect(s32 x, s32 y, s32 w, s32 h);
	virtual void setViewport(s32 width, s32 height, f32 min_depth, f32 max_depth);
	virtual void resizeBuffer(s32 width, s32 height);
	virtual void init(s32 width, s32 height, s32 refresh_rate, SDL_Window *window);
	virtual void uninit();
	virtual void clear(float color[4]);
	virtual void present();
	
	virtual Texture2D createTexture2D(void *data, u32 width, u32 height, Format format);
	virtual void bindTexture2D(Texture2D *texture);
	virtual void destroyTexture2D(Texture2D *texture);
	
	virtual Sampler createSampler();
	virtual void bindSampler(Sampler *sampler);
	virtual void destroySampler(Sampler *sampler);
	
	virtual Shader createShader(const std::string &name);
	virtual void destroyShader(Shader *shader);
	virtual void bindShader(Shader *shader);
	virtual void unbindShader();
	
	virtual ShaderLayout createShaderLayout(RenderContext::LayoutElement *elements, u32 count, Shader *shader);
	virtual void bindShaderLayout(ShaderLayout *constant);
	
	virtual ShaderConstant createShaderConstant(u32 buffer_size);
	virtual void updateShaderConstant(ShaderConstant *constant, void *data);
	virtual void bindShaderConstant(ShaderConstant *constant, s32 vs_loc, s32 ps_loc);
	
	virtual VertexBuffer createVertexBuffer(void *vertices, u32 vertex_size, u32 num_vertices, BufferType type = BufferType::Vertex);
	virtual void destroyVertexBuffer(VertexBuffer *vb);
	virtual void bindVertexBuffer(VertexBuffer *vb, u32 slot);
	virtual void bindIndexBuffer(VertexBuffer *vb, Format format);
	
	virtual RasterState createRasterState(bool scissor_enabled, bool depth_enabled);
	virtual void bindRasterState(RasterState *state);
	virtual void destroyRasterState(RasterState *state);
	
	virtual BlendState createBlendState();
	virtual void bindBlendState(BlendState *state, const float factor[4], u32 mask);
	virtual void destroyBlendState(BlendState *state);
	
	virtual DepthStencilState createDepthStencilState();
	virtual void bindDepthStencilState(DepthStencilState *state);
	virtual void destroyDepthStencilState(DepthStencilState *state);

	virtual PlatformRenderState *saveRenderState();
	virtual void reloadRenderState(PlatformRenderState *state);
	virtual void destroyRenderState(PlatformRenderState *state);
	
	virtual void sendDraw(Topology topology, u32 num_vertices);
	virtual void sendDrawIndexed(Topology topology, u32 num_indices, int vertex_offset = 0, int index_offset = 0);
};

#endif // RENDER_CONTEXT_H