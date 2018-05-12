cbuffer PerScene {
	float4x4 projection;	
};

cbuffer PerObject {
	float4x4 model;
	float4 color;
};

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float4 world_pos : POSITION0;
};

VS_OUTPUT  vs_main(float3 in_pos : POSITION) {
	VS_OUTPUT output;
	float4 world_pos = mul(float4(in_pos, 1.0f), model);
	output.pos = mul(world_pos, projection);
	output.world_pos = world_pos;
	return output;
}

float4 ps_main(VS_OUTPUT input) : SV_TARGET  {
	float distance = 1.0f - (length(color - input.world_pos) / 5.0f);
	
	return float4(1.0f, 1.0f,  1.0f, distance);
}