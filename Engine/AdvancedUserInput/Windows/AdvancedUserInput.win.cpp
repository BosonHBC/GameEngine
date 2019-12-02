#include "AdvancedUserInput.h"
#include <Engine/Windows/Includes.h>
#include "Engine/UserInput/UserInput.h"

namespace eae6320 {
	namespace UserInput {
		bool AdvancedUserInput::IsKeyDown(const KeyCodes::eKeyCodes& i_key) const
		{
			// 1111 1111 1111 1110
			const short isKeyDownMask = ~1;
			// Reference: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate
			auto keyState = GetAsyncKeyState(i_key);
			// if the high-order bit is 1, it is down
			bool isKeyDown = ((keyState & isKeyDownMask) != 0);
			return isKeyDown;
		}
	}
}