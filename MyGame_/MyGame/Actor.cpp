#include "Actor.h"
namespace eae6320 {

	Actor::Actor(const Actor& other)
	{
		m_effect = other.m_effect;
		m_geometry = other.m_geometry;
		m_rigidbody = other.m_rigidbody;
		m_bVisible = other.m_bVisible;
	}

	Actor::Actor(eae6320::Graphics::cGeometry* i_geo, Graphics::cEffect* i_eff)
		:m_geometry (i_geo), m_effect(i_eff), m_bVisible(true)
	{
		m_geometry = nullptr;
	}

	Actor::Actor(eae6320::Graphics::cGeometry* i_geo, Graphics::cEffect* i_eff, Math::sVector i_initialLocation, Math::cQuaternion i_initialQuaternion)
		: Object(i_initialLocation, i_initialQuaternion), m_geometry(i_geo), m_effect(i_eff), m_bVisible(true)
	{
	}

	void Actor::SetGeometry(eae6320::Graphics::cGeometry* newGeo)
	{
		m_geometry = newGeo;
	}

	void Actor::SetEffect(eae6320::Graphics::cEffect* newEffect)
	{
		m_effect = newEffect;
	}

	Actor::~Actor()
	{
		m_geometry = nullptr;
		m_effect = nullptr;
	}
	void Actor::operator=(const Actor& other)
	{
		this->m_effect = other.m_effect;
		this->m_geometry = other.m_geometry;
		this->m_rigidbody = other.m_rigidbody;
	}

}
