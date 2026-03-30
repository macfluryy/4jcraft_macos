#include "../../../../../../Header Files/stdafx.h"
#include "EnchantmentTableTileEntity.h"
#include "../../../entity/player/net.minecraft.world.entity.player.h"
#include "../../net.minecraft.world.level.h"

EnchantmentTableEntity::EnchantmentTableEntity() {
    random = new Random();

    time = 0;
    flip = 0.0f;
    oFlip = 0.0f;
    flipT = 0.0f;
    flipA = 0.0f;
    open = 0.0f;
    oOpen = 0.0f;
    rot = 0.0f;
    oRot = 0.0f;
    tRot = 0.0f;
    name = L"";
}

EnchantmentTableEntity::~EnchantmentTableEntity() { delete random; }

void EnchantmentTableEntity::save(CompoundTag* base) {
    TileEntity::save(base);
    if (hasCustomName()) base->putString(L"CustomName", name);
}

void EnchantmentTableEntity::load(CompoundTag* base) {
    TileEntity::load(base);
    if (base->contains(L"CustomName")) name = base->getString(L"CustomName");
}

void EnchantmentTableEntity::tick() {
    TileEntity::tick();
    oOpen = open;
    oRot = rot;

    std::shared_ptr<Player> player =
        level->getNearestPlayer(x + 0.5f, y + 0.5f, z + 0.5f, 3);
    if (player != nullptr) {
        double xd = player->x - (x + 0.5f);
        double zd = player->z - (z + 0.5f);

        tRot = (float)atan2(zd, xd);

        open += 0.1f;

        if (open < 0.5f || random->nextInt(40) == 0) {
            float old = flipT;
            do {
                flipT += random->nextInt(4) - random->nextInt(4);
            } while (old == flipT);
        }

    } else {
        tRot += 0.02f;
        open -= 0.1f;
    }

    while (rot >= M_PI) rot -= M_PI * 2;
    while (rot < -M_PI ) rot += M_PI * 2;
    while (tRot >= M_PI) tRot -= M_PI * 2;
    while (tRot < -M_PI ) tRot += M_PI * 2;
    float rotDir = tRot - rot;
    while (rotDir >= M_PI) rotDir -= M_PI * 2;
    while (rotDir < -M_PI ) rotDir += M_PI * 2;

    rot += rotDir * 0.4f;

    if (open < 0) open = 0;
    if (open > 1) open = 1;

    time++;
    oFlip = flip;

    float diff = (flipT - flip) * 0.4f;
    float max = 0.2f;
    if (diff < -max) diff = -max;
    if (diff > +max) diff = +max;
    flipA += (diff - flipA) * 0.9f;

    flip = flip + flipA;
}

std::wstring EnchantmentTableEntity::getName() {
    return hasCustomName() ? name : app.GetString(IDS_ENCHANT);
}

std::wstring EnchantmentTableEntity::getCustomName() {
    return hasCustomName() ? name : L"";
}

bool EnchantmentTableEntity::hasCustomName() { return !name.empty(); }

void EnchantmentTableEntity::setCustomName(const std::wstring& name) {
    this->name = name;
}

std::shared_ptr<TileEntity> EnchantmentTableEntity::clone() {
    std::shared_ptr<EnchantmentTableEntity> result =
        std::shared_ptr<EnchantmentTableEntity>(new EnchantmentTableEntity());
    TileEntity::clone(result);

    result->time = time;
    result->flip = flip;
    result->oFlip = oFlip;
    result->flipT = flipT;
    result->flipA = flipA;
    result->open = open;
    result->oOpen = oOpen;
    result->rot = rot;
    result->oRot = oRot;
    result->tRot = tRot;

    return result;
}