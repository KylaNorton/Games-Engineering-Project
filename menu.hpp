#pragma once
#include <SFML/Graphics.hpp>
#include <array>

enum class MenuAction { None, Start, Level, Settings };

class Menu {
public:
    explicit Menu(sf::RenderWindow& window);
    void handleEvent(const sf::Event& e);
    void draw();
    MenuAction getAction() const { return action; }
    void clearAction() { action = MenuAction::None; } 

private:
    struct Button {
        sf::RectangleShape box;
        bool contains(const sf::RenderWindow& w, sf::Vector2i mp) const {
            return box.getGlobalBounds().contains(w.mapPixelToCoords(mp));
        }
    };

    void setupButton(Button& b, sf::Vector2f pos);
    void resetColors();
    void checkHover();

    sf::RenderWindow& window;
    std::array<Button, 3> buttons;
    sf::Color bgColor, idle, hover;
    MenuAction action;
};
