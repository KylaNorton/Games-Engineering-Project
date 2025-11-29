#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class PlayerSave {

public:
    PlayerSave(const std::string& name);

    void saveToFile() const;
    void registerInPlayerList() const;

    bool loadFromFile();              // NEW
    void applyLoadedData();           // NEW — updates global variables after selecting player

    std::string getFilename() const;

    static std::vector<std::string> loadPlayerList();
    static std::string showPlayerSelection(sf::RenderWindow& window);


    static void loadPlayerData(const std::string& filename);
    static void setActivePlayer(const PlayerSave& ps); // NEW

    static void createNewPlayer(const std::string& name);


public:
    std::string name;
    int levelsCompleted;
    std::vector<int> highScores;

    static PlayerSave activePlayer;   // CURRENT PLAYER
};
