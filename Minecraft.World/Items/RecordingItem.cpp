#include "../Platform/stdafx.h"
#include "../Headers/net.minecraft.world.entity.player.h"
#include "../Headers/net.minecraft.world.level.h"
#include "../Headers/net.minecraft.world.level.tile.h"
#include "../Headers/net.minecraft.world.item.h"
#include "../Headers/net.minecraft.world.h"
#include "ItemInstance.h"
#include "RecordingItem.h"
#include "../Stats/GenericStats.h"

std::unordered_map<std::wstring, RecordingItem*> RecordingItem::BY_NAME;

RecordingItem::RecordingItem(int id, const std::wstring& recording)
    : Item(id), recording(recording) {
    this->maxStackSize = 1;
    BY_NAME[recording] = this;
}

Icon* RecordingItem::getIcon(int auxValue) { return icon; }

bool RecordingItem::useOn(std::shared_ptr<ItemInstance> itemInstance,
                          std::shared_ptr<Player> player, Level* level, int x,
                          int y, int z, int face, float clickX, float clickY,
                          float clickZ, bool bTestUseOnOnly) {
    // 4J-PB - Adding a test only version to allow tooltips to be displayed
    if (level->getTile(x, y, z) == Tile::jukebox_Id &&
        level->getData(x, y, z) == 0) {
        if (!bTestUseOnOnly) {
            if (level->isClientSide) return true;

            ((JukeboxTile*)Tile::jukebox)
                ->setRecord(level, x, y, z, itemInstance);
            level->levelEvent(nullptr, LevelEvent::SOUND_PLAY_RECORDING, x, y,
                              z, id);
            itemInstance->count--;

            player->awardStat(GenericStats::musicToMyEars(),
                              GenericStats::param_musicToMyEars(id));
        }
        return true;
    }
    return false;
}

void RecordingItem::appendHoverText(std::shared_ptr<ItemInstance> itemInstance,
                                    std::shared_ptr<Player> player,
                                    std::vector<HtmlString>* lines,
                                    bool advanced) {
    eMinecraftColour color = getRarity(std::shared_ptr<ItemInstance>())->color;

    wchar_t text[256];
    swprintf(text, 256, L"%ls %ls", L"C418 -", recording.c_str());

    lines->push_back(HtmlString(text, color));
}

const Rarity* RecordingItem::getRarity(
    std::shared_ptr<ItemInstance> itemInstance) {
    return (Rarity*)Rarity::rare;
}

void RecordingItem::registerIcons(IconRegister* iconRegister) {
    icon = iconRegister->registerIcon(L"record_" + recording);
}

RecordingItem* RecordingItem::getByName(const std::wstring& name) {
    AUTO_VAR(it, BY_NAME.find(name));
    if (it != BY_NAME.end()) {
        return it->second;
    } else {
        return NULL;
    }
}