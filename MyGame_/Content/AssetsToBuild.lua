--[[
	This file lists every asset that must be built by the AssetBuildSystem
]]
--[[
		{ path = "Shaders/Vertex/standard.shader", arguments = { "vertex" } },
		{ path = "Shaders/Fragment/standard.shader", arguments = { "fragment" } },
		{ path = "Shaders/Fragment/animatedColor.shader", arguments = { "fragment" } }
]]
return
{
	shaders = {
		{ path = "Shaders/Vertex/vertexInputLayout_3dObject.shader", arguments = { "vertex" } },
	},
	geometries = {
		{ path = "Geometries/plane.hbc" },
		{ path = "Geometries/cone.hbc" },
		{ path = "Geometries/octopus.hbc" },
		{ path = "Geometries/player.hbc" },
		{ path = "Geometries/bullet.hbc" }
	},
	effects = {
		{ path = "Effects/standard.efx" },
		{ path = "Effects/animated.efx" },
		{ path = "Effects/bullet.efx" },
	},
	inputs = {
		{ path = "Inputs/defaultInput.input" }
	},
	colliders = {
		{ path = "Colliders/BoxCollider.col" }
	}
}
