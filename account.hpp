#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include <vector>

enum class AccountAction { None, SelectPlayer, CreatePlayer}; 

class Account {
public:
    explicit Account(sf::RenderWindow& window);
    void handleEvent(const sf::Event& e);
    void draw();
    AccountAction getAction() const { return action; }
    void clearAction() { action = AccountAction::None; } // for when you return to menu
private:
    struct PlayerButton {
        sf::RectangleShape box;
        sf::Text label;
        std::string playerFilename;
        bool contains(const sf::RenderWindow& w, sf::Vector2i mp) const {
            return box.getGlobalBounds().contains(w.mapPixelToCoords(mp));
        }
    };

    struct Button {
        sf::RectangleShape box;
        sf::Text label;
        bool contains(const sf::RenderWindow& w, sf::Vector2i mp) const {
            return box.getGlobalBounds().contains(w.mapPixelToCoords(mp));
        }
    };

    void setupButton(Button& b, const std::string& text, sf::Vector2f pos);
    void setupPlayerButton(PlayerButton& pb, const std::string& name);
    void centerLabel(Button& b);
    void centerPlayerLabel(PlayerButton& pb);
    void loadPlayerButtons();
    void updateScrollPosition();

    sf::RenderWindow& window;
    std::vector<PlayerButton> playerButtons;
    Button createButton;
    sf::Font font;
    bool hasFont = false;

    // Hover / delete UI
    int hoveredIndex = -1; // index of player currently hovered, -1 if none
    const float deleteButtonWidth = 100.f;
    const float renameButtonWidth = 100.f;
    const float deleteButtonPadding = 12.f;

    // Colors
    sf::Color bgColor{30, 20, 50};
    sf::Color idle{126, 92, 210};
    sf::Color hover{146, 112, 230};
    sf::Color textColor{255, 230, 255};

    AccountAction action{AccountAction::None};

    // Scrollable list properties
    float scrollOffset = 0.f;
    const float playerButtonHeight = 60.f;
    const float playerButtonGap = 15.f;
    const float scrollSpeed = 30.f;
    const float listStartY = 100.f;
    const float listMaxHeight = 380.f;

    // Text entry mode
    bool enteringName = false;
    std::string newNameInput;
    sf::Text inputText;

    // Rename mode
    int renamingIndex = -1; // -1 if not renaming, else index of player being renamed
    std::string renameInput;
    sf::Text renameText;

    void positionPlayerLabelLeft(PlayerButton& pb);

};

