#include "stdafx.h"

#include "../Network/Packets/Packet.h"
#include "../Blocks/MaterialColor.h"
#include "../Blocks/Material.h"
#include "../Blocks/Tile.h"
#include "../Items/HatchetItem.h"
#include "../Items/PickaxeItem.h"
#include "../Items/ShovelItem.h"
#include "../Level/BlockReplacements.h"
#include "../WorldGen/Biomes/Biome.h"
#include "../Items/Item.h"
#include "../Recipes/FurnaceRecipes.h"
#include "../Recipes/Recipes.h"
#include "../Stats/Stats.h"
#include "../Stats/Achievements.h"
#include "../Entities/Mobs/Skeleton.h"
#include "../Entities/Mobs/PigZombie.h"
#include "../Blocks/TileEntities/TileEntity.h"
#include "../Level/Storage/EntityIO.h"
#include "../Util/SharedConstants.h"
#include "../Entities/MobCategory.h"
#include "../Level/LevelChunk.h"
#include "../WorldGen/Structures/MineShaftPieces.h"
#include "../WorldGen/Features/StrongholdFeature.h"
#include "../WorldGen/Features/VillageFeature.h"
#include "../Level/Storage/LevelType.h"
#include "../Entities/Mobs/EnderMan.h"
#include "../Blocks/TileEntities/PotionBrewing.h"
#include "../Enchantments/Enchantment.h"
#include "../WorldGen/Structures/VillagePieces.h"
#include "../WorldGen/Features/RandomScatteredLargeFeature.h"

#include "Minecraft.World.h"
#include "../../Minecraft.Client/Level/ServerLevel.h"
#include "../Level/Storage/SparseLightStorage.h"
#include "../Level/Storage/SparseDataStorage.h"
#include "../Level/Storage/McRegionChunkStorage.h"
#include "../Entities/Mobs/Villager.h"
#include "../Level/Storage/LevelSettings.h"

#ifdef _DURANGO
#include "../Stats/DurangoStats.h"
#else
#include "../Stats/CommonStats.h"
#endif

void MinecraftWorld_RunStaticCtors() {
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
        Item::staticCtor();
        FurnaceRecipes::staticCtor();
        Recipes::staticCtor();
#ifdef _DURANGO
        GenericStats::setInstance(new DurangoStats());
#else
        GenericStats::setInstance(new CommonStats());
        Stats::staticCtor();
#endif
        // Achievements::staticCtor(); // 4J Stu - This is now called from
        // within the Stats::staticCtor()
        Skeleton::staticCtor();
        PigZombie::staticCtor();
        TileEntity::staticCtor();
        EntityIO::staticCtor();
        MobCategory::staticCtor();

        Item::staticInit();
        LevelChunk::staticCtor();

        LevelType::staticCtor();

        MineShaftPieces::staticCtor();
        StrongholdFeature::staticCtor();
        VillagePieces::Smithy::staticCtor();
        VillageFeature::staticCtor();
        RandomScatteredLargeFeature::staticCtor();
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
}
