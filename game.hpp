#pragma once
#include <SFML/Graphics.hpp>

enum class GameAction { None, Back, Pause, Play, Quit };

class Game {
public:
    explicit Game(sf::RenderWindow& window);
    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw();
    GameAction getAction() const { return action; }
    void clearAction() { action = GameAction::None; } // for when you return to menu


    void setSpeed(float s) { speed = s; } // from Level page
private:
    sf::RenderWindow& window;
    sf::CircleShape player; // minimal player
    float speed = 200.f;
    sf::Font font;                // <-- loaded once for all labels
    bool hasFont = false;
    bool PauseGame { false }; // shows a popup page during the pause

    struct IconButton {
        sf::RectangleShape box;
        sf::Sprite sprite; //image on the box
    };

    IconButton backButton; //arrow to go back to menu
    IconButton pauseButton; //play/pause button

    struct InfoBoard {
        sf::RectangleShape box;
    };

    InfoBoard score;
    InfoBoard timer;
    InfoBoard board;

    GameAction action{GameAction::None};
};
