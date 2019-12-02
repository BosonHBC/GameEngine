#pragma once


#include <Tools/AssetBuildLibrary/cbBuilder.h>
#include <Engine/Graphics/Configuration.h>
#include <Engine/PhysicsSystem/PhysicsSystem.h>
#include <External/Lua/Includes.h>
namespace PlutoShe
{
	namespace Assets
	{

		class ColliderBuilder : public eae6320::Assets::cbBuilder
		{
		private:
			virtual eae6320::cResult Build(const std::vector<std::string>& i_arguments) override;
			virtual eae6320::cResult InitData(std::string i_path, PlutoShe::Physics::Collider& t_collider);
			eae6320::cResult LoadVerticesFromLua(lua_State& io_luaState, PlutoShe::Physics::Collider& t_collider);
		};

	}
}