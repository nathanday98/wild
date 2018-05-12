cbuffer PerScene {
	float4x4 projection;	
};

cbuffer PerObject {
	float4x4 model;
	float4 color;
};

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
};

VS_OUTPUT  vs_main(float3 in_pos : POSITION) {
	VS_OUTPUT output;
	float4 world_pos = mul(float4(in_pos, 1.0f), model);
	output.pos = mul(world_pos, projection);
	return output;
}

float4 ps_main(VS_OUTPUT input) : SV_TARGET  {
	return color;
}