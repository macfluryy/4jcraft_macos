#include "ServerScoreboard.h"

class Packet;

ServerScoreboard::ServerScoreboard(MinecraftServer* server) {
    this->server = server;
}

MinecraftServer* ServerScoreboard::getServer() { return server; }

void ServerScoreboard::onScoreChanged(Score* score) {
    

    
    
    
    
    

    
}

void ServerScoreboard::onPlayerRemoved(const std::wstring& player) {
    
    
    
}

void ServerScoreboard::setDisplayObjective(int slot, Objective* objective) {
    

    

    
    
    
    
    
    
    
    
    
    
    
    
    

    
    
    
    
    
    
    
    
    
    
    
    
    

    
}

void ServerScoreboard::addPlayerToTeam(const std::wstring& player,
                                       PlayerTeam* team) {
    

    
    
    

    
}

void ServerScoreboard::removePlayerFromTeam(const std::wstring& player,
                                            PlayerTeam* team) {
    

    
    
    

    
}

void ServerScoreboard::onObjectiveAdded(Objective* objective) {
    
    
}

void ServerScoreboard::onObjectiveChanged(Objective* objective) {
    

    
    
    
    
    

    
}

void ServerScoreboard::onObjectiveRemoved(Objective* objective) {
    

    
    
    
    

    
}

void ServerScoreboard::onTeamAdded(PlayerTeam* team) {
    

    
    

    
}

void ServerScoreboard::onTeamChanged(PlayerTeam* team) {
    

    
    

    
}

void ServerScoreboard::onTeamRemoved(PlayerTeam* team) {
    

    
    

    
}

void ServerScoreboard::setSaveData(ScoreboardSaveData* data) {
    
}

void ServerScoreboard::setDirty() {
    
    
    
    
}

std::vector<std::shared_ptr<Packet> >*
ServerScoreboard::getStartTrackingPackets(Objective* objective) {
    return nullptr;

    
    
    

    
    
    
    
    
    

    
    
    
    
    

    
}

void ServerScoreboard::startTrackingObjective(Objective* objective) {
    
    

    
    
    
    
    
    
    

    
}

std::vector<std::shared_ptr<Packet> >* ServerScoreboard::getStopTrackingPackets(
    Objective* objective) {
    return nullptr;

    
    
    

    
    
    
    
    

    
}

void ServerScoreboard::stopTrackingObjective(Objective* objective) {
    

    
    
    
    
    
    
    

    
}

int ServerScoreboard::getObjectiveDisplaySlotCount(Objective* objective) {
    return 0;
    

    
    
    
    

    
}