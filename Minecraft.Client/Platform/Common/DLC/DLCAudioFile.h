#pragma once
#include "DLCFile.h"

class DLCAudioFile : public DLCFile
{

public:

	// If you add to the Enum,then you need to add the array of type names
	// These are the names used in the XML for the parameters
	enum EAudioType
	{
		e_AudioType_Invalid = -1,

		e_AudioType_Overworld = 0,
		e_AudioType_Nether,
		e_AudioType_End,

		e_AudioType_Max,
	};
	enum EAudioParameterType
	{
		e_AudioParamType_Invalid = -1,

		e_AudioParamType_Cuename = 0,
		e_AudioParamType_Credit,

		e_AudioParamType_Max,

	};
	static const WCHAR *wchTypeNamesA[e_AudioParamType_Max];

	DLCAudioFile(const std::wstring &path);

	virtual void addData(PBYTE pbData, DWORD dwBytes);
	virtual PBYTE getData(DWORD &dwBytes);

	bool processDLCDataFile(PBYTE pbData, DWORD dwLength);
	int GetCountofType(DLCAudioFile::EAudioType ptype);
	std::wstring &GetSoundName(int iIndex);

private:
	using DLCFile::addParameter;

	PBYTE m_pbData;
	DWORD m_dwBytes;
	static const int CURRENT_AUDIO_VERSION_NUM=1;
	//std::unordered_map<int, std::wstring> m_parameters;
	std::vector<std::wstring> m_parameters[e_AudioType_Max];

	// use the EAudioType to order these
	void addParameter(DLCAudioFile::EAudioType type, DLCAudioFile::EAudioParameterType ptype, const std::wstring &value);
	DLCAudioFile::EAudioParameterType getParameterType(const std::wstring &paramName);
};
