#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include <vector>

// ------------------------------------------------------------------
// Global appearance configuration for player and AI
// ------------------------------------------------------------------
struct PlayerAppearance {
    sf::Color playerColor = sf::Color::Cyan;
    sf::Color aiColor     = sf::Color::Red;

    // If later we use sprites:
    int playerTextureIndex = 0;
    int aiTextureIndex     = 0;
};

// This is the global instance (declared extern here, defined in player.cpp)
extern PlayerAppearance gAppearance;

// What actions the PlayerSettings screen can trigger
enum class PlayerSetAction { None, Back };

// ------------------------------------------------------------------
// PlayerSettings screen (separate from Settings)
// ------------------------------------------------------------------
class PlayerSettings {
public:
    explicit PlayerSettings(sf::RenderWindow& window);

    void handleEvent(const sf::Event& e);
    void draw();
    void applySettings(); // (for now this will just update gAppearance)

    PlayerSetAction getAction() const { return action; }
    void clearAction() { action = PlayerSetAction::None; }

private:
    struct Button {
        sf::RectangleShape box;
        sf::Text label;

        bool contains(sf::RenderWindow& window, sf::Vector2i mousePos) const {
            return box.getGlobalBounds().contains(
                static_cast<float>(mousePos.x),
                static_cast<float>(mousePos.y)
            );
        }
    };

    sf::RenderWindow& window;

    sf::Font font;
    bool hasFont = false;

    // you can reduce/increase the number depending on how many buttons you want
    std::array<Button, 1> buttons;  // e.g. just a "Back" or "Apply" button
    PlayerSetAction action { PlayerSetAction::None };

    sf::Color idle     { 80, 80, 120 };
    sf::Color hover    {120, 120, 180};
    sf::Color textColor{230, 230, 230};
    sf::Color bgColor  {20, 40, 60};

    // Visual customization
    sf::RectangleShape playerColorBox;
    sf::RectangleShape aiColorBox;

    std::vector<sf::Color> palette;
    int playerColorIndex = 0;
    int aiColorIndex     = 1;   // different from player by default

    void setupButton(Button& b, const std::string& text, sf::Vector2f p);
    void centerLabel(Button& b);
    void checkHover();
    void resetColors();

    void initPalette();
    void applyAppearanceFromGlobal();
    void updateGlobalAppearance();
};
