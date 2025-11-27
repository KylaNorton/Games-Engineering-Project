#pragma once
#include <SFML/Graphics.hpp>

enum class Difficulty { Easy=0, Medium=1, Hard=2, None=3 };

class LevelPage {
public:
    explicit LevelPage(sf::RenderWindow& window);
    void handleEvent(const sf::Event& e);
    void draw();
    Difficulty chosen() const { return choice; }
    void reset() { choice = Difficulty::None; }
    struct Btn { sf::RectangleShape box; sf::Text label; };

private:
    sf::RenderWindow& window;
    sf::Font font;
    bool hasFont = false;
    Btn easy, medium, hard;
    sf::Color idle{90, 90, 140}, hover{120, 120, 180}, text{255,255,255};
    Difficulty choice{Difficulty::None};
};

