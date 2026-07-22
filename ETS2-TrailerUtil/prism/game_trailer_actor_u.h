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

	class game_trailer_actor_u : public unit_t
	{
	public:
		char pad_0010[408]; //0x0010 (0x198)
		class vehicle_u* vehicle; //0x01A8 (0x08)
		char pad_01B0[624]; //0x01B0 (0x270)
		float suspention_height; //0x0420 (0x04)
		char pad_0424[292]; //0x0424 (0x124)
		float steering; //0x0548 (0x04)
		char pad_054C[4]; //0x054C (0x04)
		class steering_data_t* steering_data; //0x0550 (0x08)
		char pad_0558[2240]; //0x0558 (0x8c0)
		class physics_joint_physx_t* hook_joint; //0x0E18 (0x08) exists when attached
		char pad_0E20[136]; //0x0E20 (0x88)
		game_trailer_actor_u* slave; //0x0EA8 (0x08)
		char pad_0EB0[460]; //0x0EB0 (0x1cc)
		float trailer_brace_animation_target; //0x107C (0x04) 0 = lowered
		float trailer_brace_animation_position; //0x1080 (0x04)
		float trailer_brace_animation_speed; //0x1084 (0x04)
		char pad_1088[2120]; //0x1088 (0x848)
	};
	static_assert(sizeof(game_trailer_actor_u) == 0x18D0);
}
#pragma pack(pop)