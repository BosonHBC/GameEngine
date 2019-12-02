#include "Camera.h"
namespace eae6320 {
	void Camera::UpdateMatrices()
	{
		m_transform_worldToCamera = Math::cMatrix_transformation::CreateWorldToCameraTransform(m_transform_localToWorld);
		m_transform_cameraToProjected = Math::cMatrix_transformation::CreateCameraToProjectedTransform_perspective(m_verticalFoV, m_asp, m_zNearPlane, m_zFarPlane);
	}
}
