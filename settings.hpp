#pragma once
#include <SFML/Graphics.hpp>

class SettingsPage {
public:
    explicit SettingsPage(sf::RenderWindow& window);
    void handleEvent(const sf::Event& e);
    void draw();
    bool showFps() const { return fpsOn; }
private:
    sf::RenderWindow& window;
    sf::Font font; bool hasFont=false;
    sf::Text label;
    sf::RectangleShape toggleBox, knob;
    bool fpsOn = false;
};
