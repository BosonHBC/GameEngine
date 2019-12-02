#pragma once
#include "Engine/Physics/sRigidBodyState.h"
#include "Engine/PhysicsSystem/PhysicsSystem.h"
#include "Engine/Math/cMatrix_transformation.h"

namespace eae6320 {
	// Object is a base type for all objects that has transformation
	class Object {

	public:
		Object() {};
		Object(Math::sVector i_initialLocation, Math::cQuaternion i_initialQuaternion);
		~Object() {};

	 virtual	void AddForce(Math::sVector i_force);
	 virtual	void ApplyResistance();

	 virtual void Update(const float i_secondCountToIntegrate);
	 void PredictTransformation(const float i_secondCountToExtrapolate);
	 void SetInitialTransform(Math::sVector i_initialLocation, Math::cQuaternion i_initialQuaternion);

	 Math::cMatrix_transformation GetLocalToWorldMatrix() const { return m_transform_localToWorld; }

	protected:
		Math::cMatrix_transformation m_transform_localToWorld;
		Physics::sRigidBodyState m_rigidbody;
		bool m_addingForce;
		PlutoShe::Physics::Collider m_BoxCollider;
	};
}
