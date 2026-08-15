#pragma once
#include "physics.h"

#pragma pack(push, 1)
namespace prism
{
	class steering_data_t // Size: 0x0048
	{
	public:
		array_dyn_t<float> steps;
		int32_t total_steps; //0x0020 (0x04)
		int32_t N00003996; //0x0024 (0x04)
		array_dyn_t<float> model_steering_pos; //0x0028 (0x20)
	};
	static_assert(sizeof(steering_data_t) == 0x48);

	class game_trailer_actor_u : public unit_t // Size: 0x4288
	{
	public:
		char pad_0010[488]; //0x0010 (0x1e8)
		class vehicle_u* vehicle; //0x01F8 (0x08)
		char pad_0200[720]; //0x0200 (0x2d0)
		float suspention_height; //0x04D0 (0x04)
		char pad_04D4[364]; //0x04D4 (0x16c)
		float steering; //0x0640 (0x04)
		char pad_0644[4]; //0x0644 (0x04)
		class steering_data_t* steering_data; //0x0648 (0x08)
		char pad_0650[2328]; //0x0650 (0x918)
		class physics_joint_physx_t* hook_joint; //0x0F68 (0x08) exists when attached
		char pad_0F70[160]; //0x0F70 (0xa0)
		game_trailer_actor_u* slave; //0x1010 (0x08)
		char pad_1018[12912]; //0x1018 (0x3270)
	};
	static_assert(sizeof(game_trailer_actor_u) == 0x4288);
}
#pragma pack(pop)