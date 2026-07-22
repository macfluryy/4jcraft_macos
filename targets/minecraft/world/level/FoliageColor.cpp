#include "FoliageColor.h"

#include "app/common/App_enums.h"
#include "app/common/src/Colours/ColourTable.h"
#include "minecraft/client/Minecraft.h"




















int FoliageColor::getEvergreenColor() {
    return Minecraft::GetInstance()->getColourTable()->getColor(
        eMinecraftColour_Foliage_Evergreen);
    
}

int FoliageColor::getBirchColor() {
    return Minecraft::GetInstance()->getColourTable()->getColor(
        eMinecraftColour_Foliage_Birch);
    
}

int FoliageColor::getDefaultColor() {
    return Minecraft::GetInstance()->getColourTable()->getColor(
        eMinecraftColour_Foliage_Default);
    
}