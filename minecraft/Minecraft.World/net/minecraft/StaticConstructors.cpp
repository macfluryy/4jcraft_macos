#include "Minecraft.World/net/minecraft/StaticConstructors.h"
#include "Minecraft.Client/net/minecraft/server/level/ServerLevel.h"
#include "Minecraft.World/net/minecraft/stats/CommonStats.h"
#include "Minecraft.World/net/minecraft/SharedConstants.h"
#include "Minecraft.World/net/minecraft/network/packet/Packet.h"
#include "Minecraft.World/net/minecraft/stats/GenericStats.h"
#include "Minecraft.World/net/minecraft/stats/Stats.h"
#include "Minecraft.World/net/minecraft/world/effect/MobEffect.h"
#include "Minecraft.World/net/minecraft/world/entity/EntityIO.h"
#include "Minecraft.World/net/minecraft/world/entity/MobCategory.h"
#include "Minecraft.World/net/minecraft/world/entity/monster/EnderMan.h"
#include "Minecraft.World/net/minecraft/world/entity/npc/Villager.h"
#include "Minecraft.World/net/minecraft/world/item/HatchetItem.h"
#include "Minecraft.World/net/minecraft/world/item/Item.h"
#include "Minecraft.World/net/minecraft/world/item/PickaxeItem.h"
#include "Minecraft.World/net/minecraft/world/item/ShovelItem.h"
#include "Minecraft.World/net/minecraft/world/item/alchemy/PotionBrewing.h"
#include "Minecraft.World/net/minecraft/world/item/crafting/FurnaceRecipes.h"
#include "Minecraft.World/net/minecraft/world/item/crafting/Recipes.h"
#include "Minecraft.World/net/minecraft/world/item/enchantment/Enchantment.h"
#include "Minecraft.World/net/minecraft/world/level/LevelSettings.h"
#include "Minecraft.World/net/minecraft/world/level/LevelType.h"
#include "Minecraft.World/net/minecraft/world/level/biome/Biome.h"
#include "Minecraft.World/net/minecraft/world/level/chunk/BlockReplacements.h"
#include "Minecraft.World/net/minecraft/world/level/chunk/CompressedTileStorage.h"
#include "Minecraft.World/net/minecraft/world/level/chunk/LevelChunk.h"
#include "Minecraft.World/net/minecraft/world/level/chunk/SparseDataStorage.h"
#include "Minecraft.World/net/minecraft/world/level/chunk/SparseLightStorage.h"
#include "Minecraft.World/net/minecraft/world/level/chunk/storage/McRegionChunkStorage.h"
#include "Minecraft.World/net/minecraft/world/level/levelgen/structure/MineShaftPieces.h"
#include "Minecraft.World/net/minecraft/world/level/levelgen/structure/RandomScatteredLargeFeature.h"
#include "Minecraft.World/net/minecraft/world/level/levelgen/structure/StrongholdFeature.h"
#include "Minecraft.World/net/minecraft/world/level/levelgen/structure/StructureFeatureIO.h"
#include "Minecraft.World/net/minecraft/world/level/levelgen/structure/VillageFeature.h"
#include "Minecraft.World/net/minecraft/world/level/levelgen/structure/VillagePieces.h"
#include "Minecraft.World/net/minecraft/world/level/material/Material.h"
#include "Minecraft.World/net/minecraft/world/level/material/MaterialColor.h"
#include "Minecraft.World/net/minecraft/world/level/tile/Tile.h"
#include "Minecraft.World/net/minecraft/world/level/tile/entity/BeaconTileEntity.h"
#include "Minecraft.World/net/minecraft/world/level/tile/entity/TileEntity.h"

void Minecraft_RunStaticCtors() {
    // The ordering of these static ctors can be important. If they are within
    // statement blocks then DO NOT CHANGE the ordering - 4J Stu

    Packet::staticCtor();

    {
        MaterialColor::staticCtor();
        Material::staticCtor();
        Tile::staticCtor();
        HatchetItem::staticCtor();
        PickaxeItem::staticCtor();
        ShovelItem::staticCtor();
        BlockReplacements::staticCtor();
        Biome::staticCtor();
        MobEffect::staticCtor();
        Item::staticCtor();
        FurnaceRecipes::staticCtor();
        Recipes::staticCtor();
        GenericStats::setInstance(new CommonStats());
        Stats::staticCtor();
        // Achievements::staticCtor(); // 4J Stu - This is now called from
        // within the Stats::staticCtor()
        TileEntity::staticCtor();
        EntityIO::staticCtor();
        MobCategory::staticCtor();

        Item::staticInit();
        LevelChunk::staticCtor();

        LevelType::staticCtor();

        {
            StructureFeatureIO::staticCtor();

            MineShaftPieces::staticCtor();
            StrongholdFeature::staticCtor();
            VillagePieces::Smithy::staticCtor();
            VillageFeature::staticCtor();
            RandomScatteredLargeFeature::staticCtor();
        }
    }
    EnderMan::staticCtor();
    PotionBrewing::staticCtor();
    Enchantment::staticCtor();

    SharedConstants::staticCtor();

    ServerLevel::staticCtor();
    SparseLightStorage::staticCtor();
    CompressedTileStorage::staticCtor();
    SparseDataStorage::staticCtor();
    McRegionChunkStorage::staticCtor();
    Villager::staticCtor();
    GameType::staticCtor();
    BeaconTileEntity::staticCtor();
}
