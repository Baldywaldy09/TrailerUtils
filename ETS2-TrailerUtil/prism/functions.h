#pragma once

#include "game_trailer_actor_u.h"

namespace prism
{
	typedef void (*g_set_vehicle_steering_t)(
		prism::steering_data_t* vehicle_steering_data,
		float steering_pos
	);
	inline g_set_vehicle_steering_t g_set_vehicle_steering;

	typedef __int64 (*g_advance_steering_t)(
		prism::game_trailer_actor_u* vehicle_actor
	);
	inline g_advance_steering_t g_advance_steering;
};