#pragma once
#include "Object.h"

namespace eae6320 {
	namespace Math {
		class cMatrix_transformation;
	}
	namespace Graphics {
		class cGeometry;
		class cEffect;
	}

	class Actor : public Object {
	public:
		// default constructor
		Actor() :m_geometry(nullptr), m_effect(nullptr), m_bVisible(true) {}
		// constructor without transformation information
		Actor(Graphics::cGeometry* i_geo, Graphics::cEffect* i_eff);
		// constructor with transformation information
		Actor(Graphics::cGeometry* i_geo, Graphics::cEffect* i_eff,
			Math::sVector i_initialLocation, Math::cQuaternion i_initialQuaternion);
			
		~Actor();
		Actor(const Actor& other);
		void operator = (const Actor& other);

		void SetGeometry(Graphics::cGeometry* newGeo);
		void SetEffect(Graphics::cEffect* newEffect);
		
		Graphics::cGeometry* GetGeometry()const { return m_geometry; }
		Graphics::cEffect* GetEffect()const { return m_effect; }

		void SetVisibility(bool i_visible) { m_bVisible = i_visible; }
		bool GetVisibility() const { return m_bVisible; }
	protected:
		Graphics::cGeometry* m_geometry;
		Graphics::cEffect* m_effect;
		bool m_bVisible;
	private:

	};
}