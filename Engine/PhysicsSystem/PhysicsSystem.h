#pragma once
#include <vector>
#include <Engine/Math/sVector.h>
#include <Engine/Math/cMatrix_transformation.h>
#include <Engine/Platform/Platform.h>
#include <Engine/Logging/Logging.h>
#include <string>

namespace PlutoShe
{
	namespace Physics
	{
		class Vector3
		{
		public:
			float m_x, m_y, m_z;

			Vector3() { m_x = m_y = m_z = 0; }
			Vector3(float x, float y, float z) { m_x = x; m_y = y; m_z = z; }
			Vector3(const Vector3& c) { this->Equal(c); }
			Vector3(const eae6320::Math::sVector& c) { m_x = c.x; m_y = c.y; m_z = c.z; }

			void operator = (const Vector3& c) { this->Equal(c); }
			void Equal(const Vector3& c) { m_x = c.m_x; m_y = c.m_y; m_z = c.m_z; }

			Vector3 operator + (const Vector3& i_v) { return Vector3(m_x + i_v.m_x, m_y + i_v.m_y, m_z + i_v.m_z); }
			Vector3 operator - (const Vector3& i_v) { return Vector3(m_x - i_v.m_x, m_y - i_v.m_y, m_z - i_v.m_z); }

			Vector3 operator + (const float i_v) { return Vector3(m_x + i_v, m_y + i_v, m_z + i_v); }
			Vector3 operator - (const float i_v) { return Vector3(m_x - i_v, m_y - i_v, m_z - i_v); }
			Vector3 operator * (const float i_v) { return Vector3(m_x * i_v, m_y * i_v, m_z * i_v); }
			Vector3 operator / (const float i_v) { return Vector3(m_x / i_v, m_y / i_v, m_z / i_v); }

			Vector3 cross(const Vector3& i_v) {
				return Vector3(m_y * i_v.m_z - m_z * i_v.m_y, -(m_x * i_v.m_z - m_z * i_v.m_x), m_x * i_v.m_y - m_y * i_v.m_x);
			}
			Vector3 Negate()
			{
				return *this * -1;
			}
			float dot(const Vector3& i_v)
			{
				return m_x * i_v.m_x + m_y * i_v.m_y + m_z * i_v.m_z;
			}

			friend Vector3 operator *(eae6320::Math::cMatrix_transformation &i_m, const Vector3 &i_rhs) 
			{
				return i_m * eae6320::Math::sVector(i_rhs.m_x, i_rhs.m_y, i_rhs.m_z);
			}

		};

		class Collider
		{
		public:
			Collider();
			Collider(std::vector<Vector3>& i_v);
			Collider(const Collider& i_v);
			Collider(std::string i_path);

			eae6320::cResult InitData(std::string i_path);;
			void UpdateTransformation(eae6320::Math::cMatrix_transformation i_t);
			Vector3 Center();
			bool IsCollided(Collider& i_B);
			
			std::vector<Vector3> m_vertices;
		private:
			static Vector3 supportFunction(Collider& i_A, Collider& i_B, Vector3 i_dir);
			Vector3 getFarthestPointInDirection(Vector3 i_dir);
			eae6320::Math::cMatrix_transformation m_transformation;

		};

		class Simplex
		{
		public:
			std::vector<Vector3> m_points;

			Simplex() { m_points.clear(); }
			size_t GetSize() { return m_points.size(); }
			void Clear() { m_points.clear(); }
			Vector3 GetA() { return m_points[0]; }
			Vector3 GetB() { return m_points[1]; }
			Vector3 GetC() { return m_points[2]; }
			Vector3 GetD() { return m_points[3]; }
			void RemoveA() { m_points.erase(m_points.begin()); }
			void RemoveB() { m_points.erase(m_points.begin() + 1); }
			void RemoveC() { m_points.erase(m_points.begin() + 2); }
			void RemoveD() { m_points.erase(m_points.begin() + 3); }
			void Add(Vector3 i_data) { m_points.push_back(i_data); }
			Vector3 GetLast() { return m_points[m_points.size() - 1]; }
			bool ContainsOrigin(Vector3& i_d);
		};

		class ColliderList
		{
		public:
			ColliderList();
			ColliderList(const Collider& i_c);
			ColliderList(const ColliderList& i_c);

			void ClearAllCollider();
			void AddCollider(Collider i_c);
			size_t GetSize();
			Collider GetColliderByIndex(int i_index);
			bool IsCollided(ColliderList& i_queryColliderList);
			void UpdateTransformation(eae6320::Math::cMatrix_transformation i_t);
		protected:
			std::vector<Collider> m_colliders;
		};

		
	}
}