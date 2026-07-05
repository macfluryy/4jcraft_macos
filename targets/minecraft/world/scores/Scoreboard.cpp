#include "Scoreboard.h"

#include "minecraft/world/scores/Objective.h"
#include "minecraft/world/scores/PlayerTeam.h"
#include "minecraft/world/scores/Score.h"

class Player;

Objective* Scoreboard::getObjective(const std::wstring& name) {
    auto it = objectivesByName.find(name);
    return it == objectivesByName.end() ? nullptr : it->second;
}

Objective* Scoreboard::addObjective(const std::wstring& name,
                                    ObjectiveCriteria* criteria) {
    Objective* objective = getObjective(name);
    if (objective != nullptr) {
        // Java throws here; a remote server resending a name is survivable,
        // so treat it as a lookup instead.
        return objective;
    }

    objective = new Objective(this, name, criteria);

    if (criteria != nullptr) {
        std::vector<Objective*>*& criteriaList = objectivesByCriteria[criteria];
        if (criteriaList == nullptr)
            criteriaList = new std::vector<Objective*>();
        criteriaList->push_back(objective);
    }
    objectivesByName[name] = objective;
    onObjectiveAdded(objective);

    return objective;
}

std::vector<Objective*>* Scoreboard::findObjectiveFor(
    ObjectiveCriteria* criteria) {
    return nullptr;
    // vector<Objective *> *objectives =
    // objectivesByCriteria.find(criteria)->second;

    // return objectives == nullptr ? new vector<Objective *>() : new
    // vector<Objective *>(objectives);
}

Score* Scoreboard::getPlayerScore(const std::wstring& name,
                                  Objective* objective) {
    std::unordered_map<Objective*, Score*>& scores = playerScores[name];
    Score*& score = scores[objective];
    if (score == nullptr) {
        score = new Score(this, objective, name);
        onScoreChanged(score);
    }
    return score;
}

std::vector<Score*>* Scoreboard::getPlayerScores(Objective* objective) {
    return nullptr;
    // vector<Score *> *result = new vector<Score *>();

    // for (Map<Objective, Score> scores : playerScores.values())
    //{
    //	Score score = scores.get(objective);
    //	if (score != null) result.add(score);
    // }

    // Collections.sort(result, Score.SCORE_COMPARATOR);

    // return result;
}

std::vector<Objective*>* Scoreboard::getObjectives() {
    return nullptr;
    // return objectivesByName.values();
}

std::vector<std::wstring>* Scoreboard::getTrackedPlayers() {
    return nullptr;
    // return playerScores.keySet();
}

void Scoreboard::resetPlayerScore(const std::wstring& player) {
    auto it = playerScores.find(player);
    if (it != playerScores.end()) {
        for (auto& entry : it->second) delete entry.second;
        playerScores.erase(it);
        onPlayerRemoved(player);
    }
}

std::vector<Score*>* Scoreboard::getScores() {
    return nullptr;
    // Collection<Map<Objective, Score>> values = playerScores.values();
    // List<Score> result = new ArrayList<Score>();

    // for (Map<Objective, Score> map : values)
    //{
    //	result.addAll(map.values());
    // }

    // return result;
}

std::vector<Score*>* Scoreboard::getScores(Objective* objective) {
    std::vector<Score*>* result = new std::vector<Score*>();
    for (auto& player : playerScores) {
        auto it = player.second.find(objective);
        if (it != player.second.end() && it->second != nullptr)
            result->push_back(it->second);
    }
    return result;
}

std::unordered_map<Objective*, Score*>* Scoreboard::getPlayerScores(
    const std::wstring& player) {
    return nullptr;
    // Map<Objective, Score> result = playerScores.get(player);
    // if (result == null) result = new HashMap<Objective, Score>();
    // return result;
}

void Scoreboard::removeObjective(Objective* objective) {
    if (objective == nullptr) return;
    objectivesByName.erase(objective->getName());

    for (int i = 0; i < DISPLAY_SLOTS; i++) {
        if (getDisplayObjective(i) == objective)
            setDisplayObjective(i, nullptr);
    }

    auto critIt = objectivesByCriteria.find(objective->getCriteria());
    if (critIt != objectivesByCriteria.end() && critIt->second != nullptr) {
        std::vector<Objective*>& list = *critIt->second;
        for (auto it = list.begin(); it != list.end(); ++it) {
            if (*it == objective) {
                list.erase(it);
                break;
            }
        }
    }

    for (auto& player : playerScores) {
        auto it = player.second.find(objective);
        if (it != player.second.end()) {
            delete it->second;
            player.second.erase(it);
        }
    }

    onObjectiveRemoved(objective);
    delete objective;
}

void Scoreboard::setDisplayObjective(int slot, Objective* objective) {
    if (slot < 0 || slot >= DISPLAY_SLOTS) return;
    displayObjectives[slot] = objective;
    ++m_revision;
}

Objective* Scoreboard::getDisplayObjective(int slot) {
    if (slot < 0 || slot >= DISPLAY_SLOTS) return nullptr;
    return displayObjectives[slot];
}

PlayerTeam* Scoreboard::getPlayerTeam(const std::wstring& name) {
    auto it = teamsByName.find(name);
    return it == teamsByName.end() ? nullptr : it->second;
}

PlayerTeam* Scoreboard::addPlayerTeam(const std::wstring& name) {
    PlayerTeam* team = getPlayerTeam(name);
    if (team != nullptr) {
        // Java throws; a remote server resending a create is survivable, so
        // treat it as a lookup instead.
        return team;
    }

    team = new PlayerTeam(this, name);
    teamsByName[name] = team;
    onTeamAdded(team);

    return team;
}

void Scoreboard::removePlayerTeam(PlayerTeam* team) {
    if (team == nullptr) return;
    teamsByName.erase(team->getName());

    for (const std::wstring& player : *team->getPlayers()) {
        teamsByPlayer.erase(player);
    }

    onTeamRemoved(team);
    delete team;
}

void Scoreboard::addPlayerToTeam(const std::wstring& player, PlayerTeam* team) {
    if (team == nullptr) return;
    if (getPlayersTeam(player) != nullptr) {
        removePlayerFromTeam(player);
    }

    teamsByPlayer[player] = team;
    team->getPlayers()->insert(player);
    onTeamChanged(team);
}

bool Scoreboard::removePlayerFromTeam(const std::wstring& player) {
    PlayerTeam* team = getPlayersTeam(player);

    if (team != nullptr) {
        removePlayerFromTeam(player, team);
        return true;
    }
    return false;
}

void Scoreboard::removePlayerFromTeam(const std::wstring& player,
                                      PlayerTeam* team) {
    // Java throws when the player is on a different team; tolerate it and
    // only detach when the membership actually matches.
    if (team == nullptr || getPlayersTeam(player) != team) return;

    teamsByPlayer.erase(player);
    team->getPlayers()->erase(player);
    onTeamChanged(team);
}

std::vector<std::wstring>* Scoreboard::getTeamNames() {
    std::vector<std::wstring>* result = new std::vector<std::wstring>();
    result->reserve(teamsByName.size());
    for (auto& entry : teamsByName) result->push_back(entry.first);
    return result;
}

std::vector<PlayerTeam*>* Scoreboard::getPlayerTeams() {
    std::vector<PlayerTeam*>* result = new std::vector<PlayerTeam*>();
    result->reserve(teamsByName.size());
    for (auto& entry : teamsByName) result->push_back(entry.second);
    return result;
}

std::shared_ptr<Player> Scoreboard::getPlayer(const std::wstring& name) {
    return nullptr;
    // return MinecraftServer.getInstance().getPlayers().getPlayer(name);
}

PlayerTeam* Scoreboard::getPlayersTeam(const std::wstring& name) {
    auto it = teamsByPlayer.find(name);
    return it == teamsByPlayer.end() ? nullptr : it->second;
}

void Scoreboard::onObjectiveAdded(Objective* objective) { ++m_revision; }

void Scoreboard::onObjectiveChanged(Objective* objective) { ++m_revision; }

void Scoreboard::onObjectiveRemoved(Objective* objective) { ++m_revision; }

void Scoreboard::onScoreChanged(Score* score) { ++m_revision; }

void Scoreboard::onPlayerRemoved(const std::wstring& player) { ++m_revision; }

void Scoreboard::onTeamAdded(PlayerTeam* team) { ++m_revision; }

void Scoreboard::onTeamChanged(PlayerTeam* team) { ++m_revision; }

void Scoreboard::onTeamRemoved(PlayerTeam* team) { ++m_revision; }

std::wstring Scoreboard::getDisplaySlotName(int slot) {
    switch (slot) {
        case DISPLAY_SLOT_LIST:
            return L"list";
        case DISPLAY_SLOT_SIDEBAR:
            return L"sidebar";
        case DISPLAY_SLOT_BELOW_NAME:
            return L"belowName";
        default:
            return L"";
    }
}

int Scoreboard::getDisplaySlotByName(const std::wstring& name) {
    return -1;
    // if (name.equalsIgnoreCase("list"))
    //{
    //	return DISPLAY_SLOT_LIST;
    // }
    // else if (name.equalsIgnoreCase("sidebar"))
    //{
    //	return DISPLAY_SLOT_SIDEBAR;
    // }
    // else if (name.equalsIgnoreCase("belowName"))
    //{
    //	return DISPLAY_SLOT_BELOW_NAME;
    // }
    // else
    //{
    //	return -1;
    // }
}