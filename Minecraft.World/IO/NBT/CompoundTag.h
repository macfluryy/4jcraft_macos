#pragma once
#include "Tag.h"
#include "ListTag.h"
#include "ByteTag.h"
#include "DoubleTag.h"
#include "FloatTag.h"
#include "IntTag.h"
#include "LongTag.h"
#include "ShortTag.h"
#include "StringTag.h"
#include "ByteArrayTag.h"
#include "IntArrayTag.h"

class CompoundTag : public Tag    
{
private:
	std::unordered_map<std::wstring, Tag *> tags;

public:
	CompoundTag() : Tag(L"") {}
	CompoundTag(const std::wstring &name) : Tag(name) {}

    void write(DataOutput *dos)
	{
		AUTO_VAR(itEnd, tags.end());
		for( std::unordered_map<std::wstring, Tag *>::iterator it = tags.begin(); it != itEnd; it++ )
		{
			Tag::writeNamedTag(it->second, dos);
		}
		dos->writeByte(Tag::TAG_End);
    }


	void load(DataInput *dis)
	{
        tags.clear();
        Tag *tag;
        while ((tag = Tag::readNamedTag(dis))->getId() != Tag::TAG_End)
		{
			tags[tag->getName()] = tag;
        }
		delete tag;
    }

    std::vector<Tag *> *getAllTags()		// 4J - was collection
	{
		// 4J - was return tags.values();
		std::vector<Tag *> *ret = new std::vector<Tag *>;
		
		AUTO_VAR(itEnd, tags.end());
		for( std::unordered_map<std::wstring, Tag *>::iterator it = tags.begin(); it != itEnd; it++ )
		{
			ret->push_back(it->second);
		}
        return ret;
    }

    uint8_t getId()
	{
        return TAG_Compound;
    }

    void put(const wchar_t *name, Tag *tag)
	{
        tags[name] = tag->setName(std::wstring( name ));
    }

    void putByte(const wchar_t * name, uint8_t value)
	{
        tags[name] = (new ByteTag(name,value));
    }

    void putShort(const wchar_t * name, short value)
	{
        tags[name] = (new ShortTag(name,value));
    }

    void putInt(const wchar_t * name, int value)
	{
        tags[name] = (new IntTag(name,value));
    }

    void putLong(const wchar_t * name, __int64 value)
	{
        tags[name] = (new LongTag(name,value));
    }

    void putFloat(const wchar_t * name, float value)
	{
        tags[name] = (new FloatTag(name,value));
    }

    void putDouble(const wchar_t * name, double value)
	{
        tags[name] = (new DoubleTag(name,value));
    }

    void putString(const wchar_t *name, const std::wstring& value)
	{
        tags[name] = (new StringTag(name,value));
    }

    void putByteArray(const wchar_t * name, byteArray value)
	{
        tags[name] = (new ByteArrayTag(name,value));
    }

	void putIntArray(const wchar_t * name, intArray value)
	{
		tags[name] = (new IntArrayTag(name, value));
	}

    void putCompound(const wchar_t * name, CompoundTag *value)
	{
        tags[name] = value->setName( std::wstring( name ) );
    }

    void putBoolean(const wchar_t * string, bool val)
	{
        putByte(string, val?static_cast<uint8_t>(1):static_cast<uint8_t>(0));
    }

    Tag *get(const wchar_t *name)
	{
		AUTO_VAR(it, tags.find(name));
		if(it != tags.end()) return it->second;
		return NULL;
    }
    
    bool contains(const wchar_t * name)
	{
		return tags.find(name) != tags.end();
    }

    uint8_t getByte(const wchar_t * name)
	{
        if (tags.find(name) == tags.end()) return (uint8_t)0;
        return ((ByteTag *) tags[name])->data;
    }

    short getShort(const wchar_t * name)
	{
        if (tags.find(name) == tags.end()) return (short)0;
        return ((ShortTag *) tags[name])->data;
    }

    int getInt(const wchar_t * name)
	{
        if (tags.find(name) == tags.end()) return (int)0;
        return ((IntTag *) tags[name])->data;
    }

    __int64 getLong(const wchar_t * name)
	{
        if (tags.find(name) == tags.end()) return (__int64)0;
        return ((LongTag *) tags[name])->data;
    }

    float getFloat(const wchar_t * name)
	{
        if (tags.find(name) == tags.end()) return (float)0;
        return ((FloatTag *) tags[name])->data;
    }

    double getDouble(const wchar_t * name)
	{
        if (tags.find(name) == tags.end()) return (double)0;
        return ((DoubleTag *) tags[name])->data;
    }

    std::wstring getString(const wchar_t * name)
	{
        if (tags.find(name) == tags.end()) return std::wstring( L"" );
        return ((StringTag *) tags[name])->data;
    }

    byteArray getByteArray(const wchar_t * name)
	{
        if (tags.find(name) == tags.end()) return byteArray();
        return ((ByteArrayTag *) tags[name])->data;
    }

	intArray getIntArray(const wchar_t * name)
	{
		if (tags.find(name) == tags.end()) return intArray(0);
		return ((IntArrayTag *) tags[name])->data;
	}

    CompoundTag *getCompound(const wchar_t * name)
	{
        if (tags.find(name) == tags.end()) return new CompoundTag(name);
        return (CompoundTag *) tags[name];
    }

    ListTag<Tag> *getList(const wchar_t * name)
	{
	// 4jcraft changed this function to not do a c style cast
	// of a templated class
	auto it = tags.find(name);
	if(it == tags.end()) return new ListTag<Tag>(name);
	return dynamic_cast<ListTag<Tag>*>(it->second);
    }

    bool getBoolean(const wchar_t *string)
	{
        return getByte(string) != static_cast<uint8_t>(0);
    }

	void remove(const std::wstring &name)
	{
		AUTO_VAR(it, tags.find(name));
		if(it != tags.end()) tags.erase(it);
		//tags.remove(name);
	}

    std::wstring toString()
	{
		static const int bufSize = 32;
		static wchar_t buf[bufSize];
		swprintf(buf,bufSize,L"%d entries",tags.size());
		return std::wstring( buf );
    }

    void print(char *prefix, std::ostream out)
	{
		/*
        Tag::print(prefix, out);
		out << prefix << "{" << std::endl;

		char *newPrefix = new char[ strlen(prefix) + 4 ];
		strcpy( newPrefix, prefix);
		strcat( newPrefix, "   ");
		
		AUTO_VAR(itEnd, tags.end());
		for( std::unordered_map<string, Tag *>::iterator it = tags.begin(); it != itEnd; it++ )
		{
			it->second->print(newPrefix, out);
		}
		delete[] newPrefix;
		out << prefix << "}" << std::endl;
		*/
    }

    bool isEmpty()
	{
		return tags.empty();
    }

	virtual ~CompoundTag()
	{
		AUTO_VAR(itEnd, tags.end());
		for( AUTO_VAR(it, tags.begin()); it != itEnd; it++ )
		{
			delete it->second;
		}
	}

	Tag *copy()
	{
		CompoundTag *tag = new CompoundTag(getName());
		
		AUTO_VAR(itEnd, tags.end());
		for( AUTO_VAR(it, tags.begin()); it != itEnd; it++ )
		{			
			tag->put((wchar_t *)it->first.c_str(), it->second->copy());
		}
		return tag;
	}

	bool equals(Tag *obj)
	{
		if (Tag::equals(obj))
		{
			CompoundTag *o = (CompoundTag *) obj;

			if(tags.size() == o->tags.size())
			{
				bool equal = true;
				AUTO_VAR(itEnd, tags.end());
				for( AUTO_VAR(it, tags.begin()); it != itEnd; it++ )
				{
					AUTO_VAR(itFind, o->tags.find(it->first));
					if(itFind == o->tags.end() || !it->second->equals(itFind->second) )
					{
						equal = false;
						break;
					}
				}
				return equal;
				//return tags.entrySet().equals(o.tags.entrySet());
			}
		}
		return false;
	}
};
