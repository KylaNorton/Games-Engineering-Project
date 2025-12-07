#include "game.hpp"
#include "player.hpp"
#include "playerSave.hpp"
#include "spriteLib.hpp"
#include <iostream>
#include <fstream>
#include <random>

#include <queue>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <algorithm>

// Duration for the temporary sold visual (seconds)
static constexpr float sold_visual_temp = 0.8f;
// Duration for the temporary seed-taken visual (seconds)
static constexpr float seed_take_visual_temp = 0.6f;

// --- Convert position to tile index (or -1 if outside) ---
int Game::tileIndexFromPos(const sf::Vector2f& pos) const {
    // pos relative to gridOrigin
    float relX = pos.x - gridOrigin.x;
    float relY = pos.y - gridOrigin.y;
    if (relX < 0 || relY < 0) return -1;

    // compute tile size from your existing layout:
    float fullWidth = static_cast<float>(window.getSize().x);
    float playTop = board.box.getPosition().y + board.box.getSize().y;
    float playLeft = 0.f;
    float playRight = fullWidth;
    float playBottom = static_cast<float>(window.getSize().y);

    float playWidth = playRight - playLeft;
    float playHeight = playBottom - playTop;

    float tileW = playWidth / gridCols;
    float tileH = playHeight / gridRows;

    int col = static_cast<int>(relX / tileW);
    int row = static_cast<int>(relY / tileH);

    if (col < 0 || col >= gridCols || row < 0 || row >= gridRows) return -1;
    return row * gridCols + col;
}

// --- Tile center coordinates ---
sf::Vector2f Game::tileCenter(int index) const {
    if (index < 0 || index >= static_cast<int>(farm.size())) return {0.f, 0.f};
    const FarmTile& t = farm[index];
    auto pos = t.rect.getPosition();
    auto size = t.rect.getSize();
    return { pos.x + size.x * 0.5f, pos.y + size.y * 0.5f };
}

bool Game::isTileWalkable(int index) const {
    if (index < 0 || index >= static_cast<int>(farm.size())) return false;
    return farm[index].walkable;
}


// Simple A* on the grid using Manhattan distance
std::vector<int> Game::findPathAStar(int startIdx, int goalIdx) {
    std::vector<int> emptyPath;
    if (startIdx < 0 || goalIdx < 0) return emptyPath;
    if (startIdx == goalIdx) return {startIdx};

    auto heuristic = [&](int a, int b)->int {
        int ax = a % gridCols, ay = a / gridCols;
        int bx = b % gridCols, by = b / gridCols;
        return std::abs(ax - bx) + std::abs(ay - by);
    };

    struct Node {
        int idx;
        int f;
        int g;
        int cameFrom;
    };

    const int N = gridRows * gridCols;
    const int INF = std::numeric_limits<int>::max();

    std::vector<int> gScore(N, INF);
    std::vector<int> fScore(N, INF);
    std::vector<int> cameFrom(N, -1);
    std::vector<char> closed(N, 0);

    auto pushToOpen = [&](std::priority_queue<std::pair<int,int>,
                        std::vector<std::pair<int,int>>, std::greater<>> &pq, int idx, int f){
        pq.push({f, idx});
    };

    std::priority_queue<std::pair<int,int>,
        std::vector<std::pair<int,int>>, std::greater<>> openSet;

    gScore[startIdx] = 0;
    fScore[startIdx] = heuristic(startIdx, goalIdx);
    pushToOpen(openSet, startIdx, fScore[startIdx]);

    while (!openSet.empty()) {
        int current = openSet.top().second;
        openSet.pop();

        if (closed[current]) continue;
        if (current == goalIdx) {
            // reconstruct path
            std::vector<int> path;
            int cur = current;
            while (cur != -1) {
                path.push_back(cur);
                cur = cameFrom[cur];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }
        closed[current] = 1;

        int cx = current % gridCols;
        int cy = current / gridCols;

        // neighbors 4-dir
        const int dx[4] = {1,-1,0,0};
        const int dy[4] = {0,0,1,-1};

        for (int k = 0; k < 4; ++k) {
            int nx = cx + dx[k];
            int ny = cy + dy[k];
            if (nx < 0 || nx >= gridCols || ny < 0 || ny >= gridRows) continue;
            int nidx = ny * gridCols + nx;
            if (!isTileWalkable(nidx)) continue;

            // Restrict A* to the right side of the centre path (AI should not path into the player's side)
            // Compute the right edge X of the centrePath and skip any neighbour whose tile center
            // is left of (or on) that edge.
            {
                sf::FloatRect wall = centerPath.getGlobalBounds();
                float wallRightX = wall.left + wall.width;
                sf::Vector2f tc = tileCenter(nidx);
                if (tc.x <= wallRightX) continue; // skip tiles on or left of the divider
            }

            int tentativeG = gScore[current] + 1; // cost = 1 per step
            if (tentativeG < gScore[nidx]) {
                cameFrom[nidx] = current;
                gScore[nidx] = tentativeG;
                fScore[nidx] = tentativeG + heuristic(nidx, goalIdx);
                pushToOpen(openSet, nidx, fScore[nidx]);
            }
        }
    }

    // no path found
    return emptyPath;
}


// Helper to convert from char in level file to GroundType and CropType
static void charToGroundType(char c, GroundType& gt, CropType& ct) {
    gt = GroundType::Empty;
    ct = CropType::None;

    switch (c) {
    case 'T': 
        gt = GroundType::Soil;
        break;
        // Seed boxes
            case '1':   // tomato seeds
                gt = GroundType::Seeds;
                ct = CropType::Tomato;
                break;
            case '2':   // corn seeds
                gt = GroundType::Seeds;
                ct = CropType::Corn;
                break;
            case '3':   // potato seeds
                gt = GroundType::Seeds;
                ct = CropType::Potato;
                break;
            case '4':   // carrot seeds
                gt = GroundType::Seeds;
                ct = CropType::Carrot;
                break;
            case '5':   // lettuce seeds
                gt = GroundType::Seeds;
                ct = CropType::Lettuce;
                break;

    case 'G': 
         gt = GroundType::Seeds;
        break;
    case 'E': 
        gt = GroundType::Water;
        break;
    case 'S': 
        gt = GroundType::Sun;
        break;
    case 'M': 
        gt = GroundType::Market;
        break;
    case 'P': 
        gt = GroundType::Trash;
        break;
    case 'F': 
        gt = GroundType::Empty;
        break;
    default :  
        gt = GroundType::Empty;
        break;
    }
}

// Helper to get crop name as string
static const char* cropName(CropType c) {
    switch (c) {
    case CropType::Carrot:  return "Carrot";
    case CropType::Tomato:  return "Tomato";
    case CropType::Lettuce: return "Lettuce";
    case CropType::Corn:    return "Corn";
    case CropType::Potato:  return "Potato";
    case CropType::None:
    default:
        return "None";
    }
}

// Convert a Request to a human-readable string
std::string Game::requestToString(const Request& r) const {
    if (r.items.empty())
        return "No request";

    std::string s;
    for (size_t i = 0; i < r.items.size(); ++i) {
        CropType ct = r.items[i].first;
        int qty     = r.items[i].second;
        if (qty <= 0) continue;  // already fulfilled

        if (!s.empty())
            s += "  |  ";

        s += std::to_string(qty);
        s += "x ";
        s += cropName(ct);
    }
    if (s.empty())
        s = "Completed";
    return s;
}


/// REQUEST GENERATION ///

// Global RNG for the game file
static std::mt19937 rng{ std::random_device{}() };

// Which crops are allowed per level
std::vector<CropType> Game::allowedCropsForLevel(int level) const {
    switch (level) {
        case 1:  // level 1 : T, P, Ca
            return { CropType::Tomato, CropType::Potato, CropType::Carrot };
        case 2:  // level 2 : T, P, Ca, L
            return { CropType::Tomato, CropType::Potato, CropType::Carrot, CropType::Lettuce };
        case 3:  // level 3 : T, P, Ca, L, Co
        case 4:  // level 4 : same variety, harder numbers
        default:
            return { CropType::Tomato, CropType::Potato, CropType::Carrot,
                     CropType::Lettuce, CropType::Corn };
    }
}

// Max quantity per vegetable type, per level
int Game::maxQtyForLevel(int level) const {
    switch (level) {
        case 1: return 3;  // from your examples
        case 2: return 3;
        case 3: return 4;
        case 4: return 5;
        default: return 5;
    }
}

// How many requests per level
int Game::numRequestsForLevel(int level) const {
    switch (level) {
        case 1: return 5;
        case 2: return 7;
        case 3: return 9;
        case 4: return 12;
        default: return 5;
    }
}

// Generate ONE random request respecting all your rules
Request Game::makeRandomRequest(int level) {
    Request r;

    auto allowed = allowedCropsForLevel(level);
    int maxQty   = maxQtyForLevel(level);

    if (allowed.empty()) return r;

    // 1) decide how many different vegetables (1..3, but not more than allowed.size())
    int maxTypes = static_cast<int>(std::min<size_t>(3, allowed.size()));
    std::uniform_int_distribution<int> distTypes(1, maxTypes);
    int k = distTypes(rng);   // number of different crops in this request

    // 2) choose k distinct crops: shuffle then take first k
    std::shuffle(allowed.begin(), allowed.end(), rng);

    // 3) for each chosen crop, choose a quantity 1..maxQty
    std::uniform_int_distribution<int> distQty(1, maxQty);

    for (int i = 0; i < k; ++i) {
        CropType ct = allowed[i];
        int qty = distQty(rng);
        r.items.push_back({ct, qty});
        r.initialQty.push_back(qty);
        r.playerContrib.push_back(0);
        r.aiContrib.push_back(0);
    }

    return r;
}

static void centerSpriteOrigin(sf::Sprite& s) {
    sf::FloatRect bounds = s.getLocalBounds();
    s.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
}


// ------------------------
// Game constructor 
// ------------------------
Game::Game(sf::RenderWindow& win, int levelID) : window(win), levelID(levelID) {

    // --------------------------------------------------
    // 1) FONT
    // --------------------------------------------------
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) {
        std::cerr << "[WARN] Font not found at res/fonts/Inter-Regular.ttf. Buttons will show without text.\n";
    }

    // --------------------------------------------------
    // 2) ICON TEXTURES (back + pause)
    // --------------------------------------------------
    static sf::Texture backTexture;
    static sf::Texture pauseTexure;

    backTexture.loadFromFile("res/icons/arrow.png");
    pauseTexure.loadFromFile("res/icons/pause-play.png");

    // BACK BUTTON (top-left)
    backButton.box.setSize({50.f, 50.f});
    backButton.box.setFillColor(sf::Color(0, 0, 0, 0)); // transparent box for click area
    backButton.box.setPosition({20.f, 5.f});

    backButton.sprite.setTexture(backTexture);
    backButton.sprite.setScale(0.06f, 0.06f);
    backButton.sprite.setPosition({25.f, 10.f});

    // PAUSE BUTTON (top-right)
    pauseButton.box.setSize({50.f, 50.f});
    pauseButton.box.setFillColor(sf::Color(0, 0, 0, 0));
    pauseButton.box.setPosition({window.getSize().x - 70.f, 5.f});

    pauseButton.sprite.setTexture(pauseTexure);
    pauseButton.sprite.setScale(0.08f, 0.08f);
    pauseButton.sprite.setPosition({window.getSize().x - 65.f, 10.f});

    // --------------------------------------------------
    // 3) BASIC WINDOW DIMENSIONS
    // --------------------------------------------------
    const float winW = static_cast<float>(window.getSize().x);
    const float winH = static_cast<float>(window.getSize().y);

    const float topBarHeight = 80.f; // coloured strip at the top
    const float bottomBarHeight = 0.f; // set to 0 for now, no MARKER bar
    const float sideBarWidth = 0.f; // no gameplay side bars now

    // --------------------------------------------------
    // 4) TOP BAR + INFO BOARD (You / Timer / AI)
    // --------------------------------------------------

    // top background bar
    topBar.setSize({winW, topBarHeight}); 
    topBar.setPosition(0.f, 0.f);
    topBar.setFillColor(sf::Color(40, 30, 70)); // dark purple

    // info board rectangle (dark grey, centered)
    board.box.setSize({800.f, 50.f});
    board.box.setFillColor(sf::Color(50, 50, 50)); // dark grey
    board.box.setPosition({(winW - 800.f) / 2.f, 0.f});

    // bottom bar is not used now, but we keep the object in case you need it later
    bottomBar.setSize({winW, bottomBarHeight});
    bottomBar.setPosition(0.f, winH - bottomBarHeight);
    bottomBar.setFillColor(sf::Color(40, 30, 70)); // dark purple

    // left/right bars are also unused for gameplay now
    leftBar.setSize({sideBarWidth, winH - topBarHeight - bottomBarHeight});
    leftBar.setPosition(0.f, topBarHeight);
    leftBar.setFillColor(sf::Color(60, 40, 90)); // darker purple

    rightBar.setSize({sideBarWidth, winH - topBarHeight - bottomBarHeight});
    rightBar.setPosition(winW - sideBarWidth, topBarHeight);
    rightBar.setFillColor(sf::Color(60, 40, 90)); // darker purple

    // --------------------------------------------------
    // 5) TEXTS INSIDE THE INFO BOARD
    // --------------------------------------------------
    if (hasFont) {
        // Player score text (left)
        playerScoreText.setFont(font);
        playerScoreText.setCharacterSize(20);
        playerScoreText.setFillColor(sf::Color::White);
        playerScoreText.setString("You: 0  Req: 0");
        playerScoreText.setPosition(board.box.getPosition().x + 20.f, board.box.getPosition().y + 25.f);

        // AI score text (right)
        aiScoreText.setFont(font);
        aiScoreText.setCharacterSize(20);
        aiScoreText.setFillColor(sf::Color::White);
        aiScoreText.setString("AI: 0  Req: 0");
        aiScoreText.setPosition(board.box.getPosition().x + board.box.getSize().x - 120.f, board.box.getPosition().y + 25.f);

        // Timer text (center)
        timerText.setFont(font);
        timerText.setCharacterSize(20);
        timerText.setFillColor(sf::Color::White);
        timerText.setString("60s");
        float centerX = board.box.getPosition().x + board.box.getSize().x / 2.f;
        timerText.setPosition(centerX - 20.f, board.box.getPosition().y + 25.f);
    
        // Current request display 
        currentRequestText.setFont(font);
        currentRequestText.setCharacterSize(20);
        currentRequestText.setFillColor(sf::Color::White);
        currentRequestText.setString("Request: -");
        currentRequestText.setPosition(
            board.box.getPosition().x + 170.f,
            board.box.getPosition().y + 0.5f // adjust 
        );
    }

    // --------------------------------------------------
    // 6) PLAYABLE AREA = EVERYTHING UNDER THE INFO BOARD
    //    - tiles will cover this whole area
    //    - both players can move anywhere inside this rect
    // --------------------------------------------------

    // bottom of the info board
    float playTop = board.box.getPosition().y + board.box.getSize().y;
    float playLeft = 0.f;
    float playRight = winW;
    float playBottom = winH;

    float playWidth  = playRight - playLeft;
    float playHeight = playBottom - playTop;

    // --------------------------------------------------
    // 7) GRID SIZE
    //    - choose how many tiles horizontally / vertically
    //    - tiles will be rectangles that exactly cover the area
    // --------------------------------------------------

    // Each tile will be a rectangle; we compute its size from the playable area
    float tileWidth  = playWidth  / gridCols;
    float tileHeight = playHeight / gridRows;

    // we store a "generic" tileSize for later if you need it (useful e.g. for movement)
    tileSize = std::min(tileWidth, tileHeight);

    // top-left of the grid
    gridOrigin = { playLeft, playTop };

    // --------------------------------------------------
    // 8) CREATE BROWN TILE GRID COVERING ALL PLAYABLE AREA
    // --------------------------------------------------
    farm.clear();
    farm.resize(gridRows * gridCols);

    for (int row = 0; row < gridRows; ++row) {
        for (int col = 0; col < gridCols; ++col) {

            int idx = row * gridCols + col;
            FarmTile& t = farm[idx];

            // each tile is slightly smaller than its "slot"
            // so you see a small grid line between them
            t.rect.setSize({tileWidth - 2.f, tileHeight - 2.f});
            t.rect.setPosition({
                gridOrigin.x + col * tileWidth + 1.f,
                gridOrigin.y + row * tileHeight + 1.f
            });

            t.type = GroundType::Empty; // will become Soil later
            t.walkable = true; // player can walk on all ground
            t.state = TileState::Empty; // nothing planted yet
            t.growthTimer = 0.f;
            t.rect.setFillColor(sf::Color(90, 60, 30)); // brown soil colour
        }
    }

    // --------------------------------------------------
    // 9) LOAD LEVEL LAYOUT FROM TEXT FILE
    // --------------------------------------------------
    std::string levelPath = "res/levels/level" + std::to_string(levelID) + ".txt";

    centerPath.setSize({4.f, playHeight});
    centerPath.setPosition(winW / 2.f - 2.f, playTop);
    centerPath.setFillColor(sf::Color(255, 0, 0)); // red line
    
    std::ifstream in(levelPath);
    if (!in) {
        std::cerr << "[ERROR] Cannot open level file: " << levelPath << "\n";
        // fall back: keep default GroundType::Empty for all tiles
    } else {
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') // Windows line endings
                line.pop_back();
            if ((int)line.size() >= gridCols)
                lines.push_back(line.substr(0, gridCols));
        }

        if ((int)lines.size() < gridRows) {
            std::cerr << "[WARN] Level file has fewer than " << gridRows << " rows\n";
        } else {
            for (int row = 0; row < gridRows; ++row) {
                for (int col = 0; col < gridCols; ++col) {
                    int idx = row * gridCols + col;
                    FarmTile& t = farm[idx];

                    char c = lines[row][col];
                    GroundType gt;
                    CropType ct;
                    charToGroundType(c, gt, ct);

                    t.type = gt;
                    t.crop = ct;

                    // choose color based on type
                    switch (gt) {
                    case GroundType::Soil:
                        t.rect.setFillColor(sf::Color(102, 51, 0));       // dark soil
                        t.walkable = true;
                        break;
                    case GroundType::Seeds:
                        t.rect.setFillColor(sf::Color(255, 128, 0));      // orange
                        t.walkable = true;
                        break;
                    case GroundType::Water:
                        t.rect.setFillColor(sf::Color(153, 204, 255));    // light blue
                        t.walkable = true;
                        break;
                    case GroundType::Sun:
                        t.rect.setFillColor(sf::Color(255, 255, 0));      // yellow
                        t.walkable = true;
                        break;
                    case GroundType::Market:
                        t.rect.setFillColor(sf::Color(51, 102, 0));       // dark green
                        t.walkable = true;
                        break;
                    case GroundType::Trash:
                        t.rect.setFillColor(sf::Color(128, 128, 128));       // grey
                        t.walkable = true;
                        break;
                    case GroundType::Empty:
                    default:
                        t.rect.setFillColor(sf::Color(40, 40, 60));       // floor
                        t.walkable = true; // or false if you want
                        break;
                    }
                }
            }
        }
    }

    tomatoTexture.loadFromFile("res/crops/tomato.png");
    cornTexture.loadFromFile("res/crops/corn.png"); 
    carrotTexture.loadFromFile("res/crops/carrot.png");
    lettuceTexture.loadFromFile("res/crops/lettuce.png");
    potatoTexture.loadFromFile("res/crops/potato.png");

    // --------------------------------------------------
    // 10) FARMER POSITIONS
    //      - both start roughly in the middle of their half
    // --------------------------------------------------
    sf::Vector2f playerStart(
        winW * 0.25f,                         // quarter of the screen width
        playTop + playHeight * 0.5f           // vertical center of the playable area
    );

    // Player farmer
    playerFarmer.body.setRadius(18.f);
    playerFarmer.body.setOrigin(18.f, 18.f);
    playerFarmer.body.setFillColor(gAppearance.playerColor);
    playerFarmer.body.setPosition(playerStart);
    playerFarmer.score = 0;

    playerFarmer.sprite.setTexture( PlayerSpriteLibrary::instance().getTexture(gAppearance.playerTextureIndex));
    centerSpriteOrigin(playerFarmer.sprite);
    playerFarmer.sprite.setPosition(playerFarmer.body.getPosition());

    // Set initial frame for player sprite
    auto texSize = playerFarmer.sprite.getTexture()->getSize();
    int frameCols = 4;
    int frameRows = 4;
    int frameW = texSize.x / frameCols;
    int frameH = texSize.y / frameRows;

    // pick frame (col=0, row=0) = facing down, first frame
    playerFarmer.sprite.setTextureRect(sf::IntRect(0, 0, frameW, frameH));

    // NOW center origin based on frame, not full sheet
    centerSpriteOrigin(playerFarmer.sprite);
    playerFarmer.sprite.setScale(0.5f, 0.5f); 
    playerFarmer.sprite.setPosition(playerFarmer.body.getPosition());


    // AI farmer
    sf::Vector2f aiStart(winW * 0.75f, playTop + playHeight * 0.5f);  // three quarters of the screen width
    aiFarmer.body.setRadius(18.f);
    aiFarmer.body.setOrigin(18.f, 18.f);
    aiFarmer.body.setFillColor(gAppearance.aiColor);
    aiFarmer.body.setPosition(aiStart);
    aiFarmer.score = 0;

    aiFarmer.sprite.setTexture(PlayerSpriteLibrary::instance().getTexture(gAppearance.aiTextureIndex));
    centerSpriteOrigin(aiFarmer.sprite);
    aiFarmer.sprite.setScale(0.8f, 0.8f); 
    aiFarmer.sprite.setPosition(aiFarmer.body.getPosition());

    auto texSizeAI = aiFarmer.sprite.getTexture()->getSize();
    int frameW_AI = texSizeAI.x / 4;
    int frameH_AI = texSizeAI.y / 4;
    aiFarmer.sprite.setTextureRect(sf::IntRect(0, 0, frameW_AI, frameH_AI));
    centerSpriteOrigin(aiFarmer.sprite);
    aiFarmer.sprite.setPosition(aiFarmer.body.getPosition());


    // --------------------------------------------------
    // 11) GENERATE RANDOM REQUESTS FOR THIS LEVEL
    // --------------------------------------------------
    int nReq = numRequestsForLevel(levelID);
    requests.clear();
    requests.reserve(nReq);

    for (int i = 0; i < nReq; ++i) {
        Request r = makeRandomRequest(levelID);
        requests.push_back(r);
    }

    currentRequestIndex = 0;

    // Debug: print them in console so you can see them
    std::cout << "=== Requests for level " << levelID << " ===\n";
    for (int i = 0; i < static_cast<int>(requests.size()); ++i) {
        std::cout << "Request " << i + 1 << ": ";

        for (const auto& item : requests[i].items) {
            CropType ct = item.first;
            int qty     = item.second;
            std::cout << qty << "x " << cropName(ct) << "  ";
        }

        std::cout << "\n";
    }

    if (!requests.empty() && hasFont) {
        updateCurrentRequestText();
    }
}

void Game::showTextPopup(const sf::Font& font, const std::string& msg, sf::Vector2f position) {
    popup.text.setFont(font);
    popup.text.setString(msg);
    popup.text.setCharacterSize(32);
    popup.text.setFillColor(sf::Color::White);
    popup.text.setPosition(position);

    popup.useText = true;
    popup.timer = 0.f;
    popup.duration = 1.f;
    popup.active = true;
}

sf::Texture& Game::seedTexture(CropType ct) {
    switch (ct) {
        case CropType::Tomato: return tomatoTexture;
        case CropType::Corn:   return cornTexture;
        case CropType::Carrot: return carrotTexture;
        case CropType::Lettuce:return lettuceTexture;
        case CropType::Potato: return potatoTexture;
        default: 
            return tomatoTexture; // default texture
    }
}

void Game::updateCurrentRequestText()
{
    if (!hasFont) return;

    if (currentRequestIndex >= 0 &&
        currentRequestIndex < static_cast<int>(requests.size()))
    {
        const Request& r = requests[currentRequestIndex];

        std::string label = "Request " +
            std::to_string(currentRequestIndex + 1) + "/" +
            std::to_string(static_cast<int>(requests.size())) + ": ";

        currentRequestText.setString(label + requestToString(r));
    }
    else {
        currentRequestText.setString("All requests completed!");
    }
}

void Game::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f m{(float)e.mouseButton.x, (float)e.mouseButton.y};
        if (backButton.box.getGlobalBounds().contains(m)) {
            action = GameAction::Back;
        }
        if (pauseButton.box.getGlobalBounds().contains(m)) {
            PauseGame = true;
        }
    }

    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Q) {
        window.close(); //quit app
    }

    if (!PauseGame && !Tutorial && !EndGame && e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Space) {
        PauseGame = true; 
        return;
    }
    
    if (Tutorial) {
        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::C) {
                PauseGame = true;
                Tutorial = false; // close popup, back to pause
            }
        }
        return; // while popup is open, ignore other events
    }

    if (PauseGame) {
        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::Space) {
                PauseGame = false; // close popup, back to game
            }
            if (e.key.code == sf::Keyboard::T) {
                Tutorial = true;   // open tutorial popup
            }
        }
        return; // while popup is open, ignore other events
    }

    if (EndGame) {
        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::P) {
              action = GameAction::Play;   // play again
            }
            else if (e.key.code == sf::Keyboard::M) {
                action = GameAction::Back;   // back to menu
            }
        }
        return; // while popup is open, ignore other events
    }

    if (!PauseGame && e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::T) { //T = take

        // Player interacts with the tile under them
        sf::Vector2f p = playerFarmer.body.getPosition();

        for (auto& tile : farm) { 
            if (!tile.rect.getGlobalBounds().contains(p)) continue; 
                if (tile.type == GroundType::Seeds && !playerFarmer.hasSeed) {
                    // take seed
                    playerFarmer.carriedSeed = tile.crop;
                    playerFarmer.hasSeed = true; 
                    std::cout << "Player: " << cropName(tile.crop) << " seed taken\n";
                    // trigger a small visual on the seed box to indicate it was taken
                    tile.seedTakenTimer = seed_take_visual_temp;
                    tile.seedTakenCrop = tile.crop;
                    break;
                }
                if (tile.type == GroundType::Water && !playerFarmer.hasWater) {
                    // take water
                    playerFarmer.hasWater = true;
                    std::cout << "Player: Water taken\n";   
                    break;             
                }
                if (tile.type == GroundType::Sun && !playerFarmer.hasSun) {
                    // take sun
                    playerFarmer.hasSun = true;
                    std::cout << "Player: Sun taken\n";
                    break;
                }
                if (tile.state == TileState::Grown && tile.type == GroundType::Soil) {
                    // harvest
                    tile.state = TileState::Empty;
                    playerFarmer.carriedSeed = tile.crop;
                    tile.growthTimer = 0.f;
                    tile.rect.setFillColor(sf::Color(102, 51, 0)); // back to soil
                    playerFarmer.hasProduct = true;
                    std::cout << "Player: " << cropName(tile.crop) << " harvested\n";
                    break;
                } 
                break;
        }
    }

    if (!PauseGame && e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::D) { //D = drop

        // Player interacts with the tile under them
        sf::Vector2f p = playerFarmer.body.getPosition();

        for (auto& tile : farm) { 
            if (!tile.rect.getGlobalBounds().contains(p)) continue; 

                if (tile.state == TileState::Empty && tile.type == GroundType::Soil && playerFarmer.hasSeed) {
                    // plant seed
                    tile.state = TileState::Seeded;
                    tile.growthTimer = 0.f;
                    tile.crop = playerFarmer.carriedSeed;
                    playerFarmer.hasSeed = false;   
                    playerFarmer.carriedSeed = CropType::None;
                    std::cout << "Player: " << cropName(tile.crop) << " seed planted\n";
                    tile.rect.setFillColor(sf::Color(51, 25, 0)); // darker soil
                    break;
                }
                if (tile.state == TileState::Seeded && tile.type == GroundType::Soil && playerFarmer.hasWater) {
                    // drop water
                    tile.state = TileState::Watered;
                    tile.growthTimer = 0.f;
                    playerFarmer.hasWater = false;
                    std::cout << "Player: " << cropName(tile.crop) << " plant watered\n";
                    break;
                }
                if (tile.state == TileState::Seeded && tile.type == GroundType::Soil && playerFarmer.hasSun) {
                    // drop sun
                    tile.state = TileState::Suned;
                    tile.growthTimer = 0.f;
                    std::cout << "Player: Sun dropped\n";
                    playerFarmer.hasSun = false;
                    break;
                } 
                if (tile.type == GroundType::Market && playerFarmer.hasProduct) {
                    // sell product
                    CropType product = playerFarmer.carriedSeed;

                    if(currentRequestIndex >= 0 && currentRequestIndex < static_cast<int>(requests.size())) {
                        Request& r = requests[currentRequestIndex];
                        bool completed = false;
                        // find the matching item index and attribute this sale to the player
                        for (size_t i = 0; i < r.items.size(); ++i) {
                            auto &item = r.items[i];
                            if (item.first == product && item.second > 0) {
                                item.second -= 1; // decrease quantity needed
                                r.playerContrib[i] += 1; // attribute to player
                                completed = true;
                                playerFarmer.score += 5; // give 5 points per required veg delivered
                                playerCorrectDeliveries += 1; // count correct deliveries
                                std::cout << "Player score +5\n";
                                std::cout << "Player score: " << playerFarmer.score << "\n";
                                break;
                            }
                        }

                        if (completed) {
                            std::cout << "Player delivered " << cropName(product) << " for the request \n";
                                    // trigger a short "sold" visual on this market tile
                                    tile.soldTimer = sold_visual_temp;
                                    tile.soldCrop = product;
                                    updateCurrentRequestText();


                            // Check if the entire request is fulfilled
                            bool allDone = true;
                            for (const auto& item : r.items) {
                                if (item.second > 0) { allDone = false; break; }
                            }

                            if (allDone) {
                                // Ensure completion is only processed once
                                if (!r.completed) {
                                    // determine total initial qty
                                    int totalQty = 0;
                                    for (size_t i = 0; i < r.items.size(); ++i) totalQty += r.initialQty[i];
                                    // compute how many items each side delivered for this request
                                    int playerDelivered = 0;
                                    int aiDelivered = 0;
                                    for (size_t j = 0; j < r.playerContrib.size(); ++j) playerDelivered += r.playerContrib[j];
                                    for (size_t j = 0; j < r.aiContrib.size(); ++j) aiDelivered += r.aiContrib[j];
                                    int N = totalQty;

                                    if (playerDelivered > aiDelivered) {
                                        playerRequestsCompleted += 1; // dominated count
                                        playerFarmer.score += 3 * N;  // full bonus to player
                                        std::cout << "Player completion bonus +" << 3 * N << "\n";
                                    } else if (aiDelivered > playerDelivered) {
                                        aiRequestsCompleted += 1;
                                        aiFarmer.score += 3 * N;
                                        std::cout << "AI completion bonus +" << 3 * N << "\n";
                                    } else {
                                        // tie: split the 3*N bonus evenly (round to nearest)
                                        int tieBonus = static_cast<int>(std::round((3.0 * N) / 2.0));
                                        playerRequestsCompleted += 1;
                                        aiRequestsCompleted += 1;
                                        playerFarmer.score += tieBonus;
                                        aiFarmer.score += tieBonus;
                                        std::cout << "Tie completion bonus +" << tieBonus << " each\n";
                                    }

                                    r.completed = true; // mark so we don't double-award
                                    std::cout << "[DBG] Request " << (currentRequestIndex + 1) << " N=" << N << " playerDelivered=" << playerDelivered << " aiDelivered=" << aiDelivered << "\n";
                                }

                                showTextPopup(font, "Request " + std::to_string(currentRequestIndex + 1) + " completed!\n", {300.f, 50.f});
                                std::cout << "Request " << (currentRequestIndex + 1) << " completed!\n";
                                currentRequestIndex++;
                                if (currentRequestIndex < static_cast<int>(requests.size()) && hasFont) {
                                    currentRequestText.setString("Request: " + requestToString(requests[currentRequestIndex]));
                                } else {
                                    currentRequestText.setString("All requests completed!");
                                }
                                updateCurrentRequestText();

                                // Other player completed the request — make AI abandon its current task
                                // so it immediately re-evaluates the new request (or picks a new target).
                                aiPath.clear();
                                aiPathIndex = 0;
                                aiTargetCrop = CropType::None;
                                aiState = AIState::SelectGoal;
                            }
                        } else {
                            std::cout << "Player: " << cropName(product) << " is not needed for the current request\n";
                        }
                    }

                    playerFarmer.hasProduct = false;
                    playerFarmer.carriedSeed = CropType::None;
                    break;
                }
                if (tile.type == GroundType::Trash) {
                    if (playerFarmer.hasSeed) {
                        playerFarmer.hasSeed = false;
                        std::cout << "Player: " << cropName(playerFarmer.carriedSeed) << " seed discarded\n";
                        playerFarmer.carriedSeed = CropType::None;
                        break;
                    }
                    if (playerFarmer.hasWater) {
                        playerFarmer.hasWater = false;
                        std::cout << "Player: Water discarded\n";
                        break;
                    }
                    if (playerFarmer.hasSun) {
                        playerFarmer.hasSun = false;
                        std::cout << "Player: Sun discarded\n";
                        break;
                    }
                    if (playerFarmer.hasProduct) {
                        playerFarmer.hasProduct = false;
                        std::cout << "Player: " << cropName(playerFarmer.carriedSeed) << "  discarded\n";
                        playerFarmer.carriedSeed = CropType::None;
                        break;
                    }
                }
        }
    }  
}

void Game::update(float dt) {
    if (PauseGame || EndGame) return; // don't update when game is paused

    // --------------------------------------------------
    // 1) PLAYER MOVEMENT INPUT
    // --------------------------------------------------
    sf::Vector2f v(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  v.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) v.x += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    v.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  v.y += 1.f;

    // normalise diagonal movement so speed is the same in all directions
    if (v.x != 0.f || v.y != 0.f) {
        float len = std::sqrt(v.x * v.x + v.y * v.y);
        v /= len;
    }

    // --------------------------------------------------
    // 2) PLAYABLE AREA BOUNDS
    //    - left = 0
    //    - right = window width
    //    - top = bottom of info board
    //    - bottom = window height
    // --------------------------------------------------
    float playTop = board.box.getPosition().y + board.box.getSize().y;
    float playLeft = 0.f;
    float playRight = static_cast<float>(window.getSize().x);
    float playBottom = static_cast<float>(window.getSize().y);

    sf::FloatRect playArea(playLeft, playTop, playRight - playLeft, playBottom - playTop);

    sf::FloatRect wall = centerPath.getGlobalBounds(); // wall between the two players
    
    // --------------------------------------------------
    // 3) MOVE PLAYER IF THE WHOLE CIRCLE STAYS INSIDE playArea
    // --------------------------------------------------
    if (v.x != 0.f || v.y != 0.f) {
        sf::Vector2f next = playerFarmer.body.getPosition() + v * speed * dt;
        playerFarmer.sprite.setPosition(playerFarmer.body.getPosition()); // keep sprite in sync
        float r = playerFarmer.body.getRadius();
    

        // bounding box of the circle at the next position
        sf::FloatRect circle(next.x - r, next.y - r, 2.f * r, 2.f * r);

        // check all 4 corners: if they are inside playArea,
        // then the whole circle is inside the playable zone
        bool inside =
            playArea.contains(circle.left, circle.top) &&
            playArea.contains(circle.left + circle.width, circle.top) &&
            playArea.contains(circle.left, circle.top + circle.height) &&
            playArea.contains(circle.left + circle.width, circle.top + circle.height);
        
        bool leftOfWall = (next.x + r) <= wall.left; // must stay on the left side of the wall

        if (inside && leftOfWall) {
            playerFarmer.body.setPosition(next); 
            playerFarmer.sprite.setPosition(playerFarmer.body.getPosition()); // keep sprite in sync
        }
    }

    // --------------------------------------------------
    // 4) AI MOVEMENT
    // --------------------------------------------------
    static float aiDir = 1.f; // 1 = move right, -1 = move left

    sf::Vector2f aiVel(aiDir, 0.f); // simple left-right movement
    sf::Vector2f aiNext = aiFarmer.body.getPosition() + aiVel * (speed * 0.4f * dt);
    aiFarmer.sprite.setPosition(aiFarmer.body.getPosition()); // keep sprite in sync
    float ar = aiFarmer.body.getRadius();

    sf::FloatRect aiCircle(aiNext.x - ar, aiNext.y - ar, 2.f * ar, 2.f * ar);

    bool aiInside =
        playArea.contains(aiCircle.left, aiCircle.top) &&
        playArea.contains(aiCircle.left + aiCircle.width, aiCircle.top) &&
        playArea.contains(aiCircle.left, aiCircle.top + aiCircle.height) &&
        playArea.contains(aiCircle.left + aiCircle.width, aiCircle.top + aiCircle.height);

    bool rightOfWall = (aiNext.x - ar) >= (wall.left + wall.width); // must stay on the right side of the wall

    if (aiInside && rightOfWall) {
        aiFarmer.body.setPosition(aiNext);
        aiFarmer.sprite.setPosition(aiFarmer.body.getPosition()); // keep sprite in sync
    } else {
        aiDir *= -1.f; // if next position would leave the area, bounce
    }

    // --------------------------------------------------
    // 5) REST OF YOUR UPDATE (crops, timer, scores...)
    // --------------------------------------------------

    // Grow crops
    sf::Vector2f p = playerFarmer.body.getPosition();
    for (auto& tile : farm) {
        if (tile.type == GroundType::Soil && tile.state != TileState::Empty && tile.state != TileState::Grown) {
            tile.growthTimer += dt;
            if (tile.growthTimer > 3.f) { // 3 seconds to grow
                tile.state = TileState::Grown;
                tile.rect.setFillColor(sf::Color(102, 51, 0)); // back to brown soil colour 
            }
        }
    }

    // Update sold visual timers
    for (auto& tile : farm) {
        if (tile.soldTimer > 0.f) {
            tile.soldTimer -= dt;
            if (tile.soldTimer <= 0.f) {
                tile.soldTimer = 0.f;
                tile.soldCrop = CropType::None;
            }
        }
        // update seed-taken timers as well
        if (tile.seedTakenTimer > 0.f) {
            tile.seedTakenTimer -= dt;
            if (tile.seedTakenTimer <= 0.f) {
                tile.seedTakenTimer = 0.f;
                tile.seedTakenCrop = CropType::None;
            }
        }
    }

    // update global timer
    gameTimer -= dt;
    if (gameTimer <= 0.f) {
        gameTimer = 0.f;
        if (!EndGame) {
            EndGame = true;
            // Determine winner with tiebreakers:
            if (playerFarmer.score > aiFarmer.score) {
                winner = Winner::Player;
            } else if (aiFarmer.score > playerFarmer.score) {
                winner = Winner::AI;
            } else {
                // tie on score -> compare number of requests dominated
                if (playerRequestsCompleted > aiRequestsCompleted) winner = Winner::Player;
                else if (aiRequestsCompleted > playerRequestsCompleted) winner = Winner::AI;
                else {
                    // still tie -> compare correct deliveries
                    if (playerCorrectDeliveries > aiCorrectDeliveries) winner = Winner::Player;
                    else if (aiCorrectDeliveries > playerCorrectDeliveries) winner = Winner::AI;
                    else winner = Winner::Tie;
                }
            }
        }
            // Save the player's score for this level (always overwrite).
            if (!scoreSaved) {
                int idx = levelID - 1;
                if (idx < 0) idx = 0;
                if ((int)PlayerSave::activePlayer.highScores.size() <= idx) {
                    PlayerSave::activePlayer.highScores.resize(idx + 1, 0);
                }
                PlayerSave::activePlayer.highScores[idx] = playerFarmer.score;
                PlayerSave::activePlayer.saveToFile();
                scoreSaved = true;
            }
    }

    if (hasFont) {
        timerText.setString(std::to_string(static_cast<int>(gameTimer)) + "s");
        playerScoreText.setString("You: " + std::to_string(playerFarmer.score) + "  Req: " + std::to_string(playerRequestsCompleted));
        aiScoreText.setString("AI: " + std::to_string(aiFarmer.score) + "  Req: " + std::to_string(aiRequestsCompleted));
    }

    // AI state machine & path-following 
    auto aiPos = aiFarmer.body.getPosition();

    // Helper to set path to a tile index
    auto setAIPathToTile = [&](int tileIdx){
        int start = tileIndexFromPos(aiPos);
        if (start < 0) {
            // fallback: compute from current position -> approximate nearest tile
            // pick the tile under ai
            start = tileIndexFromPos(aiFarmer.body.getPosition());
        }
        aiPath = findPathAStar(start, tileIdx);
        aiPathIndex = 0;

        // If no path found (often because the goal is on the player's side),
        // try a fallback: find the nearest suitable tile on the AI's right side
        // and attempt to path to that instead.
        if (aiPath.empty()) {
            if (tileIdx < 0 || tileIdx >= static_cast<int>(farm.size())) return;

            // compute right edge of divider
            sf::FloatRect wall = centerPath.getGlobalBounds();
            float wallRightX = wall.left + wall.width;

            // determine what kind of tile we were trying to reach
            GroundType targetType = farm[tileIdx].type;
            CropType targetCrop = farm[tileIdx].crop;
            TileState targetState = farm[tileIdx].state;

            int bestCandidate = -1;
            float bestDist = std::numeric_limits<float>::max();

            for (int i = 0; i < static_cast<int>(farm.size()); ++i) {
                // must be on the right side of the wall
                sf::Vector2f tc = tileCenter(i);
                if (tc.x <= wallRightX) continue;

                // must be walkable
                if (!isTileWalkable(i)) continue;

                // match candidate to the original target semantics
                bool match = false;
                if (targetType == GroundType::Seeds) {
                    match = (farm[i].type == GroundType::Seeds && farm[i].crop == targetCrop);
                } else if (targetType == GroundType::Soil) {
                    // prefer soil tiles that are empty (planting target)
                    match = (farm[i].type == GroundType::Soil && farm[i].state == TileState::Empty);
                } else if (targetType == GroundType::Market) {
                    match = (farm[i].type == GroundType::Market);
                } else {
                    // generic fallback: allow any walkable tile on right side
                    match = true;
                }

                if (!match) continue;

                float d = std::hypot(tc.x - aiPos.x, tc.y - aiPos.y);
                if (d < bestDist) {
                    bestDist = d;
                    bestCandidate = i;
                }
            }

            if (bestCandidate >= 0) {
                auto tryPath = findPathAStar(start, bestCandidate);
                if (!tryPath.empty()) {
                    aiPath = std::move(tryPath);
                    aiPathIndex = 0;
                }
            }
        }
    };

    auto moveAIAlongPath = [&](float dt)->bool {
        if (aiPath.empty() || aiPathIndex >= static_cast<int>(aiPath.size())) return false;
        sf::Vector2f target = tileCenter(aiPath[aiPathIndex]);
        sf::Vector2f dir = target - aiPos;
        float dist = std::sqrt(dir.x*dir.x + dir.y*dir.y);
        if (dist < aiArriveThreshold) { // reached waypoint
            aiPathIndex++;
            return true;
        }
        // normalise and apply speed (seek)
        dir /= dist;
        sf::Vector2f vel = dir * (aiMaxSpeed * dt);
        // move ai, but keep on right side of centerPath wall as before
        sf::FloatRect wall = centerPath.getGlobalBounds();
        float ar = aiFarmer.body.getRadius();
        sf::Vector2f next = aiPos + vel;
        sf::FloatRect aiCircle(next.x - ar, next.y - ar, 2*ar, 2*ar);
        // ensure inside play area and right of wall (reuse your earlier checks)
        float playTop = board.box.getPosition().y + board.box.getSize().y;
        float playLeft = 0.f;
        float playRight = static_cast<float>(window.getSize().x);
        float playBottom = static_cast<float>(window.getSize().y);

        sf::FloatRect playArea(playLeft, playTop, playRight - playLeft, playBottom - playTop);
        bool aiInside =
            playArea.contains(aiCircle.left, aiCircle.top) &&
            playArea.contains(aiCircle.left + aiCircle.width, aiCircle.top) &&
            playArea.contains(aiCircle.left, aiCircle.top + aiCircle.height) &&
            playArea.contains(aiCircle.left + aiCircle.width, aiCircle.top + aiCircle.height);

        bool rightOfWall = (next.x - ar) >= (wall.left + wall.width);

        if (aiInside && rightOfWall) {
            aiFarmer.body.setPosition(next);
        } else {
            // cannot move directly; clear path so next iteration recalculates
            aiPath.clear();
        }
        return true;
    };

    // AI decision helper: choose a crop requested (highest remaining qty) or nearest seed if none
    auto chooseTargetCrop = [&]()->CropType {
        if (currentRequestIndex >= 0 && currentRequestIndex < static_cast<int>(requests.size())) {
            Request& r = requests[currentRequestIndex];
            // choose highest qty remaining
            int bestQty = 0;
            CropType best = CropType::None;
            for (auto &it : r.items) {
                if (it.second > bestQty) { bestQty = it.second; best = it.first; }
            }
            if (best != CropType::None) return best;
        }
        // fallback: pick first allowed crop that exists in seed boxes
        for (int i = 0; i < static_cast<int>(farm.size()); ++i) {
            if (farm[i].type == GroundType::Seeds && farm[i].crop != CropType::None) {
                return farm[i].crop;
            }
        }
        return CropType::None;
    };

    // State machine transitions & actions
    switch (aiState) {
        case AIState::SelectGoal: {
            aiTargetCrop = chooseTargetCrop();
            if (aiTargetCrop == CropType::None) {
                aiState = AIState::Idle;
                break;
            }
            // find nearest seed tile for that crop
            int bestIdx = -1; float bestDist = 1e9;
            for (int i = 0; i < static_cast<int>(farm.size()); ++i) {
                if (farm[i].type == GroundType::Seeds && farm[i].crop == aiTargetCrop) {
                    float d = std::hypot(tileCenter(i).x - aiPos.x, tileCenter(i).y - aiPos.y);
                    if (d < bestDist) { bestDist = d; bestIdx = i; }
                }
            }
            if (bestIdx >= 0) {
                setAIPathToTile(bestIdx);
                aiState = AIState::GoToSeeds;
            } else {
                // no seeds available: idle for a moment
                aiState = AIState::Idle;
            }
            break;
        }

        case AIState::GoToSeeds: {
            if (aiPath.empty()) {
                // recompute path to nearest seed tile
                int targetIdx = -1; float bestDist = 1e9;
                for (int i = 0; i < static_cast<int>(farm.size()); ++i) {
                    if (farm[i].type == GroundType::Seeds && farm[i].crop == aiTargetCrop) {
                        float d = std::hypot(tileCenter(i).x - aiPos.x, tileCenter(i).y - aiPos.y);
                        if (d < bestDist) { bestDist = d; targetIdx = i; }
                    }
                }
                if (targetIdx >= 0) setAIPathToTile(targetIdx);
                else aiState = AIState::SelectGoal;
                break;
            }
            // follow the path
            moveAIAlongPath(dt);
            // If close enough to final goal tile, simulate 'take seed' like player `T` does
            if (aiPathIndex >= static_cast<int>(aiPath.size())) {
                int finalTile = aiPath.empty() ? -1 : aiPath.back();
                if (finalTile >= 0) {
                    // perform take seed
                    if (farm[finalTile].type == GroundType::Seeds && !aiFarmer.hasSeed && farm[finalTile].crop == aiTargetCrop) {
                        aiFarmer.carriedSeed = farm[finalTile].crop;
                        aiFarmer.hasSeed = true;
                        // optionally: leave the seed box as is (multiple seeds) or mark as taken
                        std::cout << "AI: took " << cropName(aiFarmer.carriedSeed) << " seed\n";
                        // seed-taken visual for AI taking a seed
                        farm[finalTile].seedTakenTimer = seed_take_visual_temp;
                        farm[finalTile].seedTakenCrop = farm[finalTile].crop;
                        aiState = AIState::GoToPlant;
                        // Choose planting spot: nearest soil empty tile
                        int plantIdx = -1; float bd = 1e9;
                        for (int i = 0; i < static_cast<int>(farm.size()); ++i) {
                            if (farm[i].type == GroundType::Soil && farm[i].state == TileState::Empty) {
                                float d = std::hypot(tileCenter(i).x - aiPos.x, tileCenter(i).y - aiPos.y);
                                if (d < bd) { bd = d; plantIdx = i; }
                            }
                        }
                        if (plantIdx >= 0) setAIPathToTile(plantIdx);
                        else aiState = AIState::Idle;
                    } else {
                        aiState = AIState::SelectGoal;
                    }
                } else {
                    aiState = AIState::SelectGoal;
                }
            }
            break;
        }

        case AIState::GoToPlant: {
            if (aiPath.empty()) {
                // no available planting tile: return to select
                aiState = AIState::SelectGoal;
                break;
            }
            moveAIAlongPath(dt);
            if (aiPathIndex >= static_cast<int>(aiPath.size())) {
                // arrived at tile: plant if possible
                int finalTile = aiPath.empty() ? -1 : aiPath.back();
                if (finalTile >= 0 && aiFarmer.hasSeed && farm[finalTile].type == GroundType::Soil && farm[finalTile].state == TileState::Empty) {
                    farm[finalTile].state = TileState::Seeded;
                    farm[finalTile].growthTimer = 0.f;
                    farm[finalTile].crop = aiFarmer.carriedSeed;
                    aiFarmer.hasSeed = false;
                    aiFarmer.carriedSeed = CropType::None;
                    std::cout << "AI: planted\n";
                    aiState = AIState::WaitForGrowth;
                } else {
                    aiState = AIState::SelectGoal;
                }
            }
            break;
        }

        case AIState::WaitForGrowth: {
            // look for any grown crop of aiTargetCrop to harvest
            int grownIdx = -1; float bd = 1e9;
            for (int i = 0; i < static_cast<int>(farm.size()); ++i) {
                if (farm[i].type == GroundType::Soil && farm[i].state == TileState::Grown && farm[i].crop == aiTargetCrop) {
                    float d = std::hypot(tileCenter(i).x - aiPos.x, tileCenter(i).y - aiPos.y);
                    if (d < bd) { bd = d; grownIdx = i; }
                }
            }
            if (grownIdx >= 0) {
                setAIPathToTile(grownIdx);
                aiState = AIState::Harvest;
            } else {
                // do nothing this frame; you might let the AI wander or idle
                // we'll let it remain in WaitForGrowth and recheck next frame
            }
            break;
        }

        case AIState::Harvest: {
            if (aiPath.empty()) {
                aiState = AIState::WaitForGrowth;
                break;
            }
            moveAIAlongPath(dt);
            if (aiPathIndex >= static_cast<int>(aiPath.size())) {
                int finalTile = aiPath.empty() ? -1 : aiPath.back();
                if (finalTile >= 0 && farm[finalTile].state == TileState::Grown) {
                    // harvest - mimic player logic
                    farm[finalTile].state = TileState::Empty;
                    aiFarmer.carriedSeed = farm[finalTile].crop;
                    farm[finalTile].growthTimer = 0.f;
                    farm[finalTile].rect.setFillColor(sf::Color(102, 51, 0)); // back to soil
                    aiFarmer.hasProduct = true;
                    std::cout << "AI: harvested " << cropName(aiFarmer.carriedSeed) << "\n";
                    // go to market
                    // find market tile
                    int marketIdx = -1;
                    float bestD = 1e9;
                    for (int i = 0; i < static_cast<int>(farm.size()); ++i) {
                        if (farm[i].type == GroundType::Market) {
                            float d = std::hypot(tileCenter(i).x - aiPos.x, tileCenter(i).y - aiPos.y);
                            if (d < bestD) { bestD = d; marketIdx = i; }
                        }
                    }
                    if (marketIdx >= 0) { setAIPathToTile(marketIdx); aiState = AIState::GoToMarket; }
                    else aiState = AIState::SelectGoal;
                } else {
                    aiState = AIState::WaitForGrowth;
                }
            }
            break;
        }

        case AIState::GoToMarket: {
            if (aiPath.empty()) { aiState = AIState::SelectGoal; break; }
            moveAIAlongPath(dt);
            if (aiPathIndex >= static_cast<int>(aiPath.size())) {
                int finalTile = aiPath.empty() ? -1 : aiPath.back();
                if (finalTile >= 0 && farm[finalTile].type == GroundType::Market && aiFarmer.hasProduct) {
                    // sell to current request (reuse your player selling logic)
                    CropType product = aiFarmer.carriedSeed;
                    if (currentRequestIndex >= 0 && currentRequestIndex < static_cast<int>(requests.size())) {
                        Request& r = requests[currentRequestIndex];
                        bool completed = false;
                        for (auto& item : r.items) {
                            if (item.first == product && item.second > 0) {
                                item.second -= 1;
                                // find index to credit the AI
                                for (size_t j = 0; j < r.items.size(); ++j) {
                                    if (r.items[j].first == product) { r.aiContrib[j] += 1; break; }
                                }
                                completed = true;
                                aiFarmer.score += 5; // 5 points per correct delivery
                                aiCorrectDeliveries += 1;
                                std::cout << "AI score +5\n";
                                std::cout << "AI score: " << aiFarmer.score << "\n";
                                break;
                            }
                        }
                        if (completed) {
                            std::cout << "AI: delivered " << cropName(product) << " for the request\n";
                            updateCurrentRequestText();
                            // show temporary sold visual on that market tile
                            if (finalTile >= 0 && finalTile < static_cast<int>(farm.size())) {
                                farm[finalTile].soldTimer = sold_visual_temp;
                                farm[finalTile].soldCrop = product;
                            }
                            bool allDone = true;
                            for (const auto& it : r.items) if (it.second > 0) { allDone = false; break; }
                            if (allDone) {
                                // determine exclusivity
                                bool playerExclusive = true;
                                bool aiExclusive = true;
                                int totalQty = 0;
                                for (size_t i = 0; i < r.items.size(); ++i) {
                                    totalQty += r.initialQty[i];
                                    if (r.playerContrib[i] != r.initialQty[i]) playerExclusive = false;
                                    if (r.aiContrib[i] != r.initialQty[i]) aiExclusive = false;
                                }
                                if (playerExclusive) {
                                    playerRequestsCompleted += 1;
                                    playerFarmer.score += totalQty;
                                }
                                if (aiExclusive) {
                                    aiRequestsCompleted += 1;
                                    aiFarmer.score += totalQty;
                                }

                                currentRequestIndex++;
                                if (currentRequestIndex < static_cast<int>(requests.size()) && hasFont) {
                                    currentRequestText.setString("Request: " + requestToString(requests[currentRequestIndex]));
                                } else {
                                    currentRequestText.setString("All requests completed!");
                                }
                                updateCurrentRequestText();
                            }
                        } else {
                            std::cout << "AI: wrong product for current request\n";
                        }
                    }
                    aiFarmer.hasProduct = false;
                    aiFarmer.carriedSeed = CropType::None;
                    aiState = AIState::SelectGoal;
                } else {
                    aiState = AIState::SelectGoal;
                }
            }
            break;
        }

        case AIState::Idle:
        default: {
            // every few seconds re-evaluate
            static float idleTimer = 0.f;
            idleTimer += dt;
            if (idleTimer > 0.2f) {
                idleTimer = 0.f;
                aiState = AIState::SelectGoal;
            }
            break;
        }
    } // end switch

    if (popup.active) {
        popup.timer += dt;
        if (popup.timer >= popup.duration) {
            popup.active = false; // stop showing it
        }
    }
}

void Game::draw() {
    window.clear(sf::Color(20, 40, 60));

    window.draw(topBar); // level design
    window.draw(centerPath);

    window.draw(backButton.box); // HUD / TASKS
    window.draw(backButton.sprite);

    window.draw(pauseButton.box); //show pause button
    window.draw(pauseButton.sprite);

    window.draw(board.box); //show info bar

    if (hasFont) { //show text
        window.draw(playerScoreText);
        window.draw(aiScoreText);
        window.draw(timerText);
        window.draw(currentRequestText);
    }

    // Farm
    for (auto& tile : farm) {
        window.draw(tile.rect);

        //Seed box icons
        if ((tile.type == GroundType::Seeds && tile.crop != CropType::None) || (tile.type == GroundType::Soil && tile.state == TileState::Grown && tile.crop != CropType::None)) {
            sf::Sprite cropSprite;
            cropSprite.setTexture(seedTexture(tile.crop));
            cropSprite.setPosition(tile.rect.getPosition());

            // scale down the sprite to fit in tile
            auto texSize = cropSprite.getTexture()->getSize();
            auto tileSize = tile.rect.getSize();
            cropSprite.setScale( tileSize.x / texSize.x, tileSize.y/ texSize.y);
            window.draw(cropSprite);
        }

        // Draw temporary sold visual if present (fades out)
        // seed-taken visual: small sprite that rises and fades
        if (tile.seedTakenTimer > 0.f && tile.seedTakenCrop != CropType::None) {
            sf::Sprite takenSprite;
            takenSprite.setTexture(seedTexture(tile.seedTakenCrop));
            // position starts at tile center
            auto tilePos = tile.rect.getPosition();
            auto tileSize = tile.rect.getSize();
            takenSprite.setOrigin(0.f, 0.f);
            // compute fraction (1.0 -> just started, 0.0 -> finished)
            float fracSeed = std::max(0.f, tile.seedTakenTimer / seed_take_visual_temp);
            // upward offset so the sprite rises while fading
            float yOffset = (1.f - fracSeed) * (tileSize.y * 0.6f);
            takenSprite.setPosition(tilePos.x, tilePos.y - yOffset);
            auto texSz = takenSprite.getTexture()->getSize();
            // smaller scale than full tile
            float scale = 0.6f;
            takenSprite.setScale((tileSize.x * scale) / texSz.x, (tileSize.y * scale) / texSz.y);
            sf::Color tc = takenSprite.getColor();
            tc.a = static_cast<sf::Uint8>(255.f * fracSeed);
            takenSprite.setColor(tc);
            window.draw(takenSprite);
        }

        if (tile.soldTimer > 0.f && tile.soldCrop != CropType::None) {
            sf::Sprite soldSprite;
            soldSprite.setTexture(seedTexture(tile.soldCrop));
            soldSprite.setPosition(tile.rect.getPosition());
            auto texSize2 = soldSprite.getTexture()->getSize();
            auto tileSize2 = tile.rect.getSize();
            soldSprite.setScale(tileSize2.x / texSize2.x, tileSize2.y / texSize2.y);

            // alpha proportional to remaining time (fade out)
            float frac = std::min(1.f, tile.soldTimer / sold_visual_temp);
            sf::Color c = soldSprite.getColor();
            c.a = static_cast<sf::Uint8>(255.f * frac);
            soldSprite.setColor(c);

            window.draw(soldSprite);
        }
    }

    // Farmers
    if (gAppearance.playerUseSprite) {
        window.draw(playerFarmer.sprite);
    } else {
        window.draw(playerFarmer.body);   // circle
    }

    // AI
    if (gAppearance.aiUseSprite) {
        window.draw(aiFarmer.sprite);
    } else {
        window.draw(aiFarmer.body);
    }

    // Pause popup
    if (PauseGame && hasFont) {
        sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);

        sf::Text text("Game Paused \nPress Space bar to continue \n\n Press T to view game tutorial", font, 28);
        text.setFillColor(sf::Color::White);
        auto tb = text.getLocalBounds();
        text.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        text.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
        window.draw(text);
    }

    // End of game popup
    if (EndGame && hasFont) {
        sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);

        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);

        // End of game message
        std::string msg = "Game Over\n\n";
        msg += "Scores:\n";
        msg += "You: " + std::to_string(playerFarmer.score) + "   AI: " + std::to_string(aiFarmer.score) + "\n\n";
        msg += "Requests dominated:\n";
        msg += "You: " + std::to_string(playerRequestsCompleted) + "/" + std::to_string(numRequestsForLevel(levelID)) + "   AI: " + std::to_string(aiRequestsCompleted) + "/" + std::to_string(numRequestsForLevel(levelID)) + "\n";
        msg += "Correct deliveries:\n";
        msg += "You: " + std::to_string(playerCorrectDeliveries) + "   AI: " + std::to_string(aiCorrectDeliveries) + "\n\n";

        if (winner == Winner::Player) {
            msg += "You won!\n\n";
        } else if (winner == Winner::AI) {
            msg += "AI won!\n\n";
        } else {
            msg += "It's a tie! \n\n";
        }

        msg += "Press P to play again\n";
        msg += "Press M to go back to Menu";

        text.setString(msg);

        auto tb = text.getLocalBounds();
        text.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        text.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
        window.draw(text);
    }

    // Tutorial popup
    if (Tutorial && hasFont) {
        sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);

        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color::White);


        std::string msg = "Game Tutorial (Press C to close)\n\n";
        msg += "Move the player using the < > and up/down keys. \n\n";
        msg += "Game Goal:  Compete against AI to complete the most requests before time runs out!\n\n";
        msg += "To complete a request:\n";
        msg += "  1. Take a vegetable seed (ex tomato seed) from the seed boxes (left bar) \n";
        msg += "  To take something using the key T on your keyboard\n";
        msg += "  2. Plant the seed in an empty soil tile (brown square in the farm area) \n";
        msg += "      Plant using the key D for drop on your keyboard \n";
        msg += "  3. Wait for the crop to grow (the vegetable sprite will appear when grown) \n";
        msg += "  4. Harvest the crop using the key T on your keyboard \n";
        msg += "  5. Deliver the vegetable to the market (bottom green bar) using the key D \n\n";
        msg += "Do that until you have delivered all the vegetables requested! \n\n";
        msg += "Use key Q to quit the game anytime.\n";
        msg += "\nGood luck and have fun! (Press C to close) tutorial";    

        text.setString(msg);
       
        sf::FloatRect tb = text.getLocalBounds();
        text.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        text.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
        window.draw(text);
    }

    if (popup.active) {
        if (popup.useText)
            window.draw(popup.text);
        //else
            //window.draw(popup.sprite);
    }

    window.display();
}
