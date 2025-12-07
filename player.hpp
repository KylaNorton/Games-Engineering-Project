#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include <vector>
#include "spriteLib.hpp" //to draw spirtes

// ------------------------------------------------------------------
// Global appearance configuration for player and AI
// ------------------------------------------------------------------
struct PlayerAppearance {
    sf::Color playerColor = sf::Color::Cyan;
    sf::Color aiColor = sf::Color::Red;

    //for the sprites:
    int playerTextureIndex = 0;
    int aiTextureIndex = 1;

};

struct SkinButton {
    sf::RectangleShape box;
    sf::Sprite icon;
    int textureIndex = 0;
    bool isForPlayer = true; // true -> affects player, false -> affects AI
    sf::Text label; // small label under the icon (e.g. "Skin 1")
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
    void recomputeLayout();

    PlayerSetAction getAction() const { return action; }
    void clearAction() { action = PlayerSetAction::None; }

    // big preview sprites 
    sf::Sprite playerPreviewSprite;
    sf::Sprite aiPreviewSprite;

    void updatePreviews();


private:
    struct Button {
        sf::RectangleShape circle;
        sf::Text label;

        bool contains(sf::RenderWindow& window, sf::Vector2i mousePos) const {
            return circle.getGlobalBounds().contains(
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
    void setupButton(Button& b, const std::string& text, sf::Vector2f p);
    void centerLabel(Button& b);
    void checkHover();
    void resetColors();

    void updateGlobalAppearance();

    std::vector<SkinButton> skinButtons;
    void createSkinButtons();
    void updateSelectionHighlight();

    // --- Previews: sprites ---
    // layout helpers (updated by recomputeLayout)
    float skinRowY = 0.f;
    float skinRowTotalWidth = 0.f;
    float skinBoxSize = 64.f;
    float skinSpacing = 20.f;
};
