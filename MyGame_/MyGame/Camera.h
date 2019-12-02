#pragma once
#include "Object.h"

namespace eae6320 {
	class Camera : public Object {

	public:
		// default constructor
		Camera() : m_verticalFoV(0), m_asp(0), m_zNearPlane(0), m_zFarPlane(0) { }
		// constructor without transformation information
		Camera(const float i_verticalFieldOfView_inRadians,
			const float i_aspectRatio,
			const float i_z_nearPlane, const float i_z_farPlane) :
			m_verticalFoV(i_verticalFieldOfView_inRadians), m_asp(i_aspectRatio), m_zNearPlane(i_z_nearPlane), m_zFarPlane(m_zFarPlane) {
			UpdateMatrices();
		};
		// constructor with transformation information
		Camera(const float i_verticalFieldOfView_inRadians,
			const float i_aspectRatio,
			const float i_z_nearPlane, const float i_z_farPlane,
			Math::sVector i_initialLocation, Math::cQuaternion i_initialQuaternion) :
			Object(i_initialLocation, i_initialQuaternion),
			m_verticalFoV(i_verticalFieldOfView_inRadians), m_asp(i_aspectRatio), m_zNearPlane(i_z_nearPlane), m_zFarPlane(i_z_farPlane) {
			UpdateMatrices();
		};
		~Camera() { };
		void UpdateMatrices();

		Math::cMatrix_transformation GetWorldToCameraMatrix() const { return m_transform_worldToCamera; }
		Math::cMatrix_transformation GetCameraToProjectedMatrix() const { return m_transform_cameraToProjected; }

	protected:
		Math::cMatrix_transformation m_transform_worldToCamera;
		Math::cMatrix_transformation m_transform_cameraToProjected;

		float m_verticalFoV;
		float m_asp;
		float m_zNearPlane;
		float m_zFarPlane;
	};
}
