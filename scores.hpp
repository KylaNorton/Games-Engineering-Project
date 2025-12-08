#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include <vector>

enum class ScoresAction { None}; 

struct ScoreEntry {
    std::string playerName;
    int score;
    
    bool operator<(const ScoreEntry& other) const {
        return score > other.score;  // Descending order
    }
};

class Scores {
public:
    explicit Scores(sf::RenderWindow& window);
    void handleEvent(const sf::Event& e);
    void draw();
    void recomputeLayout();
    ScoresAction getAction() const { return action; }
    void clearAction() { action = ScoresAction::None; } // for when you return to menu

private:
    struct Button {
        sf::RectangleShape box;
        sf::Text label; // text inside the button
        bool contains(const sf::RenderWindow& w, sf::Vector2i mp) const {
            return box.getGlobalBounds().contains(w.mapPixelToCoords(mp));
        }
    };

    void setupButton(Button& b, const std::string& text, sf::Vector2f pos);
    void resetColors();
    void checkHover();
    void centerLabel(Button& b);
    void loadBestScores();          // Load and sort all best scores from save files

    sf::RenderWindow& window;
    std::array<Button, 4> buttons;
    sf::Font font;                // <- loaded once for all labels
    bool hasFont = false;         // if false, we skip drawing text
    
    // Per-level score lists (index 0 => level 1). Each vector is sorted descending.
    std::array<std::vector<ScoreEntry>, 4> levelScores;

    // Colors
    sf::Color bgColor{30, 20, 50};
    sf::Color idle{126, 92, 210};
    sf::Color hover{146, 112, 230};
    sf::Color textColor{255, 230, 255};

    ScoresAction action{ScoresAction::None};
};