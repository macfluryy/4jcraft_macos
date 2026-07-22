#pragma once


enum ePARTICLE_TYPE {
    eParticleType_bubble,
    eParticleType_smoke,
    eParticleType_note,
    eParticleType_netherportal,  
                                 
                                 
    eParticleType_endportal,     
    eParticleType_explode,
    eParticleType_flame,
    eParticleType_lava,
    eParticleType_footstep,
    eParticleType_splash,
    eParticleType_largesmoke,
    eParticleType_reddust,
    eParticleType_snowballpoof,
    eParticleType_snowshovel,
    eParticleType_slime,
    eParticleType_heart,
    eParticleType_suspended,
    eParticleType_depthsuspend,
    eParticleType_crit,
    eParticleType_hugeexplosion,
    eParticleType_largeexplode,
    eParticleType_townaura,
    eParticleType_spell,
    eParticleType_witchMagic,
    eParticleType_mobSpell,
    eParticleType_mobSpellAmbient,
    eParticleType_instantSpell,
    eParticleType_magicCrit,
    eParticleType_dripWater,
    eParticleType_dripLava,
    eParticleType_enchantmenttable,
    eParticleType_dragonbreath,
    eParticleType_ender,  
                          
    eParticleType_angryVillager,
    eParticleType_happyVillager,
    eParticleType_fireworksspark,

    
    

    eParticleType_iconcrack_base =
        0x100000,  
                   
    eParticleType_iconcrack_last = 0x1FFFFF,
    eParticleType_tilecrack_base =
        0x200000,  
                   
    eParticleType_tilecrack_last = 0x2FFFFF,
    
    
    

};

#define PARTICLE_TILECRACK(id, data)                        \
    ((ePARTICLE_TYPE)(((int)eParticleType_tilecrack_base) | \
                      ((0x0FFF & id) << 8) | (0x0FF & data)))
#define PARTICLE_ICONCRACK(id, data)                        \
    ((ePARTICLE_TYPE)(((int)eParticleType_iconcrack_base) | \
                      ((0x0FFF & id) << 8) | (0x0FF & data)))

#define PARTICLE_CRACK_ID(ePType) ((0x0FFF00 & (int)ePType) >> 8)
#define PARTICLE_CRACK_DATA(ePType) (0x0FF & (int)ePType)