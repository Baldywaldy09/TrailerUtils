#pragma once

#include "string.h"
#include <cstdint>

#pragma pack(push, 1)
namespace prism
{
	struct joint_type {
		enum Enum : uint32_t {
			fixed = 1,
			slider = 2,
			ball = 3,
			hinge = 4,
			double_hinge = 5,
			cone_twist = 6,
			six_dof = 7,
			six_dof_sping_v1 = 8,
			six_dof_sping_v2 = 9,
			cabin = 10,
		};
	};

	struct PxD6Motion
	{
		enum Enum
		{
			eLOCKED,	//!< The DOF is locked, it does not allow relative motion.
			eLIMITED,	//!< The DOF is limited, it only allows motion within a specific range.
			eFREE		//!< The DOF is free and has its full range of motion.
		};
	};


	struct PxD6Axis
	{
		enum Enum
		{
			eX = 0,	//!< motion along the X axis
			eY = 1,	//!< motion along the Y axis
			eZ = 2,	//!< motion along the Z axis
			eTWIST = 3,	//!< motion around the X axis
			eSWING1 = 4,	//!< motion around the Y axis
			eSWING2 = 5,	//!< motion around the Z axis
			eCOUNT = 6
		};
	};



	class PxD6Joint // Size: 0x0008
	{
	public:

		virtual void release();
		virtual void getConcreteTypeName();
		virtual void Function2();
		virtual void Function3();
		virtual void Function4();
		virtual void setActors();
		virtual void getActors();
		virtual void setLocalPose();
		virtual void getLocalPose();
		virtual void getRelativeTransform();
		virtual void getRelativeLinearVelocity();
		virtual void getRelativeAngularVelocity();
		virtual void setBreakForce();
		virtual void getBreakForce();
		virtual void setConstraintFlags();
		virtual void setConstraintFlag();
		virtual void getConstraintFlags();
		virtual void setInvMassScale0();
		virtual void getInvMassScale0();
		virtual void setInvInertiaScale0();
		virtual void getInvInertiaScale0();
		virtual void setInvMassScale1();
		virtual void getInvMassScale1();
		virtual void setInvInertiaScale1();
		virtual void getInvInertiaScale1();
		virtual void getConstraint();
		virtual void SetName(char* name);
		virtual char* GetName();
		virtual void Function28();
		virtual void SetMotion(PxD6Axis::Enum axis, PxD6Motion::Enum motion);
		virtual PxD6Motion::Enum GetMotion(PxD6Axis::Enum axis);
		virtual void Function31();
		virtual void Function32();
		virtual void Function33();
		virtual void Function34();
		virtual void Function35();
		virtual void Function36();
		virtual void Function37();
		virtual void Function38();
		virtual void Function39();
		virtual void Function40();
		virtual void Function41();
		virtual void Function42();
		virtual void Function43();
		virtual void Function44();
		virtual void Function45();
		virtual void Function46();
		virtual void Function47();
		virtual void Function48();
		virtual void Function49();
		virtual void Function50();
		virtual void Function51();
		virtual void Function52();
	};
	static_assert(sizeof(PxD6Joint) == 0x8);


	class physics_joint_physx_t // Size: 0x0488
	{
	public:
		void* vtable; //0x0000 (0x08)
		class physics_actor_t* physics_actor; //0x0008 (0x08)
		joint_type::Enum joint_type; //0x0010 (0x04)
		char pad_0014[4]; //0x0014 (0x04)
		PxD6Joint* physx_joint; //0x0018 (0x08)
		char pad_0020[4]; //0x0020 (0x04)
		float N0001BA48; //0x0024 (0x04)
		float N0001BA2B; //0x0028 (0x04)
		float N0001BA4B; //0x002C (0x04)
		float N0001BA2C; //0x0030 (0x04)
		float N0001BA4D; //0x0034 (0x04)
		float N0001BA2D; //0x0038 (0x04)
		float N0001BA4F; //0x003C (0x04)
		float N0001BA2E; //0x0040 (0x04)
		float N0001BA51; //0x0044 (0x04)
		float N0001BA2F; //0x0048 (0x04)
		float N0001BA53; //0x004C (0x04)
		float N0001BA30; //0x0050 (0x04)
		float N0001BA55; //0x0054 (0x04)
		float N0001BA31; //0x0058 (0x04)
		float N0001BA57; //0x005C (0x04)
		char pad_0060[1064]; //0x0060 (0x428)
	};
	static_assert(sizeof(physics_joint_physx_t) == 0x488);
};
#pragma pack(pop)