#include "4J_Input.h"

C_4JInput InputManager;

void C_4JInput::Initialise(int iInputStateC, unsigned char ucMapC, unsigned char ucActionC, unsigned char ucMenuActionC) {}
void C_4JInput::Tick(void) {}
void C_4JInput::SetDeadzoneAndMovementRange(unsigned int uiDeadzone, unsigned int uiMovementRangeMax) {}
void C_4JInput::SetGameJoypadMaps(unsigned char ucMap, unsigned char ucAction, unsigned int uiActionVal) {}
unsigned int C_4JInput::GetGameJoypadMaps(unsigned char ucMap, unsigned char ucAction) { return 0; }
void C_4JInput::SetJoypadMapVal(int iPad, unsigned char ucMap) {}
unsigned char C_4JInput::GetJoypadMapVal(int iPad) { return 0; }
void C_4JInput::SetJoypadSensitivity(int iPad, float fSensitivity) {}
unsigned int C_4JInput::GetValue(int iPad, unsigned char ucAction, bool bRepeat) { return 0; }
bool C_4JInput::ButtonPressed(int iPad, unsigned char ucAction) { return false; }
bool C_4JInput::ButtonReleased(int iPad, unsigned char ucAction) { return false; }
bool C_4JInput::ButtonDown(int iPad, unsigned char ucAction) { return false; }
void C_4JInput::SetJoypadStickAxisMap(int iPad, unsigned int uiFrom, unsigned int uiTo) {}
void C_4JInput::SetJoypadStickTriggerMap(int iPad, unsigned int uiFrom, unsigned int uiTo) {}
void C_4JInput::SetKeyRepeatRate(float fRepeatDelaySecs, float fRepeatRateSecs) {}
void C_4JInput::SetDebugSequence(const char *chSequenceA, int(*Func)(LPVOID), LPVOID lpParam) {}
FLOAT C_4JInput::GetIdleSeconds(int iPad) { return 0.0f; }
bool C_4JInput::IsPadConnected(int iPad) { return iPad == 0; }
float C_4JInput::GetJoypadStick_LX(int iPad, bool bCheckMenuDisplay) { return 0.0f; }
float C_4JInput::GetJoypadStick_LY(int iPad, bool bCheckMenuDisplay) { return 0.0f; }
float C_4JInput::GetJoypadStick_RX(int iPad, bool bCheckMenuDisplay) { return 0.0f; }
float C_4JInput::GetJoypadStick_RY(int iPad, bool bCheckMenuDisplay) { return 0.0f; }
unsigned char C_4JInput::GetJoypadLTrigger(int iPad, bool bCheckMenuDisplay) { return 0; }
unsigned char C_4JInput::GetJoypadRTrigger(int iPad, bool bCheckMenuDisplay) { return 0; }
void C_4JInput::SetMenuDisplayed(int iPad, bool bVal) {}
EKeyboardResult C_4JInput::RequestKeyboard(LPCWSTR Title, LPCWSTR Text, DWORD dwPad, UINT uiMaxChars, int(*Func)(LPVOID, const bool), LPVOID lpParam, C_4JInput::EKeyboardMode eMode) { return EKeyboard_Cancelled; }
void C_4JInput::GetText(uint16_t *UTF16String) { if (UTF16String) UTF16String[0] = 0; }
bool C_4JInput::VerifyStrings(WCHAR **pwStringA, int iStringC, int(*Func)(LPVOID, STRING_VERIFY_RESPONSE *), LPVOID lpParam) { return true; }
void C_4JInput::CancelQueuedVerifyStrings(int(*Func)(LPVOID, STRING_VERIFY_RESPONSE *), LPVOID lpParam) {}
void C_4JInput::CancelAllVerifyInProgress(void) {}
