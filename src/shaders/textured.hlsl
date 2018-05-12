cbuffer PerScene {
	float4x4 projection;	
};

cbuffer PerObject {
	float4x4 model;
	float4 color;
};

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD0;
};

VS_OUTPUT  vs_main(float3 in_pos : POSITION, float2 in_tex : TEXCOORD) {
	VS_OUTPUT output;
	float4 world_pos = mul(float4(in_pos, 1.0f), model);
	output.pos = mul(world_pos, projection);
	output.uv = in_tex;
	return output;
}


sampler sampler0;
Texture2D texture0;

float4 ps_main(VS_OUTPUT input) : SV_TARGET  {
	float4 out_color = color * texture0.Sample(sampler0, input.uv);
	return out_color;
}