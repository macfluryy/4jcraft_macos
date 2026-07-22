#include "GameRuleManager.h"

#include <assert.h>
#include <string.h>

#include <cstdint>
#include <utility>
#include <vector>

#include "app/common/src/DLC/DLCGameRulesFile.h"
#include "app/common/src/DLC/DLCGameRulesHeader.h"
#include "app/common/src/DLC/DLCLocalisationFile.h"
#include "app/common/src/DLC/DLCManager.h"
#include "app/common/src/DLC/DLCPack.h"
#include "app/common/src/GameRules/LevelGeneration/ConsoleSchematicFile.h"
#include "app/common/src/GameRules/LevelGeneration/LevelGenerationOptions.h"
#include "app/common/src/GameRules/LevelGeneration/LevelGenerators.h"
#include "app/common/src/GameRules/LevelRules/LevelRules.h"
#include "app/common/src/GameRules/LevelRules/RuleDefinitions/GameRuleDefinition.h"
#include "app/common/src/GameRules/LevelRules/RuleDefinitions/LevelRuleset.h"
#include "app/common/src/Localisation/StringTable.h"
#include "app/mac/MacGame.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/compression.h"
#include "java/File.h"
#include "java/InputOutputStream/ByteArrayInputStream.h"
#include "java/InputOutputStream/ByteArrayOutputStream.h"
#include "java/InputOutputStream/DataInputStream.h"
#include "java/InputOutputStream/DataOutputStream.h"
#include "minecraft/world/level/storage/ConsoleSaveFileIO/FileHeader.h"
#include "strings.h"

const wchar_t* GameRuleManager::wchTagNameA[] = {
    L"",                   
    L"MapOptions",         
    L"ApplySchematic",     
    L"GenerateStructure",  
    L"GenerateBox",        
    L"PlaceBlock",         
    L"PlaceContainer",     
    L"PlaceSpawner",       
    L"BiomeOverride",      
    L"StartFeature",       
    L"AddItem",            
    L"AddEnchantment",     
    L"LevelRules",         
    L"NamedArea",          
    L"UseTile",            
    L"CollectItem",        
    L"CompleteAll",        
    L"UpdatePlayer",       
};

const wchar_t* GameRuleManager::wchAttrNameA[] = {
    L"descriptionName",   
    L"promptName",        
    L"dataTag",           
    L"enchantmentId",     
    L"enchantmentLevel",  
    L"itemId",            
    L"quantity",          
    L"auxValue",          
    L"slot",              
    L"name",              
    L"food",              
    L"health",            
    L"tileId",            
    L"useCoords",         
    L"seed",              
    L"flatworld",         
    L"filename",          
    L"rot",               
    L"data",              
    L"block",             
    L"entity",            
    L"facing",            
    L"edgeTile",          
    L"fillTile",          
    L"skipAir",           
    L"x",                 
    L"x0",                
    L"x1",                
    L"y",                 
    L"y0",                
    L"y1",                
    L"z",                 
    L"z0",                
    L"z1",                
    L"chunkX",            
    L"chunkZ",            
    L"yRot",              
    L"spawnX",            
    L"spawnY",            
    L"spawnZ",            
    L"orientation",
    L"dimension",
    L"topTileId",  
    L"biomeId",    
    L"feature",    
};

GameRuleManager::GameRuleManager() {
    m_currentGameRuleDefinitions = nullptr;
    m_currentLevelGenerationOptions = nullptr;
}

void GameRuleManager::loadGameRules(DLCPack* pack) {
    StringTable* strings = nullptr;

    if (pack->doesPackContainFile(DLCManager::e_DLCType_LocalisationData,
                                  L"languages.loc")) {
        DLCLocalisationFile* localisationFile =
            (DLCLocalisationFile*)pack->getFile(
                DLCManager::e_DLCType_LocalisationData, L"languages.loc");
        strings = localisationFile->getStringTable();
    }

    int gameRulesCount =
        pack->getDLCItemsCount(DLCManager::e_DLCType_GameRulesHeader);
    for (int i = 0; i < gameRulesCount; ++i) {
        DLCGameRulesHeader* dlcHeader = (DLCGameRulesHeader*)pack->getFile(
            DLCManager::e_DLCType_GameRulesHeader, i);
        std::uint32_t dSize;
        uint8_t* dData = dlcHeader->getData(dSize);

        LevelGenerationOptions* createdLevelGenerationOptions =
            new LevelGenerationOptions(pack);
        

        createdLevelGenerationOptions->setGrSource(dlcHeader);
        createdLevelGenerationOptions->setSrc(
            LevelGenerationOptions::eSrc_fromDLC);

        readRuleFile(createdLevelGenerationOptions, dData, dSize, strings);

        dlcHeader->lgo = createdLevelGenerationOptions;
    }

    gameRulesCount = pack->getDLCItemsCount(DLCManager::e_DLCType_GameRules);
    for (int i = 0; i < gameRulesCount; ++i) {
        DLCGameRulesFile* dlcFile = (DLCGameRulesFile*)pack->getFile(
            DLCManager::e_DLCType_GameRules, i);

        std::uint32_t dSize;
        uint8_t* dData = dlcFile->getData(dSize);

        LevelGenerationOptions* createdLevelGenerationOptions =
            new LevelGenerationOptions(pack);
        

        createdLevelGenerationOptions->setGrSource(new JustGrSource());
        createdLevelGenerationOptions->setSrc(
            LevelGenerationOptions::eSrc_tutorial);

        readRuleFile(createdLevelGenerationOptions, dData, dSize, strings);

        createdLevelGenerationOptions->setLoadedData();
    }
}

LevelGenerationOptions* GameRuleManager::loadGameRules(uint8_t* dIn,
                                                       unsigned int dSize) {
    LevelGenerationOptions* lgo = new LevelGenerationOptions();
    lgo->setGrSource(new JustGrSource());
    lgo->setSrc(LevelGenerationOptions::eSrc_fromSave);
    loadGameRules(lgo, dIn, dSize);
    lgo->setLoadedData();
    return lgo;
}


void GameRuleManager::loadGameRules(LevelGenerationOptions* lgo, uint8_t* dIn,
                                    unsigned int dSize) {
    app.DebugPrintf("GameRuleManager::LoadingGameRules:\n");

    std::vector<uint8_t> inputBuf(dIn, dIn + dSize);
    ByteArrayInputStream bais(inputBuf);
    DataInputStream dis(&bais);

    

    

    short version = dis.readShort();
    assert(0x1 == version);
    app.DebugPrintf("\tversion=%d.\n", version);

    for (int i = 0; i < 8; i++) dis.readByte();

    std::uint8_t compression_type = dis.readByte();

    app.DebugPrintf("\tcompressionType=%d.\n", compression_type);

    unsigned int compr_len, decomp_len;
    compr_len = dis.readInt();
    decomp_len = dis.readInt();

    app.DebugPrintf("\tcompr_len=%d.\n\tdecomp_len=%d.\n", compr_len,
                    decomp_len);

    

    std::vector<uint8_t> content(decomp_len);
    std::vector<uint8_t> compr_content(compr_len);
    dis.read(compr_content);

    Compression::getCompression()->SetDecompressionType(
        (Compression::ECompressionTypes)compression_type);
    unsigned int contentSize = decomp_len;
    Compression::getCompression()->DecompressLZXRLE(
        content.data(), &contentSize, compr_content.data(),
        compr_content.size());
    content.resize(contentSize);
    Compression::getCompression()->SetDecompressionType(
        SAVE_FILE_PLATFORM_LOCAL);

    dis.close();
    bais.close();

    ByteArrayInputStream bais2(content);
    DataInputStream dis2(&bais2);

    
    unsigned int bStringTableSize = dis2.readInt();
    std::vector<uint8_t> bStringTable(bStringTableSize);
    dis2.read(bStringTable);
    StringTable* strings =
        new StringTable(bStringTable.data(), bStringTable.size());

    
    std::vector<uint8_t> bRuleFile(content.size() - bStringTable.size());
    dis2.read(bRuleFile);

    
    
    

    if (readRuleFile(lgo, bRuleFile.data(), bRuleFile.size(), strings)) {
        
        
        lgo->setSrc(LevelGenerationOptions::eSrc_fromSave);
        setLevelGenerationOptions(lgo);
        
    } else {
        delete lgo;
    }

    
    dis2.close();
    bais2.close();

    return;
}


void GameRuleManager::saveGameRules(uint8_t** dOut, unsigned int* dSize) {
    if (m_currentGameRuleDefinitions == nullptr &&
        m_currentLevelGenerationOptions == nullptr) {
        app.DebugPrintf("GameRuleManager:: Nothing here to save.");
        *dOut = nullptr;
        *dSize = 0;
        return;
    }

    app.DebugPrintf("GameRuleManager::saveGameRules:\n");

    
    ByteArrayOutputStream baos;
    DataOutputStream dos(&baos);

    

    
    dos.writeShort(0x1);  

    
    
    for (unsigned int i = 0; i < 8; i++) dos.writeByte(0x0);

    dos.writeByte(APPROPRIATE_COMPRESSION_TYPE);  

    
    ByteArrayOutputStream compr_baos;
    DataOutputStream compr_dos(&compr_baos);

    if (m_currentGameRuleDefinitions == nullptr) {
        compr_dos.writeInt(0);  
        compr_dos.writeInt(version_number);
        compr_dos.writeByte(
            Compression::eCompressionType_None);  
        for (int i = 0; i < 2; i++) compr_dos.writeByte(0x0);  
        compr_dos.writeInt(0);  
        compr_dos.writeInt(0);  
        compr_dos.writeInt(0);  
    } else {
        StringTable* st = m_currentGameRuleDefinitions->getStringTable();

        if (st == nullptr) {
            app.DebugPrintf(
                "GameRuleManager::saveGameRules: StringTable == nullptr!");
        } else {
            
            uint8_t* stbaPtr = nullptr;
            unsigned int stbaSize = 0;
            m_currentGameRuleDefinitions->getStringTable()->getData(&stbaPtr,
                                                                    &stbaSize);
            std::vector<uint8_t> stba(stbaPtr, stbaPtr + stbaSize);
            compr_dos.writeInt(stba.size());
            compr_dos.write(stba);

            
            
            writeRuleFile(&compr_dos);
        }
    }

    
    std::vector<uint8_t> compr_ba(compr_baos.buf.size());
    unsigned int compr_ba_size = compr_ba.size();
    Compression::getCompression()->CompressLZXRLE(
        compr_ba.data(), &compr_ba_size, compr_baos.buf.data(),
        compr_baos.buf.size());
    compr_ba.resize(compr_ba_size);

    app.DebugPrintf("\tcompr_ba.size()=%d.\n\tcompr_baos.buf.size()=%d.\n",
                    compr_ba.size(), compr_baos.buf.size());

    dos.writeInt(compr_ba.size());  
    dos.writeInt(compr_baos.buf.size());
    dos.write(compr_ba);

    compr_dos.close();
    compr_baos.close();
    

    
    *dSize = baos.buf.size();
    *dOut = new uint8_t[baos.buf.size()];
    memcpy(*dOut, baos.buf.data(), baos.buf.size());

    dos.close();
    baos.close();
}


void GameRuleManager::writeRuleFile(DataOutputStream* dos) {
    
    dos->writeShort(version_number);                       
    dos->writeByte(Compression::eCompressionType_None);    
    for (int i = 0; i < 8; i++) dos->writeBoolean(false);  

    
    int numStrings = static_cast<int>(ConsoleGameRules::eGameRuleType_Count) +
                     static_cast<int>(ConsoleGameRules::eGameRuleAttr_Count);
    dos->writeInt(numStrings);
    for (int i = 0; i < ConsoleGameRules::eGameRuleType_Count; i++)
        dos->writeUTF(wchTagNameA[i]);
    for (int i = 0; i < ConsoleGameRules::eGameRuleAttr_Count; i++)
        dos->writeUTF(wchAttrNameA[i]);

    
    std::unordered_map<std::wstring, ConsoleSchematicFile*>* files;
    files = getLevelGenerationOptions()->getUnfinishedSchematicFiles();
    dos->writeInt(files->size());
    for (auto it = files->begin(); it != files->end(); it++) {
        std::wstring filename = it->first;
        ConsoleSchematicFile* file = it->second;

        ByteArrayOutputStream fileBaos;
        DataOutputStream fileDos(&fileBaos);
        file->save(&fileDos);

        dos->writeUTF(filename);
        
        dos->writeInt(fileBaos.buf.size());
        dos->write((std::vector<uint8_t>)fileBaos.buf);

        fileDos.close();
        fileBaos.close();
    }

    
    dos->writeInt(2);  
    m_currentLevelGenerationOptions->write(dos);
    m_currentGameRuleDefinitions->write(dos);
}

bool GameRuleManager::readRuleFile(
    LevelGenerationOptions* lgo, uint8_t* dIn, unsigned int dSize,
    StringTable* strings)  
{
    bool levelGenAdded = false;
    bool gameRulesAdded = false;
    LevelGenerationOptions* levelGenerator =
        lgo;  
    LevelRuleset* gameRules = new LevelRuleset();

    
    
    

    std::vector<uint8_t> data(dIn, dIn + dSize);
    ByteArrayInputStream bais(data);
    DataInputStream dis(&bais);

    

    
    int64_t version = dis.readShort();
    unsigned char compressionType = 0;
    if (version == 0) {
        for (int i = 0; i < 14; i++) dis.readByte();  
    } else {
        compressionType = dis.readByte();

        
        for (int i = 0; i < 8; ++i) dis.readBoolean();
    }

    ByteArrayInputStream* contentBais = nullptr;
    DataInputStream* contentDis = nullptr;

    if (compressionType == Compression::eCompressionType_None) {
        
        
        app.DebugPrintf("De-compressing game rules with: None\n");
        contentDis = &dis;
    } else {
        unsigned int uncompressedSize = dis.readInt();
        unsigned int compressedSize = dis.readInt();
        std::vector<uint8_t> compressedBuffer(compressedSize);
        dis.read(compressedBuffer);

        std::vector<uint8_t> decompressedBuffer =
            std::vector<uint8_t>(uncompressedSize);
        unsigned int decompressedSize = uncompressedSize;

        switch (compressionType) {
            case Compression::eCompressionType_None:
                memcpy(decompressedBuffer.data(), compressedBuffer.data(),
                       uncompressedSize);
                break;

            case Compression::eCompressionType_RLE:
                app.DebugPrintf("De-compressing game rules with: RLE\n");
                Compression::getCompression()->Decompress(
                    decompressedBuffer.data(), &decompressedSize,
                    compressedBuffer.data(), compressedSize);
                decompressedBuffer.resize(decompressedSize);
                break;

            default:
                app.DebugPrintf("De-compressing game rules.");
#if !defined(_CONTENT_PACKAGE)
                assert(compressionType == APPROPRIATE_COMPRESSION_TYPE);
#endif
                
                
                
                Compression::getCompression()->DecompressLZXRLE(
                    decompressedBuffer.data(), &decompressedSize,
                    compressedBuffer.data(), compressedSize);
                decompressedBuffer.resize(decompressedSize);
                break;
                


















        };

        contentBais = new ByteArrayInputStream(decompressedBuffer);
        contentDis = new DataInputStream(contentBais);
    }

    
    unsigned int numStrings = contentDis->readInt();
    std::vector<std::wstring> tagsAndAtts;
    for (unsigned int i = 0; i < numStrings; i++)
        tagsAndAtts.push_back(contentDis->readUTF());

    std::unordered_map<int, ConsoleGameRules::EGameRuleType> tagIdMap;
    for (int type = (int)ConsoleGameRules::eGameRuleType_Root;
         type < (int)ConsoleGameRules::eGameRuleType_Count; ++type) {
        for (unsigned int i = 0; i < numStrings; ++i) {
            if (tagsAndAtts[i].compare(wchTagNameA[type]) == 0) {
                tagIdMap.insert(
                    std::unordered_map<int, ConsoleGameRules::EGameRuleType>::
                        value_type(i, (ConsoleGameRules::EGameRuleType)type));
                break;
            }
        }
    }

    
    















    
    unsigned int numFiles = contentDis->readInt();
    for (unsigned int i = 0; i < numFiles; i++) {
        std::wstring sFilename = contentDis->readUTF();
        int length = contentDis->readInt();
        std::vector<uint8_t> ba(length);

        contentDis->read(ba);

        levelGenerator->loadSchematicFile(sFilename, ba.data(), ba.size());
    }

    LEVEL_GEN_ID lgoID = LEVEL_GEN_ID_NULL;

    
    unsigned int numObjects = contentDis->readInt();
    for (unsigned int i = 0; i < numObjects; ++i) {
        int tagId = contentDis->readInt();
        ConsoleGameRules::EGameRuleType tagVal =
            ConsoleGameRules::eGameRuleType_Invalid;
        auto it = tagIdMap.find(tagId);
        if (it != tagIdMap.end()) tagVal = it->second;

        GameRuleDefinition* rule = nullptr;

        if (tagVal == ConsoleGameRules::eGameRuleType_LevelGenerationOptions) {
            rule = levelGenerator;
            levelGenAdded = true;
            
            lgoID = addLevelGenerationOptions(levelGenerator);
            levelGenerator->loadStringTable(strings);
        } else if (tagVal == ConsoleGameRules::eGameRuleType_LevelRules) {
            rule = gameRules;
            gameRulesAdded = true;
            m_levelRules.addLevelRule(L"", gameRules);
            levelGenerator->setRequiredGameRules(gameRules);
            gameRules->loadStringTable(strings);
        }

        readAttributes(contentDis, &tagsAndAtts, rule);
        readChildren(contentDis, &tagsAndAtts, &tagIdMap, rule);
    }

    if (compressionType != 0) {
        
        contentDis->close();
        if (contentBais != nullptr) delete contentBais;
        delete contentDis;
    }

    dis.close();
    bais.reset();

    
    if (!gameRulesAdded) delete gameRules;

    return true;
    
}

LevelGenerationOptions* GameRuleManager::readHeader(DLCGameRulesHeader* grh) {
    LevelGenerationOptions* out = new LevelGenerationOptions();

    out->setSrc(LevelGenerationOptions::eSrc_fromDLC);
    out->setGrSource(grh);
    addLevelGenerationOptions(out);

    return out;
}

void GameRuleManager::readAttributes(DataInputStream* dis,
                                     std::vector<std::wstring>* tagsAndAtts,
                                     GameRuleDefinition* rule) {
    int numAttrs = dis->readInt();
    for (unsigned int att = 0; att < static_cast<unsigned int>(numAttrs);
         ++att) {
        int attID = dis->readInt();
        std::wstring value = dis->readUTF();

        if (rule != nullptr) rule->addAttribute(tagsAndAtts->at(attID), value);
    }
}

void GameRuleManager::readChildren(
    DataInputStream* dis, std::vector<std::wstring>* tagsAndAtts,
    std::unordered_map<int, ConsoleGameRules::EGameRuleType>* tagIdMap,
    GameRuleDefinition* rule) {
    int numChildren = dis->readInt();
    for (unsigned int child = 0; child < static_cast<unsigned int>(numChildren);
         ++child) {
        int tagId = dis->readInt();
        ConsoleGameRules::EGameRuleType tagVal =
            ConsoleGameRules::eGameRuleType_Invalid;
        auto it = tagIdMap->find(tagId);
        if (it != tagIdMap->end()) tagVal = it->second;

        GameRuleDefinition* childRule = nullptr;
        if (rule != nullptr) childRule = rule->addChild(tagVal);

        readAttributes(dis, tagsAndAtts, childRule);
        readChildren(dis, tagsAndAtts, tagIdMap, childRule);
    }
}

void GameRuleManager::processSchematics(LevelChunk* levelChunk) {
    if (getLevelGenerationOptions() != nullptr) {
        LevelGenerationOptions* levelGenOptions = getLevelGenerationOptions();
        levelGenOptions->processSchematics(levelChunk);
    }
}

void GameRuleManager::processSchematicsLighting(LevelChunk* levelChunk) {
    if (getLevelGenerationOptions() != nullptr) {
        LevelGenerationOptions* levelGenOptions = getLevelGenerationOptions();
        levelGenOptions->processSchematicsLighting(levelChunk);
    }
}

void GameRuleManager::loadDefaultGameRules() {
#if !defined(__linux__) && !defined(__APPLE__)
#if defined(_WINDOWS64)
    File packedTutorialFile(L"Windows64Media\\Tutorial\\Tutorial.pck");
    if (!packedTutorialFile.exists())
        packedTutorialFile = File(L"Windows64\\Tutorial\\Tutorial.pck");
#else
    File packedTutorialFile(L"Tutorial\\Tutorial.pck");
#endif
    if (loadGameRulesPack(&packedTutorialFile)) {
        m_levelGenerators.getLevelGenerators()->at(0)->setWorldName(
            app.GetString(IDS_PLAY_TUTORIAL));
        
        m_levelGenerators.getLevelGenerators()->at(0)->setDefaultSaveName(
            app.GetString(IDS_TUTORIALSAVENAME));
    }
#else
    std::wstring fpTutorial = L"Tutorial.pck";
    if (app.getArchiveFileSize(fpTutorial) >= 0) {
        DLCPack* pack = new DLCPack(L"", 0xffffffff);
        uint32_t dwFilesProcessed = 0;
        if (app.m_dlcManager.readDLCDataFile(dwFilesProcessed, fpTutorial, pack,
                                             true)) {
            app.m_dlcManager.addPack(pack);
            m_levelGenerators.getLevelGenerators()->at(0)->setWorldName(
                app.GetString(IDS_PLAY_TUTORIAL));
            m_levelGenerators.getLevelGenerators()->at(0)->setDefaultSaveName(
                app.GetString(IDS_TUTORIALSAVENAME));
        } else
            delete pack;
    }
#endif
}

bool GameRuleManager::loadGameRulesPack(File* path) {
    bool success = false;
    if (path->exists()) {
        DLCPack* pack = new DLCPack(L"", 0xffffffff);
        unsigned int dwFilesProcessed = 0;
        if (app.m_dlcManager.readDLCDataFile(dwFilesProcessed, path->getPath(),
                                             pack)) {
            app.m_dlcManager.addPack(pack);
            success = true;
        } else {
            delete pack;
        }
    }
    return success;
}

void GameRuleManager::setLevelGenerationOptions(
    LevelGenerationOptions* levelGen) {
    unloadCurrentGameRules();

    m_currentGameRuleDefinitions = nullptr;
    m_currentLevelGenerationOptions = levelGen;

    if (m_currentLevelGenerationOptions != nullptr &&
        m_currentLevelGenerationOptions->requiresGameRules()) {
        m_currentGameRuleDefinitions =
            m_currentLevelGenerationOptions->getRequiredGameRules();
    }

    if (m_currentLevelGenerationOptions != nullptr)
        m_currentLevelGenerationOptions->reset_start();
}

const wchar_t* GameRuleManager::GetGameRulesString(const std::wstring& key) {
    if (m_currentGameRuleDefinitions != nullptr && !key.empty()) {
        return m_currentGameRuleDefinitions->getString(key);
    } else {
        return L"";
    }
}

LEVEL_GEN_ID GameRuleManager::addLevelGenerationOptions(
    LevelGenerationOptions* lgo) {
    std::vector<LevelGenerationOptions*>* lgs =
        m_levelGenerators.getLevelGenerators();

    for (int i = 0; i < lgs->size(); i++)
        if (lgs->at(i) == lgo) return i;

    lgs->push_back(lgo);
    return lgs->size() - 1;
}

void GameRuleManager::unloadCurrentGameRules() {
    if (m_currentLevelGenerationOptions != nullptr) {
        if (m_currentGameRuleDefinitions != nullptr &&
            m_currentLevelGenerationOptions->isFromSave())
            m_levelRules.removeLevelRule(m_currentGameRuleDefinitions);

        if (m_currentLevelGenerationOptions->isFromSave()) {
            m_levelGenerators.removeLevelGenerator(
                m_currentLevelGenerationOptions);

            delete m_currentLevelGenerationOptions;
        } else if (m_currentLevelGenerationOptions->isFromDLC()) {
            m_currentLevelGenerationOptions->reset_finish();
        }
    }

    m_currentGameRuleDefinitions = nullptr;
    m_currentLevelGenerationOptions = nullptr;
}
