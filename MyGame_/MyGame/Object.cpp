#include "Object.h"
#define RESISTANCE 5.f
#define StopVelocity_Sqr 0.01f
namespace eae6320 {

	Object::Object(Math::sVector i_initialLocation, Math::cQuaternion i_initialQuaternion)
	{
		m_rigidbody.position = i_initialLocation;
		m_rigidbody.orientation = i_initialQuaternion;
		m_addingForce = false;

		m_transform_localToWorld = Math::cMatrix_transformation(i_initialQuaternion, i_initialLocation);
	}

	void Object::AddForce(Math::sVector i_force)
	{
		m_rigidbody.acceleration = i_force;
		m_addingForce = true;
	}

	void Object::ApplyResistance()
	{
		float velocitySqr = m_rigidbody.velocity.GetLength_Sqr();
		// When player is not setting force directly and the velocity is larger than 0, apply resistance
		if (velocitySqr > 0 && !m_addingForce) {
			m_rigidbody.acceleration = -m_rigidbody.velocity * RESISTANCE;

			// When the velocity is going to stop, stop it
			if (m_rigidbody.velocity.GetLength_Sqr() < StopVelocity_Sqr) {
				m_rigidbody.velocity = Math::sVector();
				m_rigidbody.acceleration = Math::sVector();
			}
		}
	}

	void Object::Update(const float i_secondCountToIntegrate)
	{
		ApplyResistance();
		m_rigidbody.Update(i_secondCountToIntegrate);
	}

	void Object::PredictTransformation(const float i_secondCountToExtrapolate)
	{
		m_transform_localToWorld = m_rigidbody.PredictFutureTransform(i_secondCountToExtrapolate);
	}

	void Object::SetInitialTransform(Math::sVector i_initialLocation, Math::cQuaternion i_initialQuaternion)
	{
		m_rigidbody.position = i_initialLocation;
		m_rigidbody.orientation = i_initialQuaternion;
		m_transform_localToWorld = Math::cMatrix_transformation(i_initialQuaternion, i_initialLocation);
	}

}

