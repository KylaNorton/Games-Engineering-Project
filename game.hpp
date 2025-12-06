#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

//different game screen changes
enum class GameAction { None, Back, Play};

//crop types
enum class CropType { None, Carrot, Tomato, Lettuce, Corn, Potato };

// State of a tile
enum class TileState { Empty, Grown, Seeded, Watered, Suned, Marketed };

enum class ActionType { None, Plant, Harvest, TakeSeed, TakeWater, TakeSun, DropWater, DropSun, DropProduct };

enum class GroundType { Empty, Soil, Wall, Market, Seeds, Water, Sun, Trash };

// --- Add near the Farmer / Request definitions in game.hpp ---
// AI State machine for the AI farmer
enum class AIState {
    SelectGoal,
    GoToSeeds,
    PickSeeds,
    GoToPlant,
    Plant,
    WaitForGrowth,
    Harvest,
    GoToMarket,
    Sell,
    Idle
};


// One soil tile in the farm
struct FarmTile {
    sf::RectangleShape rect;
    TileState state = TileState::Empty;
    GroundType type = GroundType::Empty;
    CropType crop = CropType::None; 
    float growthTimer = 0.f;
    bool walkable = false;
};

// One farmer (player or AI)
struct Farmer {
    sf::CircleShape body;
    sf::Vector2f velocity{0.f, 0.f};
    int score = 0;
    bool hasSeed = false;
    bool hasWater = false;
    bool hasSun = false;
    bool hasProduct = false;
    CropType carriedSeed = CropType::None;
};

// One market request: up to 3 different crops with quantities
struct Request {
    std::vector<std::pair<CropType, int>> items;  // (crop, quantity)
    bool completed = false;
};


class Game {
public:
    explicit Game(sf::RenderWindow& window, int levelID = 1);

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
    bool Tutorial { false };

    sf::RectangleShape leftField;
    sf::RectangleShape rightField;
    sf::RectangleShape topBar;
    sf::RectangleShape bottomBar;
    sf::RectangleShape leftBar;
    sf::RectangleShape rightBar;
    sf::RectangleShape centerPath;

    //Winner options
    bool EndGame {false};
    enum class Winner {None, AI, Player, Tie};
    Winner winner {Winner::None};

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
    int gridCols = 12;
    int gridRows = 6;
    sf::Vector2f gridOrigin;
    float tileSize = 80.f;

    sf::FloatRect farmBounds;

    // Farmers
    Farmer playerFarmer;
    Farmer aiFarmer;

    //Crop textures
    sf::Texture carrotTexture;
    sf::Texture tomatoTexture;
    sf::Texture lettuceTexture;
    sf::Texture cornTexture;
    sf::Texture potatoTexture;

    sf::Texture& seedTexture(CropType ct);

    // Texts for HUD
    sf::Text playerScoreText;
    sf::Text aiScoreText;
    sf::Text timerText;

    // Match timer
    float gameTimer = 60.f;

    // Action for main.cpp
    GameAction action { GameAction::None };

    // --- Requests / Orders ---
    std::vector<Request> requests;
    int currentRequestIndex = 0;
    int levelID = 1;

    sf::Text currentRequestText;

    // Request helpers
    std::vector<CropType> allowedCropsForLevel(int level) const;
    int maxQtyForLevel(int level) const;
    int numRequestsForLevel(int level) const;
    Request makeRandomRequest(int level);
    std::string requestToString(const Request& r) const;

        // --- AI-related members ---
    AIState aiState = AIState::SelectGoal;
    CropType aiTargetCrop = CropType::None;   // what the AI is currently trying to produce
    std::vector<int> aiPath;                  // sequence of tile indices (A* result)
    int aiPathIndex = 0;                      // next waypoint index in aiPath
    float aiMaxSpeed = 150.f;                 // AI movement speed
    float aiArriveThreshold = 10.f;           // pixels to consider 'arrived' at a waypoint

    // AI helpers
    int tileIndexFromPos(const sf::Vector2f& pos) const;
    sf::Vector2f tileCenter(int index) const;
    bool isTileWalkable(int index) const;

    // A* pathfinding
    std::vector<int> findPathAStar(int startIdx, int goalIdx);


};

