#include "scores.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

Scores::Scores(sf::RenderWindow& window) : window(window) {
     // load a font
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) {
        std::cerr << "[WARN] Font not found at res/fonts/Inter-Regular.ttf. "
                     "Buttons will show without text.\n";
    }
    
    // Load best scores from all player save files
    loadBestScores();
}

void Scores::handleEvent(const sf::Event& e) {
    // empty for now
}

void Scores::loadBestScores() {
    for (auto &v : levelScores) v.clear();
    
    // Load player list
    std::ifstream playerList("../../../players.txt");
    if (!playerList.is_open()) {
        std::cerr << "Could not open players.txt\n";
        return;
    }
    
    std::string filename;
    while (std::getline(playerList, filename)) {
        if (filename.empty()) continue;
        
        // Remove any trailing whitespace
        filename.erase(filename.find_last_not_of(" \n\r\t") + 1);
        
        // Load save file
        std::string filePath = "../../../save_files/" + filename;
        std::ifstream saveFile(filePath);
        if (!saveFile.is_open()) {
            std::cerr << "Could not open " << filePath << "\n";
            continue;
        }
        
        std::string line;
        std::string playerName = "Unknown";
        
        while (std::getline(saveFile, line)) {
            // Extract player name
            if (line.rfind("Name:", 0) == 0) {
                playerName = line.substr(5);
                // Trim leading/trailing whitespace
                playerName.erase(0, playerName.find_first_not_of(" \t\r\n"));
                playerName.erase(playerName.find_last_not_of(" \t\r\n") + 1);
                continue;
            }

            // Parse lines like "1: 100", "2: 50", etc.
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                try {
                    int levelNum = std::stoi(line.substr(0, colonPos));
                    int score = std::stoi(line.substr(colonPos + 1));
                    // levelNum should be 1-4; only add non-zero scores
                    if (levelNum > 0 && levelNum <= 4 && score > 0) {
                        levelScores[levelNum - 1].push_back({playerName, score});
                    }
                } catch (...) {
                    // Skip malformed lines
                }
            }
        }
        saveFile.close();
    }
    playerList.close();
    // Sort each level's scores in descending order
    for (auto &v : levelScores) {
        std::sort(v.begin(), v.end());
    }
}

void Scores::draw() {
    // Reload scores so the view reflects any recent save file changes
    loadBestScores();

    window.clear(sf::Color(10, 10, 30));

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // Draw title centered

    if (hasFont) {
        sf::Text title("Best Scores by Level", font, 36);
        title.setFillColor(sf::Color::White);
        sf::FloatRect tb = title.getLocalBounds();
        title.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
        title.setPosition(winW * 0.5f, winH * 0.08f);
        title.setPosition(360, 30);
        window.draw(title);

        // Column layout for 4 levels
        const float startX = 80.f;
        const float colW = 200.f;
        const float startY = 100.f;
        const float lineH = 32.f;
        
        for (int level = 0; level < 4; ++level) {
            float x = startX + level * colW;

            // Draw level header
            sf::Text header("Level " + std::to_string(level + 1), font, 24);
            header.setFillColor(sf::Color(200, 200, 255));
            header.setPosition(x, startY - 36.f);
            window.draw(header);

            const auto &vec = levelScores[level];
            if (vec.empty()) {
                sf::Text none("No scores", font, 18);
                none.setFillColor(sf::Color(180, 180, 180));
                none.setPosition(x, startY);
                window.draw(none);
                continue;
            }

            int rank = 1;
            float y = startY;
            for (const auto &entry : vec) {
                // stop drawing if we go off the bottom of the window
                if (y > static_cast<float>(window.getSize().y) - 40.f) break;

                sf::Text t(std::to_string(rank) + ". " + entry.playerName, font, 18);
                t.setFillColor(sf::Color(255, 230, 255));
                t.setPosition(x, y);
                window.draw(t);

                sf::Text s(std::to_string(entry.score), font, 18);
                s.setFillColor(sf::Color(220, 220, 255));
                // right-align the score within the column
                float sx = x + colW - 40.f;
                s.setPosition(sx, y);
                window.draw(s);

                y += lineH;
                rank++;
            }
        }
    }


    window.display();
}

void Scores::recomputeLayout() {
    // Nothing persistent to cache; draw() computes positions each frame.
}
