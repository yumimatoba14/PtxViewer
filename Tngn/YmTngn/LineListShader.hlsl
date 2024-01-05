#include "ShaderParam.hlsl"

struct VS_INPUT
{
	float3 Pos : POSITION;
	float4 Col : COLOR;
#if PICKABLE_MODE
	uint4 PickTargetId : PICK_ID;
#endif
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Col : COLOR;
#if PICKABLE_MODE
	uint4 PickTargetId : PICK_ID;
#endif
};

struct PS_OUTPUT
{
	float4 Color : SV_TARGET0;
#if PICKABLE_MODE
	uint4 PickTargetId : SV_TARGET1;
#endif
};

#if PICKABLE_MODE
#define COPY_PICK_TARGET_ID(o, i) (o).PickTargetId = (i).PickTargetId
#else
#define COPY_PICK_TARGET_ID(o, i) (0)
#endif

PS_INPUT vsMain(VS_INPUT pos)
{
	PS_INPUT o = (PS_INPUT)0;
	float4 coord = float4(pos.Pos, 1);
	coord = mul(coord, viewMatrix);
	o.Pos = mul(coord, projectionMatrix);
	o.Col = pos.Col;	// TODO: Use normal direction. or pass it to Pixel shader.
	COPY_PICK_TARGET_ID(o, pos);
	return o;
}

PS_OUTPUT psMain(PS_INPUT input)
{
	PS_OUTPUT output;
	output.Color = input.Col;
	COPY_PICK_TARGET_ID(output, input);
	return output;
}
