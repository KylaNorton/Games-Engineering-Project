#include "playerSave.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <direct.h>   // for _mkdir on Windows


using namespace std;


PlayerSave PlayerSave::activePlayer = PlayerSave("Default");

/* ----------------------------------------------------- */
PlayerSave::PlayerSave(const string& name)
    : name(name), levelsCompleted(0), highScores(5, 0) {}

/* ----------------------------------------------------- */
string PlayerSave::getFilename() const {
    return "save_files/" + name + "_save.txt";
}

/* ----------------------------------------------------- */
void PlayerSave::saveToFile() const {
    ofstream file(getFilename());
    if (!file) return;

    file << "Name: " << name << "\n";
    file << "LevelsCompleted: " << levelsCompleted << "\n";

    file << "HighScores: ";
    for (size_t i = 0; i < highScores.size(); i++) {
        file << highScores[i];
        if (i != highScores.size() - 1) file << ", ";
    }
    file << "\n";
}

/* ----------------------------------------------------- */
bool PlayerSave::loadFromFile() {
    ifstream file(getFilename());
    if (!file) return false;

    string line;

    while (getline(file, line)) {
        if (line.rfind("LevelsCompleted:", 0) == 0) {
            levelsCompleted = stoi(line.substr(17));
        }
        else if (line.rfind("HighScores:", 0) == 0) {
            highScores.clear();
            string nums = line.substr(12);
            stringstream ss(nums);
            string token;

            while (getline(ss, token, ',')) {
                highScores.push_back(stoi(token));
            }
        }
    }

    return true;
}

/* ----------------------------------------------------- */
void PlayerSave::registerInPlayerList() const {
    ofstream list("players.txt", ios::app);
    list << name << "_save.txt\n";
}

/* ----------------------------------------------------- */
vector<string> PlayerSave::loadPlayerList() {
    vector<string> list;
    ifstream file("players.txt");

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
}

/* ----------------------------------------------------- */
void PlayerSave::loadPlayerData(const string& filename) {

    string fullpath = "save_files/" + filename;
    ifstream file(fullpath);
    if (!file) return;

    cout << "\n===== PLAYER DATA =====\n";
    string line;
    while (getline(file, line))
        cout << line << "\n";
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
    _mkdir("save_files");

    // CREATE SAVE FILE
    std::string filePath = "save_files/" + playerName + "_save.txt";
    std::ofstream saveFile(filePath);

    if (!saveFile.is_open()) {
        std::cout << "ERROR: Cannot create save file at " << filePath << "\n";
        return;
    }

    saveFile << "Name:" << playerName << "\n";
    saveFile << "LevelsCompleted: 0\n";
    saveFile << "highScores: 0, 0, 0, 0, 0\n";
    saveFile.close();

    // ADD TO PLAYERS LIST
    std::ofstream playersList("players.txt", std::ios::app);
    if (playersList.is_open()) {
        playersList << playerName << "_save.txt \n";
        playersList.close();
    }

    std::cout << "Created player: " << playerName << "\n";
}

