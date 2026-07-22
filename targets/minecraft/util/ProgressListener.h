#pragma once

class ProgressListener {
public:
    
    virtual void progressStagePercentage(int p) = 0;
    virtual void progressStart(int stringId) = 0;
    virtual void progressStartNoAbort(int stringId) = 0;
    virtual void progressStage(int stringId) = 0;

    
    virtual void progressStage(std::wstring& wstrText) = 0;
};