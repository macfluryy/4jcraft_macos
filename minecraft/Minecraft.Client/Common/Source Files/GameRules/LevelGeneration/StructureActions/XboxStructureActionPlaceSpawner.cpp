#include <wchar.h>
#include <memory>

#include "XboxStructureActionPlaceSpawner.h"
#include "Minecraft.Client/Common/Source Files/GameRules/LevelGeneration/StructureActions/XboxStructureActionPlaceSpawner.h"
#include "Minecraft.Client/Common/Source Files/GameRules/LevelGeneration/StructureActions/XboxStructureActionPlaceBlock.h"
#include "Minecraft.World/net/minecraft/world/level/Level.h"
#include "Minecraft.World/net/minecraft/world/level/levelgen/structure/BoundingBox.h"
#include "Minecraft.World/net/minecraft/world/level/levelgen/structure/StructurePiece.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"
#include "Minecraft.World/net/minecraft/world/level/tile/entity/MobSpawnerTileEntity.h"
#include "java/InputOutputStream/DataOutputStream.h"

XboxStructureActionPlaceSpawner::XboxStructureActionPlaceSpawner() {
    m_tile = Tile::mobSpawner_Id;
    m_entityId = L"Pig";
}

XboxStructureActionPlaceSpawner::~XboxStructureActionPlaceSpawner() {}

void XboxStructureActionPlaceSpawner::writeAttributes(DataOutputStream* dos,
                                                      unsigned int numAttrs) {
    XboxStructureActionPlaceBlock::writeAttributes(dos, numAttrs + 1);

    ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_entity);
    dos->writeUTF(m_entityId);
}

void XboxStructureActionPlaceSpawner::addAttribute(
    const std::wstring& attributeName, const std::wstring& attributeValue) {
    if (attributeName.compare(L"entity") == 0) {
        m_entityId = attributeValue;
#ifndef _CONTENT_PACKAGE
        wprintf(
            L"XboxStructureActionPlaceSpawner: Adding parameter entity=%ls\n",
            m_entityId.c_str());
#endif
    } else {
        XboxStructureActionPlaceBlock::addAttribute(attributeName,
                                                    attributeValue);
    }
}

bool XboxStructureActionPlaceSpawner::placeSpawnerInLevel(
    StructurePiece* structure, Level* level, BoundingBox* chunkBB) {
    int worldX = structure->getWorldX(m_x, m_z);
    int worldY = structure->getWorldY(m_y);
    int worldZ = structure->getWorldZ(m_x, m_z);

    if (chunkBB->isInside(worldX, worldY, worldZ)) {
        if (level->getTileEntity(worldX, worldY, worldZ) != nullptr) {
            // Remove the current tile entity
            level->removeTileEntity(worldX, worldY, worldZ);
            level->setTileAndData(worldX, worldY, worldZ, 0, 0,
                                  Tile::UPDATE_ALL);
        }

        level->setTileAndData(worldX, worldY, worldZ, m_tile, 0,
                              Tile::UPDATE_ALL);
        std::shared_ptr<MobSpawnerTileEntity> entity =
            std::dynamic_pointer_cast<MobSpawnerTileEntity>(
                level->getTileEntity(worldX, worldY, worldZ));

#ifndef _CONTENT_PACKAGE
        wprintf(
            L"XboxStructureActionPlaceSpawner - placing a %ls spawner at "
            L"(%d,%d,%d)\n",
            m_entityId.c_str(), worldX, worldY, worldZ);
#endif
        if (entity != nullptr) {
            entity->setEntityId(m_entityId);
        }
        return true;
    }
    return false;
}