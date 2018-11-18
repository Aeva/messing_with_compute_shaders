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
shared uint64_t Registers[2];
const uint ACC = 0;
const uint EXT = 1;

const uint OP_STOP = 0;
const uint OP_UNION = 1;
const uint OP_INTERSECT = 2;
const uint OP_REMOVE = 3;
const uint OP_SPHERE = 4;

// Using a struct for now for the sake of prototyping, but the shape
// parameters probably should be moved to their own arrays for compactness.
// And then we can probably pack everything we need just fine into one 32 bit
// uint and do away with the struct.
struct CsgCommand
{
  uint Opcode;
  uint Register;
  vec3 Origin;
  float Radius;
};

// these are going to be inputs eventually
const vec3 ModelCenter = vec3(0, 0, 0);
const vec3 PartitionCount = vec3(16, 16, 16);
const float PartitionWidth = 10; // world space size
const vec3 PartitionSize = PartitionCount * PartitionWidth;

const uint Instructions[9] {
	CsgCommand(OP_SHAPE, ACC, SPHERE, vec3(0,0,0), 10),
	CsgCommand(OP_SHAPE, EXT, SPHERE, vec3(10,0,0), 5),
	CsgCommand(OP_SHAPE, EXT, SPHERE, vec3(-10,0,0), 5),
	CsgCommand(OP_SHAPE, EXT, SPHERE, vec3(0,10,0), 5),
	CsgCommand(OP_SHAPE, EXT, SPHERE, vec3(0,-10,0), 5),
	CsgCommand(OP_SHAPE, EXT, SPHERE, vec3(0,0,10), 5),
	CsgCommand(OP_SHAPE, EXT, SPHERE, vec3(0,0,-10), 5),
	CsgCommand(OP_REMOVE, 0, 0, vec3(0), 0),
	CsgCommand(OP_STOP, 0, 0, vec3(0), 0)
};

// some shape functions
bool SphereFunction(vec3 WorldSpace, vec3 Origin, float Radius)
{
	return distance(WorldSpace, Origin) <= Radius;
}

// do the thing
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
		const CsgCommand Instruction = Instructions[i];
		if (Instruction.Opcode == OP_STOP)
		{
			break;
		}
		else if (bIsLeader && Instruction.Opcode == OP_UNION)
		{
			Registers[ACC] = Registers[ACC] | Registers[EXT];
		}
		else if (bIsLeader && Instruction.Opcode == OP_INTERSECT)
		{
			Registers[ACC] = Registers[ACC] & Registers[EXT];
		}
		else if (bIsLeader && Instruction.Opcode == OP_REMOVE)
		{
			Registers[ACC] = Registers[ACC] & ~Registers[EXT];
		}
		else if (Instruction.Opcode == OP_SPHERE)
		{
			const bool bTest = SphereFunction(WorldSpace, Instruction.Origin, Instruction.Radius);
			if (bTest)
			{
				AtomicOr(Registers[Instruction.Register], VoxelBitmask);
			}
		}
		groupMemoryBarrier();
	}

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