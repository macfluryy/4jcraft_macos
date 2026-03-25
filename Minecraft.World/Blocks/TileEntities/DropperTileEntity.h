#pragma once

#include "DispenserTileEntity.h"

class DropperTileEntity : public DispenserTileEntity {
public:
    eINSTANCEOF GetType() { return eTYPE_DROPPERTILEENTITY; }
    static TileEntity* create() { return new DropperTileEntity(); }
    // 4J Added
    virtual std::shared_ptr<TileEntity> clone();

public:
    std::wstring getName();
};