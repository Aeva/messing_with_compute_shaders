#version 420
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_storage_buffer_object : require
#extension GL_ARB_shader_image_load_store : require
#extension GL_ARB_gpu_shader5 : require

// these are determined by the work group size
const vec3 VoxelSize = vec3(4, 4, 4);
const uvec3 MASK_3D = uvec3(3, 12, 48);
const uvec2 MASK_2D = uvec2(7, 56);

// this is the machinery for processing the csg tree
shared uint64_t Registers[3];
const uint ACC = 0;
const uint EXT = 1;
const uint TMP = 2; // can't be used by instructions

// Current instruction layout:
// 31..30 -> csg mode
//  29..1 -> shape index
//      0 -> register
const uint OPCODE_OFFSET = 30;
const uint OPCODE_MASK = 3 << OPCODE_OFFSET;
const uint SHAPE_OFFSET = 1;
const uint SHAPE_MASK = 0x1fffffff << SHAPE_OFFSET;
const uint REGISTER_OFFSET = 0;
const uint REGISTER_MASK = 1;

const uint OP_STOP 			= 0 << OPCODE_OFFSET;
const uint OP_UNION 		= 1 << OPCODE_OFFSET;
const uint OP_INTERSECT 	= 2 << OPCODE_OFFSET;
const uint OP_REMOVE 		= 3 << OPCODE_OFFSET;

// Special modes:
const uint SPECIAL_COMBINE	= SHAPE_MASK >> SHAPE_OFFSET;

// Handy shape codes:
const float SHAPE_SPHERE = 0;
const float fSHAPE_SPHERE = uintBitsToFloat(SHAPE_SPHERE);



// ---- (PRETEND) SHADER INPUTS ----//


const vec3 ModelCenter = vec3(0, 0, 0);
const vec3 PartitionCount = vec3(16, 16, 16);
const float PartitionWidth = 10; // world space size
const vec3 PartitionSize = PartitionCount * PartitionWidth;

const float ShapeStuff[40] = {
	fSHAPE_SPHERE, 0, 0, 0, 10,  // offset 0
	fSHAPE_SPHERE, 10, 0, 0, 4,  // offset 5
	fSHAPE_SPHERE, -10, 0, 0, 4, // offset 10
	fSHAPE_SPHERE, 0, 10, 0, 4,  // offset 15
	fSHAPE_SPHERE, 0, -10, 0, 4, // offset 20
	fSHAPE_SPHERE, 0, 0, 10, 4,  // offset 25
	fSHAPE_SPHERE, 0, 0, -10, 4  // offset 30
	fSHAPE_SPHERE, 0, 0, 0, 6,  // offset 35
};

const uint Instructions[9] = {
	OP_UNION | ACC | (0 << SHAPE_OFFSET),
	OP_REMOVE | ACC | (5 << SHAPE_OFFSET),
	OP_REMOVE | ACC | (10 << SHAPE_OFFSET),
	OP_REMOVE | ACC | (15 << SHAPE_OFFSET),
	OP_REMOVE | ACC | (20 << SHAPE_OFFSET),
	OP_REMOVE | ACC | (25 << SHAPE_OFFSET),
	OP_REMOVE | ACC | (30 << SHAPE_OFFSET),
	OP_REMOVE | ACC | (35 << SHAPE_OFFSET),
	OP_STOP
};



// ---- SHAPE FUNCTIONS ---- //


bool SphereFunction(vec3 WorldPosition, vec3 Origin, float Radius)
{
	return distance(WorldPosition, Origin) <= Radius;
}



// ---- CSG MACHINERY ---- //


// This method determines the correct shape function and parameters for a
// given shape index and then calls the shape function for the given world
// position.
bool RenderShape(const vec3 WorldPosition, const uint ShapeIndex)
{
	const uint ShapeType = floatToUintBits(ShapeStuff[ShapeIndex]);
	if (ShapeIndex == SHAPE_SPHERE)
	{
		const vec3 SphereCenter = vec3(
			ShapeStuff[ShapeIndex+1],
			ShapeStuff[ShapeIndex+2],
			ShapeStuff[ShapeIndex+3]
		);
		const float SphereRadius = ShapeStuff[ShapeIndex+4];
		return SphereFunction(WorldPosition, SphereCenter, SphereRadius);
	}
	return false;
}


// This applies a given CSG operator on two registers and stores the result
// into whichever register was given as the left hand register.
void CombineShapes(const uint Opcode, const uint LHS, const uint RHS)
{
	if (Opcode == OP_UNION && Operand > 0)
	{
		AtomicOr(Registers[LHS], Registers[RHS]);
	}
	else if (Opcode == OP_INTERSECT)
	{
		AtomicAnd(Registers[LHS], Registers[RHS]);
	}
	else if (Opcode == OP_REMOVE)
	{
		AtomicAnd(Registers[LHS], ~Registers[RHS]);
	}
}


// Solve a CSG tree and render the results.
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;
void main()
{
	// Part one: Where are we?
	const vec3 PartitionUV =  gl_WorkGroupID.xyz / PartitionCount * 2 - 1;
	const vec3 PartitionCenter = PartitionUV * PartitionSize + ModelCenter;
	const vec3 WorldSpaceLow = -PartitionSize * 0.5 + PartitionCenter;
	const vec3 WorldSpaceHigh = PartitionSize * 0.5 + PartitionCenter;

	const bool bIsLeader = gl_LocalInvocationIndex == 0;
	const uint64_t VoxelBitmask = 1 << gl_LocalInvocationIndex;
	const uvec3 VoxelIndex = ((MASK_3D & gl_LocalInvocationIndex) >> uvec3(0, 2, 4));
	const vec3 VoxelUV = VoxelIndex / VoxelSize;
	const vec3 WorldSpace = mix(WorldSpaceLow, WorldSpaceHigh, VoxelUV);

	// At this point, each thread in the work group has a world space
	// coordinate associated to it and a csg bitmask.  The next step is to
	// solve the csg tree.

	// Part two: Solve the csg tree.
	for (uint i=0; true; ++i)
	{
		const uint Instruction = Instructions[i];
		const uint Opcode = Instruction & OPCODE_MASK;
		const uint Register = Instruction & REGISTER_MASK;
		const uint ShapeIndex = (Instruction & SHAPE_MASK) >> SHAPE_OFFSET;

		if (Opcode == OP_STOP)
		{
			break;
		}
		else
		{
			if (bIsLeader)
			{
				if (ShapeIndex == SPECIAL_COMBINE)
				{
					const uint MOVE = Register == ACC ? EXT : ACC;
					Registers[TMP] = Registers[MOVE];
				}
				else
				{
					Registers[TMP] = 0;
				}
			}
			groupMemoryBarrier();

			if (ShapeIndex != SPECIAL_COMBINE && RenderShape(WorldSpace, ShapeIndex))
			{
				AtomicOr(Registers[TMP], VoxelBitmask);
			}
			groupMemoryBarrier();

			if (bIsLeader)
			{
				CombineShapes(Opcode, Register, TMP);
			}
		}
	}
	groupMemoryBarrier();

	// Part three: Project the spatial partition into screen space.

	// Part four: Draw!

	/*
	const ivec2 Pixel = (MASK_2D & gl_LocalInvocationIndex) >> ivec3(0, 2);

	uint Offset = 1 << gl_LocalInvocationIndex;
	atomicOr(Morp, Offset);
	groupMemoryBarrier();

	float RedChannel = float(gl_LocalInvocationIndex % 2);

	vec4 Color = vec4(RedChannel, 0.0, 0.0, 1.0);
	imageStore(img_output, Pixel, Color);
	*/
}