#pragma once
#include "MobRenderer.h"
#include "../../../Minecraft.World/Player/Player.h"
class HumanoidModel;
using namespace std;

class PlayerRenderer : public MobRenderer
{
private:
	// 4J Added
	static const unsigned int s_nametagColors[MINECRAFT_NET_MAX_PLAYERS];

	HumanoidModel *humanoidModel;
    HumanoidModel *armorParts1;
    HumanoidModel *armorParts2;

public:
	PlayerRenderer();

	static unsigned int getNametagColour(int index);

private:
	static const wstring MATERIAL_NAMES[5];

protected:
	virtual int prepareArmor(std::shared_ptr<Mob> _player, int layer, float a);
	virtual void prepareSecondPassArmor(std::shared_ptr<Mob> mob, int layer, float a);
public:
	virtual void render(std::shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a);
protected:
    virtual void renderName(std::shared_ptr<Mob> _mob, double x, double y, double z);
    virtual void additionalRendering(std::shared_ptr<Mob> _mob, float a);
    virtual void scale(std::shared_ptr<Mob> _player, float a);
public:
	void renderHand();
protected:
	virtual void setupPosition(std::shared_ptr<Mob> _mob, double x, double y, double z);
    virtual void setupRotations(std::shared_ptr<Mob> _mob, float bob, float bodyRot, float a);

private:	
    virtual void renderShadow(std::shared_ptr<Entity> e, double x, double y, double z, float pow, float a); // 4J Added override
};