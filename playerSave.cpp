#include "playerSave.hpp"
#include "player.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <direct.h>   // for _mkdir on Windows


using namespace std;


PlayerSave PlayerSave::activePlayer = PlayerSave("Default");

/* ----------------------------------------------------- */
PlayerSave::PlayerSave(const string& name)
        : name(name), levelsCompleted(0), highScores(5, 0),
            playerTextureIndex(0), aiTextureIndex(1),
            playerColor(sf::Color::Cyan), aiColor(sf::Color::Red) {}

/* ----------------------------------------------------- */
string PlayerSave::getFilename() const {
    return "../../../save_files/" + name + "_save.txt";
}

/* ----------------------------------------------------- */
void PlayerSave::saveToFile() const {
    ofstream file(getFilename());
    if (!file) return;

    file << "Name: " << name << "\n";
    
    // Write scores for each level (1-indexed)
    for (size_t i = 0; i < highScores.size(); i++) {
        file << (i + 1) << ": " << highScores[i] << "\n";
    }
    // Write appearance data (sprites-only)
    file << "PlayerTextureIndex: " << playerTextureIndex << "\n";
    file << "AITextureIndex: " << aiTextureIndex << "\n";
    file << "PlayerColor: " << static_cast<int>(playerColor.r) << " " << static_cast<int>(playerColor.g) << " " << static_cast<int>(playerColor.b) << "\n";
    file << "AIColor: " << static_cast<int>(aiColor.r) << " " << static_cast<int>(aiColor.g) << " " << static_cast<int>(aiColor.b) << "\n";
}

/* ----------------------------------------------------- */
bool PlayerSave::loadFromFile() {
    ifstream file(getFilename());
    if (!file) return false;

    highScores.clear();
    string line;

    while (getline(file, line)) {
        if (line.rfind("Name:", 0) == 0) {
            continue;
        }

        // Appearance fields
        if (line.rfind("PlayerTextureIndex:", 0) == 0) {
            try { playerTextureIndex = stoi(line.substr(line.find(':') + 1)); } catch(...) {}
            continue;
        }
        if (line.rfind("AITextureIndex:", 0) == 0) {
            try { aiTextureIndex = stoi(line.substr(line.find(':') + 1)); } catch(...) {}
            continue;
        }
        if (line.rfind("PlayerColor:", 0) == 0) {
            std::istringstream iss(line.substr(line.find(':') + 1));
            int r,g,b; if (iss >> r >> g >> b) playerColor = sf::Color(r,g,b);
            continue;
        }
        if (line.rfind("AIColor:", 0) == 0) {
            std::istringstream iss(line.substr(line.find(':') + 1));
            int r,g,b; if (iss >> r >> g >> b) aiColor = sf::Color(r,g,b);
            continue;
        }

        // Parse lines like "1: 100", "2: 50", etc.
        size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            try {
                int levelNum = stoi(line.substr(0, colonPos));
                int score = stoi(line.substr(colonPos + 1));
                // Ensure the vector is large enough
                while ((int)highScores.size() < levelNum) {
                    highScores.push_back(0);
                }
                if (levelNum > 0) {
                    highScores[levelNum - 1] = score; // Store at 0-based index
                }
            } catch (...) {
                // Skip malformed lines
            }
        }
    }

    return true;
}

/* ----------------------------------------------------- */
void PlayerSave::registerInPlayerList() const {
    ofstream list("../../../players.txt", ios::app);
    list << name << "_save.txt\n";
}

/* ----------------------------------------------------- */
vector<string> PlayerSave::loadPlayerList() {
    vector<string> list;
    ifstream file("../../../players.txt");

    string line;
    while (getline(file, line)) {
        if (!line.empty())
            list.push_back(line);
    }
    return list;
}

/* ----------------------------------------------------- */
void PlayerSave::setActivePlayer(const PlayerSave& ps) {
    activePlayer = ps;
    // Load the player's file immediately so `activePlayer` contains
    // the persisted high scores and other data.
    activePlayer.loadFromFile();
    // Apply loaded appearance to global appearance
    activePlayer.applyLoadedData();
}

/* ----------------------------------------------------- */
void PlayerSave::loadPlayerData(const string& filename) {

    string fullpath = "../../../save_files/" + filename;
    ifstream file(fullpath);
    if (!file) return;

    cout << "\n===== PLAYER DATA =====\n";
    string line;
    while (getline(file, line))
        cout << line << "\n";
}

/* ----------------------------------------------------- */
void PlayerSave::applyLoadedData()
{
    // Transfer appearance from activePlayer to global gAppearance
    gAppearance.playerTextureIndex = activePlayer.playerTextureIndex;
    // AI texture index is fixed (set at startup), do not override it from save
    gAppearance.playerColor        = activePlayer.playerColor;
    // AI color not configurable by player; keep default
}

/* -----------------------------------------------------
         PLAYER SELECTION SCREEN
   ----------------------------------------------------- */
struct PlayerButton {
    sf::RectangleShape box;
    sf::Text text;
    string filename;

    bool contains(sf::RenderWindow& window, sf::Vector2i mp) {
        return box.getGlobalBounds().contains(window.mapPixelToCoords(mp));
    }
};

std::string PlayerSave::showPlayerSelection(sf::RenderWindow& window) {

    vector<string> players = loadPlayerList();
    if (players.empty()) {
        cout << "(No players found)\n";
        return "";
    }

    // Load font
    sf::Font font;
    font.loadFromFile("arial.ttf");

    // Build SFML buttons
    vector<PlayerButton> buttons;
    float y = 100;

    for (auto& filename : players) {
        PlayerButton pb;
        pb.filename = filename;

        pb.box.setSize({300.f, 50.f});
        pb.box.setPosition(330.f, y);
        pb.box.setFillColor(sf::Color(90, 60, 160));

        pb.text.setFont(font);
        pb.text.setString(filename);
        pb.text.setCharacterSize(22);
        pb.text.setFillColor(sf::Color::White);
        pb.text.setPosition(340.f, y + 10);

        buttons.push_back(pb);
        y += 70;
    }

    // Selection loop
    while (window.isOpen()) {

        sf::Event ev;
        while (window.pollEvent(ev)) {

            if (ev.type == sf::Event::Closed)
                window.close();

            if (ev.type == sf::Event::KeyPressed && ev.key.code == sf::Keyboard::Escape)
                return "";    // user canceled

            if (ev.type == sf::Event::MouseButtonPressed &&
                ev.mouseButton.button == sf::Mouse::Left) {

                sf::Vector2i mouse = sf::Mouse::getPosition(window);

                for (auto& b : buttons) {
                    if (b.contains(window, mouse)) {
                        return b.filename;   // <--- RETURN SELECTED PLAYER
                    }
                }
            }
        }

        window.clear(sf::Color(30, 20, 50));
        for (auto& b : buttons) {
            window.draw(b.box);
            window.draw(b.text);
        }
        window.display();
    }

    return "";
}


void PlayerSave::createNewPlayer(const std::string& playerName)
{
    if (playerName.empty()) {
        std::cout << "No name entered.\n";
        return;
    }

    // Ensure directory exists (works on Windows)
    _mkdir("../../../save_files");

    // CREATE SAVE FILE
    std::string filePath = "../../../save_files/" + playerName + "_save.txt";
    std::ofstream saveFile(filePath);

    if (!saveFile.is_open()) {
        std::cout << "ERROR: Cannot create save file at " << filePath << "\n";
        return;
    }

    saveFile << "Name: " << playerName << "\n";
    saveFile << "1: 0\n";
    saveFile << "2: 0\n";
    saveFile << "3: 0\n";
    saveFile << "4: 0\n";
    // Default appearance (sprites-only)
    saveFile << "PlayerTextureIndex: 0\n";
    saveFile << "AITextureIndex: 1\n";
    saveFile << "PlayerColor: " << static_cast<int>(sf::Color::Cyan.r) << " " << static_cast<int>(sf::Color::Cyan.g) << " " << static_cast<int>(sf::Color::Cyan.b) << "\n";
    saveFile << "AIColor: " << static_cast<int>(sf::Color::Red.r) << " " << static_cast<int>(sf::Color::Red.g) << " " << static_cast<int>(sf::Color::Red.b) << "\n";
    saveFile.close();

    // ADD TO PLAYERS LIST
    std::ofstream playersList("../../../players.txt", std::ios::app);
    if (playersList.is_open()) {
        playersList << playerName << "_save.txt\n";
        playersList.close();
    }

    std::cout << "Created player: " << playerName << "\n";
}

