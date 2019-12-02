#pragma once

#include "../Actor.h"
#include "Engine/Math/sVector.h"
namespace eae6320 {
	namespace UserInput {
		struct AdvancedUserInput;
	}
	class PlayerActor : public Actor {
	public:
		PlayerActor();
		float MoveSpeed;
		/** Bind input functions by passing user input*/
		void InitalizePlayerInput(UserInput::AdvancedUserInput* i_userInput);
	protected:
		void MoveForward(float i_value);
		void MoveRight(float i_value);
		void ShootUp(float i_value);
		void ShootRight(float i_value);

	private:
		Math::sVector m_aimDirection;
		bool m_inputBools[2];
	};
}


