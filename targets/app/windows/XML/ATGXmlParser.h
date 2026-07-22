

















#pragma once
#ifndef ATGXMLPARSER_H
#define ATGXMLPARSER_H

namespace ATG {




#define _ATGFAC 0x61B
#define E_COULD_NOT_OPEN_FILE MAKE_HRESULT(1, _ATGFAC, 0x0001)
#define E_INVALID_XML_SYNTAX MAKE_HRESULT(1, _ATGFAC, 0x0002)

const uint32_t XML_MAX_ATTRIBUTES_PER_ELEMENT = 32;
const uint32_t XML_MAX_NAME_LENGTH = 128;
const uint32_t XML_READ_BUFFER_SIZE = 2048;
const uint32_t XML_WRITE_BUFFER_SIZE = 2048;





struct XMLAttribute {
    wchar_t* strName;
    uint32_t NameLen;
    wchar_t* strValue;
    uint32_t ValueLen;
};


class ISAXCallback {
    friend class XMLParser;

public:
    ISAXCallback() {};
    virtual ~ISAXCallback() {};

    virtual int32_t StartDocument() = 0;
    virtual int32_t EndDocument() = 0;

    virtual int32_t ElementBegin(const wchar_t* strName, uint32_t NameLen,
                                 const XMLAttribute* pAttributes,
                                 uint32_t NumAttributes) = 0;
    virtual int32_t ElementContent(const wchar_t* strData, uint32_t DataLen,
                                   bool More) = 0;
    virtual int32_t ElementEnd(const wchar_t* strName, uint32_t NameLen) = 0;

    virtual int32_t CDATABegin() = 0;
    virtual int32_t CDATAData(const wchar_t* strCDATA, uint32_t CDATALen,
                              bool bMore) = 0;
    virtual int32_t CDATAEnd() = 0;

    virtual void Error(int32_t hError, const char* strMessage) = 0;

    virtual void SetParseProgress(uint32_t dwProgress) {}

    const char* GetFilename() { return m_strFilename; }
    uint32_t GetLineNumber() { return m_LineNum; }
    uint32_t GetLinePosition() { return m_LinePos; }

private:
    const char* m_strFilename;
    uint32_t m_LineNum;
    uint32_t m_LinePos;
};


class XMLParser {
public:
    XMLParser();
    ~XMLParser();

    
    void RegisterSAXCallbackInterface(ISAXCallback* pISAXCallback);

    
    ISAXCallback* GetSAXCallbackInterface();

    
    
    
    
    
    

    int32_t ParseXMLFile(const char* strFilename);

    
    
    
    

    int32_t ParseXMLBuffer(const char* strBuffer, uint32_t uBufferSize);

private:
    int32_t MainParseLoop();

    int32_t AdvanceCharacter(bool bOkToFail = false);
    void SkipNextAdvance();

    int32_t ConsumeSpace();
    int32_t ConvertEscape();
    int32_t AdvanceElement();
    int32_t AdvanceName();
    int32_t AdvanceAttrVal();
    int32_t AdvanceCDATA();
    int32_t AdvanceComment();

    void FillBuffer();

#ifdef _Printf_format_string_  
    void Error(int32_t hRet,
               _In_z_ _Printf_format_string_ const char* strFormat, ...);
#else
    void Error(int32_t hRet, const char* strFormat, ...);
#endif

    ISAXCallback* m_pISAXCallback;

    void* m_hFile;
    const char* m_pInXMLBuffer;
    uint32_t m_uInXMLBufferCharsLeft;
    uint32_t m_dwCharsTotal;
    uint32_t m_dwCharsConsumed;

    uint8_t m_pReadBuf[XML_READ_BUFFER_SIZE + 2];  
    wchar_t m_pWriteBuf[XML_WRITE_BUFFER_SIZE];

    uint8_t* m_pReadPtr;
    wchar_t* m_pWritePtr;  

    bool m_bUnicode;       
    bool m_bReverseBytes;  

    bool m_bSkipNextAdvance;
    wchar_t m_Ch;  
};

}  

#endif
