

#include "platform/ShutdownManager.h"
#include "platform/C4JThread.h"

void ShutdownManager::Initialise() {}
void ShutdownManager::StartShutdown() {}
void ShutdownManager::MainThreadHandleShutdown() {}

void ShutdownManager::HasStarted(ShutdownManager::EThreadId ) {}
void ShutdownManager::HasStarted(ShutdownManager::EThreadId ,
                                 C4JThread::EventArray* ) {}
bool ShutdownManager::ShouldRun(ShutdownManager::EThreadId ) {
    return true;
}
void ShutdownManager::HasFinished(ShutdownManager::EThreadId ) {}
