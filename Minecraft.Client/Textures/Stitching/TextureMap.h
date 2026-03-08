#pragma once


#include "../../../Minecraft.World/Util/IconRegister.h"

class StitchedTexture;
class Texture;
class BufferedImage;

class TextureMap : public IconRegister
{
public:
	static const std::wstring NAME_MISSING_TEXTURE;

private:
	const int iconType;

	const std::wstring name;
	const std::wstring path;
	const std::wstring extension;

	bool m_mipMap;

	typedef std::unordered_map<std::wstring, StitchedTexture *> stringStitchedTextureMap;
	stringStitchedTextureMap texturesByName; //  = new HashMap<String, StitchedTexture>();
	BufferedImage *missingTexture; // = new BufferedImage(64, 64, BufferedImage.TYPE_INT_ARGB);
	StitchedTexture *missingPosition;
	Texture *stitchResult;
	std::vector<StitchedTexture *> animatedTextures; // = new ArrayList<StitchedTexture>();

	stringStitchedTextureMap texturesToRegister; // = new HashMap<String, StitchedTexture>();

public:
	TextureMap(int type, const std::wstring &name, const std::wstring &path, BufferedImage *missingTexture, bool mipMap = false);

	void stitch();
	StitchedTexture *getTexture(const std::wstring &name);
	void cycleAnimationFrames();
	Texture *getStitchedTexture();

	// 4J Stu - register is a reserved keyword in C++
	Icon *registerIcon(const std::wstring &name);

	int getIconType();
	Icon *getMissingIcon();
};