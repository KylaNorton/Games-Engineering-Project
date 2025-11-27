#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// What the game can request for screen changes etc.
enum class GameAction { None, Back, Pause, Play, Quit };

// --- FARM TYPES ---

// State of a tile
enum class TileState { Empty, Planted, Grown };

// One soil tile in the farm
struct FarmTile {
    sf::RectangleShape rect;
    TileState state = TileState::Empty;
    float growthTimer = 0.f;
    bool walkable = false;
};

// One farmer (player or AI)
struct Farmer {
    sf::CircleShape body;
    sf::Vector2f velocity{0.f, 0.f};
    int score = 0;
};

class Game {
public:
    explicit Game(sf::RenderWindow& window);

    void handleEvent(const sf::Event& e);
    void update(float dt);
    void draw();

    GameAction getAction() const { return action; }
    void clearAction() { action = GameAction::None; }

    void setSpeed(float s) { speed = s; } // from Level page

private:
    sf::RenderWindow& window;

    // Core
    float speed = 200.f;
    bool PauseGame { false };

    // Font & HUD
    sf::Font font;
    bool hasFont = false;

    // Buttons (back / pause)
    struct IconButton {
        sf::RectangleShape box;
        sf::Sprite sprite;
    };

    IconButton backButton;
    IconButton pauseButton;

    // Info bar containers (just rectangles)
    struct InfoBoard {
        sf::RectangleShape box;
    };

    InfoBoard score;
    InfoBoard timer;
    InfoBoard board;

    // Farm grid
    std::vector<FarmTile> farm;
    int gridCols = 5;
    int gridRows = 3;
    int startRow = 1;
    int startCol = 2;
    float tileSize = 80.f;

    sf::FloatRect farmBounds;


    // Farmers
    Farmer playerFarmer;
    Farmer aiFarmer;

    // Texts for HUD
    sf::Text playerScoreText;
    sf::Text aiScoreText;
    sf::Text timerText;

    // Match timer
    float gameTimer = 60.f;

    // Action for main.cpp
    GameAction action { GameAction::None };
};
