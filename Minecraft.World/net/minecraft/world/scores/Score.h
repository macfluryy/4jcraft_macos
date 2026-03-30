#pragma once

class Scoreboard;
class Objective;

class Score {
public:
    // 4J Not converted

private:
    Scoreboard* scoreboard;
    Objective* objective;
    std::wstring owner;
    int count;

public:
    Score(Scoreboard* scoreboard, Objective* objective,
          const std::wstring& owner);

    void add(int count);
    void remove(int count);
    void increment();
    void decrement();
    int getScore();
    void setScore(int score);
    Objective* getObjective();
    std::wstring getOwner();
    Scoreboard* getScoreboard();
    void updateFor(std::vector<std::shared_ptr<Player> >* players);
};