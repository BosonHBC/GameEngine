#include "PlayerActor.h"
#include "Engine/AdvancedUserInput/AdvancedUserInput.h"

#include "Windows.h"
namespace eae6320 {

	PlayerActor::PlayerActor()
	{
		MoveSpeed = 1;
	}

	void PlayerActor::InitalizePlayerInput(UserInput::AdvancedUserInput* i_userInput)
	{
		i_userInput->BindAxis("MoveRight", this, &PlayerActor::MoveRight);
		i_userInput->BindAxis("MoveForward", this, &PlayerActor::MoveForward);

		i_userInput->BindAxis("ShootUp", this, &PlayerActor::ShootUp);
		i_userInput->BindAxis("ShootRight", this, &PlayerActor::ShootRight);
	}


	void PlayerActor::MoveForward(float i_value)
	{
		if (i_value != 0) {
			AddForce(Math::sVector::WorldForward * i_value * MoveSpeed);
			m_inputBools[0] = true;
		}
		else {
			m_inputBools[0] = false;
				m_addingForce = (m_inputBools[0] || m_inputBools[1]);
		}
	}

	void PlayerActor::MoveRight(float i_value)
	{
		if (i_value != 0) {
			m_inputBools[1] = true;
			AddForce(Math::sVector::WorldRight * i_value * MoveSpeed);
		}
		else {
			m_inputBools[1] = false;
			m_addingForce = (m_inputBools[0] || m_inputBools[1]);
		}
	}

	void PlayerActor::ShootUp(float i_value)
	{
		m_aimDirection = Math::sVector::WorldForward * i_value;
	}

	void PlayerActor::ShootRight(float i_value)
	{
		m_aimDirection = Math::sVector::WorldRight * i_value;

	}

}


