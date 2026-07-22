#pragma once


#include <unordered_map>

#include "TutorialTask.h"

class Tutorial;


class ControllerTask : public TutorialTask {
private:
    std::unordered_map<int, bool> completedMappings;
    std::unordered_map<int, bool> southpawCompletedMappings;
    bool m_bHasSouthpaw;
    unsigned int m_uiCompletionMask;
    int* m_iCompletionMaskA;
    int m_iCompletionMaskACount;
    bool CompletionMaskIsValid();

    
    float m_lastYaw;
    float m_lastPitch;
    bool m_initialized = false;

public:
    ControllerTask(Tutorial* tutorial, int descriptionId,
                   bool enablePreCompletion, bool showMinimumTime,
                   int mappings[], unsigned int mappingsLength,
                   int iCompletionMaskA[] = nullptr,
                   int iCompletionMaskACount = 0,
                   int iSouthpawMappings[] = nullptr,
                   unsigned int uiSouthpawMappingsCount = 0);
    ~ControllerTask();
    virtual bool isCompleted();
    virtual void setAsCurrentTask(bool active = true);
};