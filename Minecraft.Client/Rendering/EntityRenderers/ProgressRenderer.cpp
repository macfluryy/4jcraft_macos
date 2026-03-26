#include "../../Platform/stdafx.h"
#include "../Tesselator.h"
#include "../../Textures/Textures.h"
#include "ProgressRenderer.h"
#include "../../../Minecraft.World/Platform/System.h"

CRITICAL_SECTION ProgressRenderer::s_progress;

ProgressRenderer::ProgressRenderer(Minecraft* minecraft) {
    status = -1;
    title = -1;
    lastTime = System::currentTimeMillis();
    noAbort = false;
    this->minecraft = minecraft;
    this->m_eType = eProgressStringType_ID;
}

void ProgressRenderer::progressStart(int title) {
    noAbort = false;
    _progressStart(title);
}

void ProgressRenderer::progressStartNoAbort(int string) {
    noAbort = true;
    _progressStart(string);
}

void ProgressRenderer::_progressStart(int title) {
    // 4J Stu - Removing all progressRenderer rendering. This will be replaced
    // on the xbox
    if (!minecraft->running) {
        if (noAbort) return;
        //        throw new StopGameException();		// 4J - removed
    }

    EnterCriticalSection(&ProgressRenderer::s_progress);
    lastPercent = 0;
    this->title = title;
    LeaveCriticalSection(&ProgressRenderer::s_progress);

}

void ProgressRenderer::progressStage(int status) {
    if (!minecraft->running) {
        if (noAbort) return;
        //        throw new StopGameException();		// 4J - removed
    }

    lastTime = 0;
    EnterCriticalSection(&ProgressRenderer::s_progress);
    setType(eProgressStringType_ID);
    this->status = status;
    LeaveCriticalSection(&ProgressRenderer::s_progress);
    progressStagePercentage(-1);
    lastTime = 0;
}

void ProgressRenderer::progressStagePercentage(int i) {
    // 4J Stu - Removing all progressRenderer rendering. This will be replaced
    // on the xbox
    EnterCriticalSection(&ProgressRenderer::s_progress);
    lastPercent = i;
    LeaveCriticalSection(&ProgressRenderer::s_progress);

}

int ProgressRenderer::getCurrentPercent() {
    int returnValue = 0;
    EnterCriticalSection(&ProgressRenderer::s_progress);
    returnValue = lastPercent;
    LeaveCriticalSection(&ProgressRenderer::s_progress);
    return returnValue;
}

int ProgressRenderer::getCurrentTitle() {
    EnterCriticalSection(&ProgressRenderer::s_progress);
    int returnValue = title;
    LeaveCriticalSection(&ProgressRenderer::s_progress);
    return returnValue;
}

int ProgressRenderer::getCurrentStatus() {
    EnterCriticalSection(&ProgressRenderer::s_progress);
    int returnValue = status;
    LeaveCriticalSection(&ProgressRenderer::s_progress);
    return returnValue;
}

ProgressRenderer::eProgressStringType ProgressRenderer::getType() {
    EnterCriticalSection(&ProgressRenderer::s_progress);
    eProgressStringType returnValue = m_eType;
    LeaveCriticalSection(&ProgressRenderer::s_progress);
    return returnValue;
}

void ProgressRenderer::setType(eProgressStringType eType) {
    EnterCriticalSection(&ProgressRenderer::s_progress);
    m_eType = eType;
    LeaveCriticalSection(&ProgressRenderer::s_progress);
}

void ProgressRenderer::progressStage(std::wstring& wstrText) {
    EnterCriticalSection(&ProgressRenderer::s_progress);
    m_wstrText = wstrText;
    setType(eProgressStringType_String);
    LeaveCriticalSection(&ProgressRenderer::s_progress);
}

std::wstring& ProgressRenderer::getProgressString(void) {
    EnterCriticalSection(&ProgressRenderer::s_progress);
    std::wstring& temp = m_wstrText;
    LeaveCriticalSection(&ProgressRenderer::s_progress);
    return temp;
}
