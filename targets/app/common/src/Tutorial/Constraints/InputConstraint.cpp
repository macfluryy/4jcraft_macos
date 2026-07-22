#include "InputConstraint.h"

#include "platform/InputActions.h"
#include "platform/sdl2/Input.h"

bool InputConstraint::isMappingConstrained(int iPad, int mapping) {
    
    if ((m_inputMapping == mapping) || (mapping < ACTION_MAX_MENU)) {
        return true;
    }

    
    unsigned char layoutMapping = InputManager.GetJoypadMapVal(iPad);

    
    
    
    return (InputManager.GetGameJoypadMaps(layoutMapping, m_inputMapping) &
            InputManager.GetGameJoypadMaps(layoutMapping, mapping)) > 0;
}
