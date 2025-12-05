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
    bestScores.clear();
    
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
                playerName.erase(0, playerName.find_first_not_of(" \t"));
                playerName.erase(playerName.find_last_not_of(" \t") + 1);
            }
            
            // Parse lines like "1: 100", "2: 50", etc.
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                try {
                    int levelNum = std::stoi(line.substr(0, colonPos));
                    int score = std::stoi(line.substr(colonPos + 1));
                    // levelNum should be 1-4; only add non-zero scores
                    if (levelNum > 0 && levelNum <= 4 && score > 0) {
                        bestScores.push_back({playerName, score});
                    }
                } catch (...) {
                    // Skip malformed lines
                }
            }
        }
        saveFile.close();
    }
    playerList.close();
    
    // Sort scores in descending order
    std::sort(bestScores.begin(), bestScores.end());
}

void Scores::draw() {
    window.clear(sf::Color(10, 10, 30));
    
    // Draw title
    if (hasFont) {
        sf::Text title("Best Scores", font, 40);
        title.setFillColor(sf::Color::White);
        title.setPosition(400, 50);
        window.draw(title);
        
        // Draw score entries
        float yPos = 150;
        int rank = 1;
        
        for (const auto& entry : bestScores) {
            if (yPos > 700) break;  // Avoid drawing off-screen
            
            sf::Text scoreText(
                std::to_string(rank) + ". " + entry.playerName + " - " + std::to_string(entry.score),
                font,
                24
            );
            scoreText.setFillColor(sf::Color(255, 230, 255));
            scoreText.setPosition(150, yPos);
            window.draw(scoreText);
            
            yPos += 40;
            rank++;
        }
        
        // Draw "no scores" message if empty
        if (bestScores.empty()) {
            sf::Text noScores("No scores yet!", font, 24);
            noScores.setFillColor(sf::Color(200, 200, 200));
            noScores.setPosition(400, 350);
            window.draw(noScores);
        }
    }
    
    window.display();
}
