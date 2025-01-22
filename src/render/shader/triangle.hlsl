struct VS_INPUT {
	float3 pos   : POSITION;		// these names must match D3D11_INPUT_ELEMENT_DESC array
    float2 uv    : TEXCOORD;
    float4 color : COLOR;
};

struct PS_INPUT {
	float4 pos   : SV_POSITION; 	// these names do not matter, except SV_... ones
    float2 uv    : TEXCOORD;
    float4 color : COLOR;
};


// ------- frame_buffer
// b0 = constant buffer bound to slot 0 
cbuffer cbuffer0 : register(b0)	{
	float4x4 view_projection;
}

// ------- object buffer
cbuffer cbuffer1 : register(b1)	{
	float4x4 world;
}

// ------- View matrix buffer

// s0 = sampler bound to slot 0
sampler sampler0 : register(s0);	

// t0 = shader resource bound to slot 0
Texture2D<float4> texture0 : register(t0); 

PS_INPUT vs(VS_INPUT input) {
	PS_INPUT output;
	
	// rotation + pos transform
    output.pos = mul(float4(input.pos, 1), view_projection);
	
	output.uv = input.uv;
    output.color = input.color;
    return output;
}

float4 ps(PS_INPUT input) : SV_TARGET {
	float4 tex = texture0.Sample(sampler0, input.uv);
    return input.color * tex;
}   