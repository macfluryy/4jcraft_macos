#pragma once
#include <string>

class LevelType {
public:
    static LevelType* levelTypes[16];  
    static LevelType*
        lvl_normal;  
    static LevelType* lvl_flat;         
    static LevelType* lvl_largeBiomes;  
    static LevelType* lvl_amplified;    
    
    
    
    
    
    
    static LevelType* lvl_triple;       
    static LevelType* lvl_normal_1_1;   
                                        

    static void staticCtor();

private:
    int id;
    std::wstring m_generatorName;
    int m_version;
    bool m_selectable;
    bool m_replacement;

    LevelType(int id, std::wstring generatorName);
    LevelType(int id, std::wstring generatorName, int version);
    void init(int id, std::wstring generatorName, int version);

public:
    std::wstring getGeneratorName();
    std::wstring getDescriptionId();
    int getVersion();
    LevelType* getReplacementForVersion(int oldVersion);

private:
    LevelType* setSelectableByUser(bool selectable);

public:
    bool isSelectable();

private:
    LevelType* setHasReplacement();

public:
    bool hasReplacement();
    static LevelType* getLevelType(std::wstring name);
    int getId();
};
