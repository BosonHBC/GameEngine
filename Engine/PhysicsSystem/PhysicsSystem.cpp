// PhysicsSystem.cpp : Defines the functions for the static library.
//

#include "PhysicsSystem.h"

namespace PlutoShe
{
	namespace Physics
	{
		Vector3 Collider::getFarthestPointInDirection(Vector3 i_dir)
		{
			int selection = 0;
			float maxDist = (m_transformation * m_vertices[0]).dot(i_dir);
			for (size_t i = 1; i < m_vertices.size(); i++)
			{
				float dist = (m_transformation * m_vertices[i]).dot(i_dir);
				if (dist > maxDist)
				{
					maxDist = dist;
					selection = (int)i;
				}
			}
			return m_transformation * m_vertices[selection];
		}

		Vector3 Collider::supportFunction(Collider&i_A, Collider&i_B, Vector3 i_dir)
		{
			auto a = i_A.getFarthestPointInDirection(i_dir);
			auto b = i_B.getFarthestPointInDirection(i_dir.Negate());
			return a - b;	
		}


		Vector3 Collider::Center()
		{
			Vector3 center;
			int count = 0;
			for (size_t i = 0; i < m_vertices.size(); i++)
			{
				center = center + m_transformation * m_vertices[i];
				count++;
			}
			return center / float(count);
		}

		bool Collider::IsCollided(Collider&i_B)
		{
			Vector3 dir = i_B.Center() - this->Center();
			Simplex simplex;
			simplex.Clear();
			while (true)
			{
				simplex.Add(supportFunction(*this, i_B, dir));

				if (simplex.GetLast().dot(dir) < 0) {
					return false;
				}
				else {
					if (simplex.ContainsOrigin(dir)) {
						return true;
					}
				}
			}
		}

		bool Simplex::ContainsOrigin(Vector3 &t_direction)
		{
			Vector3 a, b, c, d;
			Vector3 ab, ac, ao;
			switch (this->GetSize())
			{
			case 1:
				t_direction = t_direction * -1;
				break;
			case 2:
				a = this->GetA();
			    b = this->GetB();
				ab = b - a;
				ao = a.Negate();
				t_direction = ab.cross(ao).cross(ab);
				break;

			case 3:
				a = this->GetA();
				b = this->GetB();
				c = this->GetC();
				ab = b - a;
				ac = c - a;
				ao = a.Negate();
				t_direction = ac.cross(ab);
				if (t_direction.dot(ao) < 0) t_direction = t_direction * -1;
				break;
			case 4:
			    a = this->GetA();
				b = this->GetB();
				c = this->GetC();
				d = this->GetD();
				auto da = a - d;
				auto db = b - d;
				auto dc = c - d;
				auto normal_dab = da.cross(db);
				auto normal_dac = dc.cross(da);
				auto normal_dbc = db.cross(dc);
				auto do_ = d.Negate(); // (0,0,0) - point d
				auto ndab = normal_dab.dot(do_); 
				auto ndac = normal_dac.dot(do_);
				auto ndbc = normal_dbc.dot(do_);
				if (ndab > 0)
				{
					this->RemoveC();

					t_direction = normal_dab;
				}
				else if (ndac > 0)
				{
					this->m_points[1] = d;
					this->RemoveD();
					t_direction = normal_dac;
				}
				else if (ndbc > 0)
				{
					this->m_points[0] = d;
					this->RemoveD();
					t_direction = normal_dbc;
				}
				else
				{
					return true;
				}
				break;
			}
			return false;
		}

		eae6320::cResult PlutoShe::Physics::Collider::InitData(std::string i_path)
		{
			auto result = eae6320::Results::Success;
			std::string errorMessage;
			eae6320::Platform::sDataFromFile dataFromFile;

			auto resultReadBinaryFile = eae6320::Platform::LoadBinaryFile(i_path.c_str(), dataFromFile, &errorMessage);
			if (!resultReadBinaryFile)
			{
				result = resultReadBinaryFile;
				eae6320::Logging::OutputError("Couldn't read binary file at path %s", i_path.c_str());
				return result;
			}

			auto currentOffset = reinterpret_cast<uintptr_t>(dataFromFile.data);
			uint16_t vertexCount;
			size_t sizeS = 0;
			if (sizeS + sizeof(uint16_t) > dataFromFile.size)
			{
				eae6320::Logging::OutputError("Wrong file size at path %s", i_path.c_str());
				return result;
			}

			memcpy(&vertexCount, reinterpret_cast<void*>(currentOffset), sizeof(uint16_t));
			
			currentOffset += sizeof(uint16_t);
			sizeS += sizeof(uint16_t);
			if (sizeS + vertexCount * sizeof(Vector3) > dataFromFile.size)
			{
				eae6320::Logging::OutputError("Wrong file size at path %s", i_path.c_str());
				return result;
			}

			m_vertices.resize(vertexCount);
			memcpy(&m_vertices[0], reinterpret_cast<void*>(currentOffset), vertexCount * sizeof(Vector3));
			return result;
		}

		PlutoShe::Physics::Collider::Collider() { m_vertices.clear(); }
		PlutoShe::Physics::Collider::Collider(std::vector<Vector3>& i_v) { m_vertices = i_v; }
		PlutoShe::Physics::Collider::Collider(const Collider& i_v) { m_vertices = i_v.m_vertices; }
		PlutoShe::Physics::Collider::Collider(std::string i_path) { InitData(i_path); }

		void PlutoShe::Physics::Collider::UpdateTransformation(eae6320::Math::cMatrix_transformation i_t)
		{
			m_transformation = i_t;
		}

		PlutoShe::Physics::ColliderList::ColliderList() { m_colliders.clear(); }
		PlutoShe::Physics::ColliderList::ColliderList(const Collider& i_c) { m_colliders.clear(); m_colliders.push_back(i_c); }
		PlutoShe::Physics::ColliderList::ColliderList(const ColliderList& i_c) { m_colliders = i_c.m_colliders; }


		void PlutoShe::Physics::ColliderList::UpdateTransformation(eae6320::Math::cMatrix_transformation i_t)
		{
			for (size_t i = 0; i < m_colliders.size(); i++)
			{
				m_colliders[i].UpdateTransformation(i_t);
			}
		}


		bool PlutoShe::Physics::ColliderList::IsCollided(ColliderList& i_queryColliderList)
		{
			for (size_t i = 0; i < m_colliders.size(); i++)
			{
				for (size_t j = 0; j < i_queryColliderList.m_colliders.size(); j++)
				{
					if (m_colliders[i].IsCollided(i_queryColliderList.m_colliders[j]))
					{
						return true;
					}
				}
			}
			return false;
		}


		Collider PlutoShe::Physics::ColliderList::GetColliderByIndex(int i_index)
		{ 
			return m_colliders[i_index]; 
		}

		size_t PlutoShe::Physics::ColliderList::GetSize() { return m_colliders.size(); }

		void PlutoShe::Physics::ColliderList::AddCollider(Collider i_c) { m_colliders.push_back(i_c); }

		void PlutoShe::Physics::ColliderList::ClearAllCollider() { m_colliders.clear(); }
	}
}
