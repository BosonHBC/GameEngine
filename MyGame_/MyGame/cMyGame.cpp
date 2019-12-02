// Includes
//=========

#include "cMyGame.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/UserInput/UserInput.h>
#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/cGeometry.h"
#include "Engine/Graphics/cEffect.h"
#include "Engine/AdvancedUserInput/AdvancedUserInput.h"
#include "Actor.h"
#include "Gameplay/PlayerActor.h"
#include "Camera.h"
#include <string>

#include "Windows.h"

// Inherited Implementation
//=========================

// Run
//----
namespace {
	// Shading Data
	//-------------
	eae6320::Graphics::cEffect::Handle s_StandardEffect;
	eae6320::Graphics::cEffect::Handle s_AnimatedEffect;
	// Geometry Data
	//--------------
	eae6320::Graphics::cGeometry::Handle s_planeGeo;
	eae6320::Graphics::cGeometry::Handle s_playerGeo;
	eae6320::Graphics::cGeometry::Handle s_MyOctopus;

	// Camera Data
	eae6320::Camera cam1(45.f, 1.f, 0.1f, 100.f, eae6320::Math::sVector(0, 10, 6.f), eae6320::Math::cQuaternion(-45.f, eae6320::Math::sVector(1.f, 0.f, 0.f)));

	// Input data
	eae6320::UserInput::AdvancedUserInput::Handle s_UserInput;
}
void eae6320::cMyGame::UpdateBasedOnInput()
{
	UserInput::AdvancedUserInput::s_manager.Get(s_UserInput)->UpdateInput();

	// Is the user pressing the ESC key?
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::Escape))
	{
		// Exit the application
		const auto result = Exit(EXIT_SUCCESS);
		EAE6320_ASSERT(result);
	}

/*
	// Change the background color
	{
		if (UserInput::IsKeyPressed(UserInput::KeyCodes::Enter))
		{
			// Swap back color
			SetBackColor(0.53f, 0.13f, 0.13f, 1);
		}
		else {
			SetBackColor(0.4f, 0.4f, 0.4f, 1);
		}
	}*/
/*
	// Show the second object or not
	{
		if (UserInput::IsKeyPressed(UserInput::KeyCodes::Space)) {
			if (!s_IsSpacePressed) {
				s_IsSpacePressed = true;
				m_initial_ActorList[1]->SetGeometry(eae6320::Graphics::cGeometry::s_manager.Get(s_MyGeo1));
			}
		}
		else {
			if (s_IsSpacePressed) {
				s_IsSpacePressed = false;
				m_initial_ActorList[1]->SetGeometry(eae6320::Graphics::cGeometry::s_manager.Get(s_playerGeo));
			}
		}
	}*/
	/*
		// First Object Movement
		{
			if (UserInput::IsKeyPressed(UserInput::KeyCodes::A))
			{
				m_initial_ActorList[1]->AddForce(-WorldRight * movingSpeed);
			}
			if (UserInput::IsKeyPressed(UserInput::KeyCodes::D))
			{
				m_initial_ActorList[1]->AddForce(WorldRight * movingSpeed);
			}
			if (UserInput::IsKeyPressed(UserInput::KeyCodes::W))
			{
				m_initial_ActorList[1]->AddForce(WorldUp * movingSpeed);
			}
			if (UserInput::IsKeyPressed(UserInput::KeyCodes::S))
			{
				m_initial_ActorList[1]->AddForce(-WorldUp * movingSpeed);
			}
			if (!UserInput::IsKeyPressed(UserInput::KeyCodes::A) && !UserInput::IsKeyPressed(UserInput::KeyCodes::D) && !UserInput::IsKeyPressed(UserInput::KeyCodes::W) && !UserInput::IsKeyPressed(UserInput::KeyCodes::S)) {
				m_initial_ActorList[1]->ApplyResistance();
			}
		}*/
	/*	// Camera Object Movement
	{
		if (UserInput::IsKeyPressed(UserInput::KeyCodes::Left))
		{
			m_mainCamera->AddForce(-WorldRight * movingSpeed);
		}
		if (UserInput::IsKeyPressed(UserInput::KeyCodes::Right))
		{
			m_mainCamera->AddForce(WorldRight * movingSpeed);
		}
		if (UserInput::IsKeyPressed(UserInput::KeyCodes::Up))
		{
			m_mainCamera->AddForce(WorldUp * movingSpeed);
		}
		if (UserInput::IsKeyPressed(UserInput::KeyCodes::Down))
		{
			m_mainCamera->AddForce(-WorldUp * movingSpeed);
		}
		if (UserInput::IsKeyPressed(UserInput::KeyCodes::Shift))
		{
			m_initial_ActorList[1]->AddForce(WorldForward * movingSpeed);
		}
		if (UserInput::IsKeyPressed(UserInput::KeyCodes::Control))
		{
			m_initial_ActorList[1]->AddForce(-WorldForward * movingSpeed);
		}
		if (!UserInput::IsKeyPressed(UserInput::KeyCodes::Left) && !UserInput::IsKeyPressed(UserInput::KeyCodes::Right) && !UserInput::IsKeyPressed(UserInput::KeyCodes::Up) && !UserInput::IsKeyPressed(UserInput::KeyCodes::Down) && !UserInput::IsKeyPressed(UserInput::KeyCodes::Shift) && !UserInput::IsKeyPressed(UserInput::KeyCodes::Control)) {
			m_mainCamera->ApplyResistance();
		}
	}*/
}



void eae6320::cMyGame::UpdateBasedOnTime(const float i_elapsedSecondCount_sinceLastUpdate)
{

	// Check the visibility of the actor, if it is invisible, doesn't require to render
	{
		// Clear data list
		m_renderList = std::vector<std::pair<Graphics::cGeometry*, Graphics::cEffect*>>();
		m_actorList = std::vector<Actor*>();
		// Check status and push required data
		for (uint16_t i = 0; i < m_initial_ActorList.size(); ++i)
		{
			if (m_initial_ActorList[i]->GetVisibility()) {
				m_actorList.push_back(m_initial_ActorList[i]);
				m_initial_RenderList[i].first = m_actorList[i]->GetGeometry();
				m_initial_RenderList[i].second = m_actorList[i]->GetEffect();
				m_renderList.push_back(m_initial_RenderList[i]);
			}
		}
	}

	// Update and predict actor location, rotation
	for (auto it = m_actorList.begin(); it != m_actorList.end(); ++it)
	{
		(*it)->Update(i_elapsedSecondCount_sinceLastUpdate);
		(*it)->PredictTransformation(i_elapsedSecondCount_sinceLastUpdate);
	}
	// Update and predict camera location, rotation
	m_mainCamera->Update(i_elapsedSecondCount_sinceLastUpdate);
	m_mainCamera->PredictTransformation(i_elapsedSecondCount_sinceLastUpdate);
	m_mainCamera->UpdateMatrices();
}

// Initialization / Clean Up
//--------------------------
eae6320::cResult eae6320::cMyGame::Initialize()
{
	auto result = Results::Success;
	// Load Geometry
	{
		if (!(result = eae6320::Graphics::cGeometry::s_manager.Load("data/geometries/plane.hbc", s_planeGeo)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Actor without Geometry data");
			return result;
		}
		if (!(result = eae6320::Graphics::cGeometry::s_manager.Load("data/geometries/player.hbc", s_playerGeo)))
		{
			EAE6320_ASSERTF(false, "Can't initialize Actor without Geometry data");
			return result;
		}
	}
	// Load Shading data
	if (!(result = eae6320::Graphics::cEffect::s_manager.Load("data/effects/bullet.efx", s_StandardEffect)))
	{
		EAE6320_ASSERTF(false, "Can't initialize Actor without Effect data");
		return result;
	}
	if (!(result = eae6320::Graphics::cEffect::s_manager.Load("data/effects/animated.efx", s_AnimatedEffect)))
	{
		EAE6320_ASSERTF(false, "Can't initialize Actor without Effect data");
		return result;
	}

	// Load Input
	if (!(result = eae6320::UserInput::AdvancedUserInput::s_manager.Load("data/inputs/defaultinput.input", s_UserInput))) {
		EAE6320_ASSERTF(false, "Can't initialize the input data data");
		return result;
	}
	/*

		// bind action with static / global function
		UserInput::AdvancedUserInput::s_manager.Get(s_UserInput)->BindAction("Jump", IT_OnReleased, OnReleaseEvent);

		UserInput::AdvancedUserInput::s_manager.Get(s_UserInput)->BindAction("Jump", IT_OnHold, OnHoldEvent);
		// bind action with member function
		UserInput::AdvancedUserInput::s_manager.Get(s_UserInput)->BindAction("Interact", IT_OnPressed, &testA, &TestA::MemberFunction);
		// bind axis with static / global function
		UserInput::AdvancedUserInput::s_manager.Get(s_UserInput)->BindAxis("MoveRight", MoveRight);
		// bind axis with member function
		UserInput::AdvancedUserInput::s_manager.Get(s_UserInput)->BindAxis("MoveForward", &testA, &TestA::MoveForward);

		UserInput::AdvancedUserInput::s_manager.Get(s_UserInput)->BindAxis("MoveUp", &testA, &TestA::MoveUp);
	*/

	// Initialize main camera
	SwitchCamera(&cam1);
	// Initialize Actors
	m_plane = new Actor();
	InitializeActorWithGraphicAndTransformationInfo(m_plane,*eae6320::Graphics::cGeometry::s_manager.Get(s_planeGeo), *eae6320::Graphics::cEffect::s_manager.Get(s_StandardEffect), eae6320::Math::sVector(0, 0, 0), eae6320::Math::cQuaternion());
	m_playerActor = new PlayerActor();
	m_playerActor->MoveSpeed = 5.f;
	m_playerActor->InitalizePlayerInput(UserInput::AdvancedUserInput::s_manager.Get(s_UserInput));
	InitializeActorWithGraphicAndTransformationInfo(m_playerActor, *eae6320::Graphics::cGeometry::s_manager.Get(s_playerGeo), *eae6320::Graphics::cEffect::s_manager.Get(s_AnimatedEffect), eae6320::Math::sVector(0, 1, 0), eae6320::Math::cQuaternion());
	//InitializeActorWithGraphicAndTransformationInfo(*eae6320::Graphics::cGeometry::s_manager.Get(s_MyOctopus), *eae6320::Graphics::cEffect::s_manager.Get(s_StandardEffect), eae6320::Math::sVector(0, 0, 0), eae6320::Math::cQuaternion());
	m_renderList = m_initial_RenderList;
	m_actorList = m_initial_ActorList;

	return result;
}

eae6320::cResult eae6320::cMyGame::CleanUp()
{
	using namespace Graphics;
	cGeometry* tempGeo = cGeometry::s_manager.Get(s_planeGeo);
	cGeometry::RemoveGeometry(tempGeo);
	tempGeo = cGeometry::s_manager.Get(s_playerGeo);
	cGeometry::RemoveGeometry(tempGeo);

	cGeometry::s_manager.Release(s_planeGeo);
	cGeometry::s_manager.Release(s_playerGeo);

	cEffect* tempEffect = cEffect::s_manager.Get(s_StandardEffect);
	cEffect::RemoveEffect(tempEffect);
	tempEffect = cEffect::s_manager.Get(s_AnimatedEffect);
	cEffect::RemoveEffect(tempEffect);
	cEffect::s_manager.Release(s_StandardEffect);
	cEffect::s_manager.Release(s_AnimatedEffect);

	// Clean up user input
	UserInput::AdvancedUserInput* tempUserInput = UserInput::AdvancedUserInput::s_manager.Get(s_UserInput);
	tempUserInput->RemoveUserInput(tempUserInput);
	UserInput::AdvancedUserInput::s_manager.Release(s_UserInput);

	for (auto it = m_initial_ActorList.begin(); it != m_initial_ActorList.end(); ++it)
	{
		delete* it;
	}

	return Results::Success;
}

void eae6320::cMyGame::InitializeActorWithGraphicAndTransformationInfo(Actor* o_actor, Graphics::cGeometry& thisGeo, Graphics::cEffect& thisEffect, eae6320::Math::sVector i_initialLocation, eae6320::Math::cQuaternion i_initialQuaternion)
{
	o_actor->SetGeometry(&thisGeo);
	o_actor->SetEffect(&thisEffect);
	o_actor->SetInitialTransform(i_initialLocation, i_initialQuaternion);
	
	// Update Actor list and render list
	m_initial_ActorList.push_back(o_actor);
	m_initial_RenderList.push_back(std::pair<eae6320::Graphics::cGeometry*, eae6320::Graphics::cEffect*>(o_actor->GetGeometry(), o_actor->GetEffect()));
	eae6320::Graphics::CreateDataFromApplicationThread(m_initial_RenderList);
}

void eae6320::cMyGame::SubmitDataToBeRendered(const float i_elapsedSecondCount_systemTime, const float i_elapsedSecondCount_sinceLastSimulationUpdate)
{
	eae6320::Graphics::SubmitBackColorDataFromApplicationThread(m_backColor[0], m_backColor[1], m_backColor[2], m_backColor[3]);
	// Submit rendering list (list of pair<Geometry, Effect>)
	eae6320::Graphics::UpdateGeometriesFromApplicationThread(m_renderList);
	// Submit camera matrices after updating it
	eae6320::Graphics::SubmitFrameRequiredMatrices(m_mainCamera->GetWorldToCameraMatrix(), m_mainCamera->GetCameraToProjectedMatrix());
	// Transfer the data in my actor to a format that graphic engine will accept
	std::vector<eae6320::Math::cMatrix_transformation> drawcallMatrices;
	for (auto it = m_actorList.begin(); it != m_actorList.end(); ++it)
	{
		drawcallMatrices.push_back((*it)->GetLocalToWorldMatrix());
	}
	// Submit matrices that graphic requires to render object
	eae6320::Graphics::SubmitDrawCallRequiredMatrices(drawcallMatrices);
}

void eae6320::cMyGame::SetBackColor(float r, float g, float b, float a)
{
	m_backColor[0] = r;
	m_backColor[1] = g;
	m_backColor[2] = b;
	m_backColor[3] = a;
}

void eae6320::cMyGame::SwitchCamera(Camera* newCamera)
{
	m_mainCamera = newCamera;
}

