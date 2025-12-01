#include "game.hpp"
#include <iostream>

Game::Game(sf::RenderWindow& win) : window(win) {

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
    backButton.box.setPosition({20.f, 20.f});

    backButton.sprite.setTexture(backTexture);
    backButton.sprite.setScale(0.06f, 0.06f);
    backButton.sprite.setPosition({25.f, 25.f});

    // PAUSE BUTTON (top-right)
    pauseButton.box.setSize({50.f, 50.f});
    pauseButton.box.setFillColor(sf::Color(0, 0, 0, 0));
    pauseButton.box.setPosition({window.getSize().x - 70.f, 20.f});

    pauseButton.sprite.setTexture(pauseTexure);
    pauseButton.sprite.setScale(0.08f, 0.08f);
    pauseButton.sprite.setPosition({window.getSize().x - 65.f, 25.f});

    // --------------------------------------------------
    // 3) BASIC WINDOW DIMENSIONS
    // --------------------------------------------------
    const float winW = static_cast<float>(window.getSize().x);
    const float winH = static_cast<float>(window.getSize().y);

    const float topBarHeight    = 80.f;  // coloured strip at the top
    const float bottomBarHeight = 0.f;   // set to 0 for now, no MARKER bar
    const float sideBarWidth    = 0.f;   // no gameplay side bars now

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
    board.box.setPosition({(winW - 800.f) / 2.f, 20.f});

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
        playerScoreText.setString("You: 0");
        playerScoreText.setPosition(board.box.getPosition().x + 20.f, board.box.getPosition().y + 15.f);

        // AI score text (right)
        aiScoreText.setFont(font);
        aiScoreText.setCharacterSize(20);
        aiScoreText.setFillColor(sf::Color::White);
        aiScoreText.setString("AI: 0");
        aiScoreText.setPosition(board.box.getPosition().x + board.box.getSize().x - 120.f, board.box.getPosition().y + 15.f);

        // Timer text (center)
        timerText.setFont(font);
        timerText.setCharacterSize(20);
        timerText.setFillColor(sf::Color::White);
        timerText.setString("60s");
        float centerX = board.box.getPosition().x + board.box.getSize().x / 2.f;
        timerText.setPosition(centerX - 20.f, board.box.getPosition().y + 15.f);
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

            t.type        = GroundType::Empty;        // will become PlayerSoil/AISoil later
            t.walkable    = true;                     // player can walk on all ground
            t.state       = TileState::Empty;         // nothing planted yet
            t.growthTimer = 0.f;
            t.rect.setFillColor(sf::Color(90, 60, 30)); // brown soil colour
        }
    }

    // --------------------------------------------------
    // 9) OPTIONAL CENTER LINE (only visual, no collision)
    // --------------------------------------------------
        centerPath.setSize({4.f, playHeight});
        centerPath.setPosition(winW / 2.f - 2.f, playTop);
        centerPath.setFillColor(sf::Color(255, 0, 0)); // red line

    int midCol = gridCols / 2; 
    int bottomRow = gridRows - 1; // last row (market + sun)
    int topFarmRow = 1; // row 1 to 3 farm zone
    int bottomFarmRow = 3;

    for (int row = 0; row < gridRows; ++row) {
        for (int col = 0; col < gridCols; ++col) {

            int idx = row * gridCols + col;
            FarmTile& t = farm[idx];

            // 8.1 Seeds 
            if ((row >= 0 && row <= bottomFarmRow && (col == 0 || col == gridCols -1)) || (row == 0 && (col == 1 || col == gridCols -2))) 
            {
                t.type = GroundType::Seeds;
                t.rect.setFillColor(sf::Color(255, 128, 0)); // orange
                continue;
            }

            // 8.2 Farm zone "soil"
            bool playerSide = (col > 1 && col < midCol-1);          // columns 2..4
            bool aiSide = (col > midCol && col < gridCols-2); // columns 7..9

            if (row >= topFarmRow && row <= bottomFarmRow && (playerSide || aiSide)) 
            {
                t.type = GroundType::Soil;
                t.rect.setFillColor(sf::Color(102, 51, 0));  // dark brown
                continue;
            }

            // 8.3 Water 
            if (row == bottomFarmRow + 1 && (col == 0 || col == gridCols - 1))
            {
                t.type = GroundType::Water;
                t.rect.setFillColor(sf::Color(153, 204, 255)); // light blue
                continue;
            }

            //  8.4 Sun
            if (row == bottomRow && (col == 0 || col == gridCols - 1))
            {
                t.type = GroundType::Sun;
                t.rect.setFillColor(sf::Color(255, 255, 0)); // yellow
                continue;
            }

            // 8.5 Market
            if (row == bottomRow &&  col >= 3 && col <= gridCols - 4 )
            {
                t.type = GroundType::Market;
                t.rect.setFillColor(sf::Color(51, 102, 0)); // dark green
                continue;
            }

            // 8.6 Trash
            if (row == bottomRow && (col == 1 || col == gridCols - 2))
            {
                t.type = GroundType::Trash;
                t.rect.setFillColor(sf::Color(128, 128, 128)); // grey
                continue;
            }
        }
    }

    // --------------------------------------------------
    // 10) FARMER POSITIONS
    //      - both start roughly in the middle of their half
    // --------------------------------------------------
    sf::Vector2f playerStart(
        winW * 0.25f,                         // quarter of the screen width
        playTop + playHeight * 0.5f           // vertical center of the playable area
    );
    playerFarmer.body.setRadius(18.f);
    playerFarmer.body.setOrigin(18.f, 18.f);
    playerFarmer.body.setFillColor(sf::Color::Cyan);
    playerFarmer.body.setPosition(playerStart);
    playerFarmer.score = 0;

    sf::Vector2f aiStart(winW * 0.75f, playTop + playHeight * 0.5f);  // three quarters of the screen width
    aiFarmer.body.setRadius(18.f);
    aiFarmer.body.setOrigin(18.f, 18.f);
    aiFarmer.body.setFillColor(sf::Color::Red);
    aiFarmer.body.setPosition(aiStart);
    aiFarmer.score = 0;
}

void Game::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f m{(float)e.mouseButton.x, (float)e.mouseButton.y};
        if (backButton.box.getGlobalBounds().contains(m)) {
            action = GameAction::Back;
        }
        if (pauseButton.box.getGlobalBounds().contains(m)) {
            PauseGame = true;
            action = GameAction::Pause;
        }
    }

    if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::Q) {
                window.close();                //quit app
            }
    }

    if (PauseGame) {
        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::Space) {
                PauseGame = false; // close popup, back to game
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
                    //tile.action = ActionType::TakeSeed;
                    playerFarmer.hasSeed = true; 
                    std::cout << "Seed taken\n";
                    break;
                }
                if (tile.type == GroundType::Water && !playerFarmer.hasWater) {
                    // take water
                    //tile.action = ActionType::TakeWater;
                    playerFarmer.hasWater = true;
                    std::cout << "Water taken\n";   
                    break;             
                }
                if (tile.type == GroundType::Sun && !playerFarmer.hasSun) {
                    // take sun
                    //tile.action = ActionType::TakeSun;
                    playerFarmer.hasSun = true;
                    std::cout << "Sun taken\n";
                    break;
                }
                if (tile.state == TileState::Grown && tile.type == GroundType::Soil) {
                    // harvest
                    tile.state = TileState::Empty;
                    tile.growthTimer = 0.f;
                    tile.rect.setFillColor(sf::Color(102, 51, 0)); // back to soil
                    playerFarmer.score += 1;
                    //tile.action = ActionType::Harvest;
                    playerFarmer.hasProduct = true;
                    std::cout << "Crop harvested\n";
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
                    playerFarmer.hasSeed = false;
                    std::cout << "Seed planted\n";
                    //tile.action = ActionType::Plant;
                    tile.rect.setFillColor(sf::Color(51, 25, 0)); // darker soil
                    break;
                }
                if (tile.state == TileState::Seeded && tile.type == GroundType::Soil && playerFarmer.hasWater) {
                    // drop water
                    tile.state = TileState::Watered;
                    tile.growthTimer = 0.f;
                    playerFarmer.hasWater = false;
                    std::cout << "Plant watered\n";
                    //tile.action = ActionType::DropWater;
                    break;
                }
                if (tile.state == TileState::Seeded && tile.type == GroundType::Soil && playerFarmer.hasSun) {
                    // drop sun
                    tile.state = TileState::Suned;
                    tile.growthTimer = 0.f;
                    std::cout << "Sun dropped\n";
                    //tile.action = ActionType::DropSun;
                    playerFarmer.hasSun = false;
                    break;
                } 
                if (tile.type == GroundType::Market && playerFarmer.hasProduct) {
                    // sell product
                    playerFarmer.score += 2; // selling gives 2 points
                    playerFarmer.hasProduct = false;
                    std::cout << "Product sold\n";
                    //tile.action = ActionType::DropProduct;
                    break;
                }
                if (tile.type == GroundType::Trash) {
                    if (playerFarmer.hasSeed) {
                        playerFarmer.hasSeed = false;
                        std::cout << "Seed discarded\n";
                        break;
                    }
                    if (playerFarmer.hasWater) {
                        playerFarmer.hasWater = false;
                        std::cout << "Water discarded\n";
                        break;
                    }
                    if (playerFarmer.hasSun) {
                        playerFarmer.hasSun = false;
                        std::cout << "Sun discarded\n";
                        break;
                    }
                    // if (playerFarmer.hasProduct) {
                    //     playerFarmer.hasProduct = false;
                    //     std::cout << "Product discarded\n";
                    //     break;
                    // }
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
        }
    }

    // --------------------------------------------------
    // 4) AI MOVEMENT (same idea: stays in playArea)
    // --------------------------------------------------
    static float aiDir = 1.f; // 1 = move right, -1 = move left

    sf::Vector2f aiVel(aiDir, 0.f); // simple left-right movement
    sf::Vector2f aiNext = aiFarmer.body.getPosition() + aiVel * (speed * 0.4f * dt);
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
            if (tile.growthTimer > 5.f) { // 5 seconds to grow
                tile.state = TileState::Grown;
                tile.rect.setFillColor(sf::Color(204, 0, 102)); // pink = ready
            }
        }
    }

    // update global timer
    gameTimer -= dt;
    if (gameTimer <= 0.f) {
        gameTimer = 0.f;
        if (!EndGame) {
            EndGame = true;
            if (playerFarmer.score > aiFarmer.score) winner = Winner::Player;
            else if (aiFarmer.score > playerFarmer.score) winner = Winner::AI;
            else winner = Winner::Tie;
        }
    }

    if (hasFont) {
        timerText.setString(std::to_string(static_cast<int>(gameTimer)) + "s");
        playerScoreText.setString("You: " + std::to_string(playerFarmer.score));
        aiScoreText.setString("AI: " + std::to_string(aiFarmer.score));
    }
}

void Game::draw() {
    window.clear(sf::Color(20, 40, 60));

    window.draw(topBar); // level design
    window.draw(bottomBar);
    window.draw(leftBar);
    window.draw(rightBar);
    //window.draw(leftField);
    //window.draw(rightField);
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
    }

    // Farm
    for (auto& tile : farm)
        window.draw(tile.rect);

    // Farmes
    window.draw(playerFarmer.body);
    window.draw(aiFarmer.body);

    // Pause popup
    if (PauseGame && hasFont) {
        sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);

        sf::Text text("Game Paused \nPress Space bar to continue", font, 28);
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
        msg += "You: " + std::to_string(playerFarmer.score) +
            "   AI: " + std::to_string(aiFarmer.score) + "\n\n";

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



    window.display();
}
