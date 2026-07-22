#pragma once

#include <string>

#include "DispenserTileEntity.h"
#include "java/Class.h"

class TileEntity;

class DropperTileEntity : public DispenserTileEntity {
public:
    eINSTANCEOF GetType() { return eTYPE_DROPPERTILEENTITY; }
    static TileEntity* create() { return new DropperTileEntity(); }
    
    virtual std::shared_ptr<TileEntity> clone();

public:
    std::wstring getName();
};