#pragma once
#include <memory>
#include <string>

#include "Minecraft.Client/include/NetTypes.h"
#include "Minecraft.Client/include/SkinBox.h"
#include "Minecraft.Client/include/XboxStubs.h"
#include "MobRenderer.h"
#include "minecraft/client/renderer/entity/LivingEntityRenderer.h"
#include "minecraft/world/entity/player/Player.h"

class HumanoidModel;
class LivingEntity;
class ResourceLocation;

class PlayerRenderer : public LivingEntityRenderer {
public:
    // 4J: Made public for use in skull renderer
    static ResourceLocation DEFAULT_LOCATION;

private:
    // 4J Added
    static const unsigned int s_nametagColors[MINECRAFT_NET_MAX_PLAYERS];

    HumanoidModel* humanoidModel;
    HumanoidModel* armorParts1;
    HumanoidModel* armorParts2;

public:
    PlayerRenderer();

    static unsigned int getNametagColour(int index);

private:
    static const std::wstring MATERIAL_NAMES[5];

protected:
    virtual int prepareArmor(std::shared_ptr<LivingEntity> _player, int layer,
                             float a);
    virtual void prepareSecondPassArmor(std::shared_ptr<LivingEntity> mob,
                                        int layer, float a);

public:
    virtual void render(std::shared_ptr<Entity> _mob, double x, double y,
                        double z, float rot, float a);

protected:
    virtual void additionalRendering(std::shared_ptr<LivingEntity> _mob,
                                     float a);
    void renderNameTags(std::shared_ptr<LivingEntity> player, double x,
                        double y, double z, std::wstring msg, float scale,
                        double dist);

    virtual void scale(std::shared_ptr<LivingEntity> _player, float a);

public:
    void renderHand();

protected:
    virtual void setupPosition(std::shared_ptr<LivingEntity> _mob, double x,
                               double y, double z);
    virtual void setupRotations(std::shared_ptr<LivingEntity> _mob, float bob,
                                float bodyRot, float a);

private:
    virtual void renderShadow(std::shared_ptr<Entity> e, double x, double y,
                              double z, float pow,
                              float a);  // 4J Added override

public:
    virtual ResourceLocation* getTextureLocation(
        std::shared_ptr<Entity> entity);

    using LivingEntityRenderer::bindTexture;
    virtual void bindTexture(
        std::shared_ptr<Entity> entity);  // 4J Added override
};