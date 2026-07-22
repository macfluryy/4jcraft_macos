#include "Achievement.h"

#include <vector>

#include "Achievements.h"
#include "DescFormatter.h"
#include "minecraft/locale/I18n.h"
#include "minecraft/stats/Stat.h"
#include "minecraft/world/item/ItemInstance.h"

class Item;
class Tile;



















void Achievement::_init() {
    isGoldenVar = false;

    if (x < Achievements::xMin) Achievements::xMin = x;
    if (y < Achievements::yMin) Achievements::yMin = y;
    if (x > Achievements::xMax) Achievements::xMax = x;
    if (y > Achievements::yMax) Achievements::yMax = y;
}











Achievement::Achievement(int id, const std::wstring& name, int x, int y,
                         Item* icon, Achievement* prerequisite)
    : Stat(Achievements::ACHIEVEMENT_OFFSET + id,
           I18n::get(std::wstring(L"achievement.").append(name))),
      desc(I18n::get(
          std::wstring(L"achievement.").append(name).append(L".desc"))),
      icon(new ItemInstance(icon)),
      x(x),
      y(y),
      prerequisite(prerequisite) {}

Achievement::Achievement(int id, const std::wstring& name, int x, int y,
                         Tile* icon, Achievement* prerequisite)
    : Stat(Achievements::ACHIEVEMENT_OFFSET + id,
           I18n::get(std::wstring(L"achievement.").append(name))),
      desc(I18n::get(
          std::wstring(L"achievement.").append(name).append(L".desc"))),
      icon(new ItemInstance(icon)),
      x(x),
      y(y),
      prerequisite(prerequisite) {}

Achievement::Achievement(int id, const std::wstring& name, int x, int y,
                         std::shared_ptr<ItemInstance> icon,
                         Achievement* prerequisite)
    : Stat(Achievements::ACHIEVEMENT_OFFSET + id,
           I18n::get(std::wstring(L"achievement.").append(name))),
      desc(I18n::get(
          std::wstring(L"achievement.").append(name).append(L".desc"))),
      icon(icon),
      x(x),
      y(y),
      prerequisite(prerequisite) {}





Achievement* Achievement::setAwardLocallyOnly() {
    awardLocallyOnly = true;
    return this;
}









Achievement* Achievement::setGolden() {
    isGoldenVar = true;
    return this;
}





Achievement* Achievement::postConstruct() {
    Stat::postConstruct();

    Achievements::achievements->push_back(this);

    return this;
}






bool Achievement::isAchievement() { return true; }





std::wstring Achievement::getDescription() {
    if (descFormatter != nullptr) {
        return descFormatter->format(desc);
    }
    return desc;
}

Achievement* Achievement::setDescFormatter(DescFormatter* descFormatter) {
    this->descFormatter = descFormatter;
    return this;
}

bool Achievement::isGolden() { return isGoldenVar; }

int Achievement::getAchievementID() {
    return id - Achievements::ACHIEVEMENT_OFFSET;
}
