#include "game.hpp"
#include <iostream>

Game::Game(sf::RenderWindow& win) : window(win) {

    // Load font for pause popup
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) {
        std::cerr << "[WARN] Font not found at res/fonts/Inter-Regular.ttf. "
                     "Buttons will show without text.\n";
    }

    // Load icon textures
    static sf::Texture backTexture;
    static sf::Texture pauseTexure;

    backTexture.loadFromFile("res/icons/arrow.png");
    pauseTexure.loadFromFile("res/icons/pause-play.png");

    // BACK BUTTON (top-left)
    backButton.box.setSize({50, 50});
    backButton.box.setFillColor(sf::Color(0, 0, 0, 0));
    backButton.box.setPosition({20, 20});

    backButton.sprite.setTexture(backTexture);
    backButton.sprite.setScale(0.06f, 0.06f);
    backButton.sprite.setPosition({25, 25});

    // PAUSE BUTTON (top-right)
    pauseButton.box.setSize({50, 50});
    pauseButton.box.setFillColor(sf::Color(0, 0, 0, 0));
    pauseButton.box.setPosition({window.getSize().x - 70.f, 20});

    pauseButton.sprite.setTexture(pauseTexure);
    pauseButton.sprite.setScale(0.08f, 0.08f);
    pauseButton.sprite.setPosition({window.getSize().x - 65.f, 25});

    //INFO BOARD   
    board.box.setSize({800.f, 50.f});
    board.box.setFillColor(sf::Color(50, 50, 50));
    board.box.setPosition({(window.getSize().x - 800.f)/2.f, 20});

    score.box.setSize({});
    score.box.setFillColor(sf::Color(0, 0, 0, 0));
    score.box.setPosition({window.getSize().x - 70.f, 20});

    timer.box.setSize({});
    timer.box.setFillColor(sf::Color(0, 0, 0, 0));
    timer.box.setPosition({window.getSize().x - 70.f, 20});

    if (hasFont) {
        // Player score text (left side)
        playerScoreText.setFont(font);
        playerScoreText.setCharacterSize(20);
        playerScoreText.setFillColor(sf::Color::White);
        playerScoreText.setString("You: 0");
        playerScoreText.setPosition(board.box.getPosition().x + 20.f,
                                    board.box.getPosition().y + 15.f);

        // AI score text (right side)
        aiScoreText.setFont(font);
        aiScoreText.setCharacterSize(20);
        aiScoreText.setFillColor(sf::Color::White);
        aiScoreText.setString("AI: 0");
        aiScoreText.setPosition(board.box.getPosition().x + board.box.getSize().x - 120.f,
                                board.box.getPosition().y + 15.f);

        // Timer text (center)
        timerText.setFont(font);
        timerText.setCharacterSize(20);
        timerText.setFillColor(sf::Color::White);
        timerText.setString("60.0s");

        float centerX = board.box.getPosition().x + board.box.getSize().x / 2.f;
        timerText.setPosition(centerX - 30.f, board.box.getPosition().y + 15.f);
    }

    // --- FARM GRID ---
    farm.clear();
    float startX = (window.getSize().x - gridCols * tileSize) / 2.f;
    float startY = 120.f; // under the info board

    for (int row = 0; row < gridRows; ++row) {
        for (int col = 0; col < gridCols; ++col) {
            FarmTile tile;
            tile.rect.setSize({tileSize - 4.f, tileSize - 4.f}); // little margin
            tile.rect.setPosition({
                startX + col * tileSize + 2.f,
                startY + row * tileSize + 2.f
            });
            tile.rect.setFillColor(sf::Color(90, 60, 30)); // brown soil
            farm.push_back(tile);
        }
    }

    // --- PLAYER FARMER ---
    playerFarmer.body.setRadius(18.f);
    playerFarmer.body.setOrigin(18.f, 18.f);
    playerFarmer.body.setFillColor(sf::Color::Cyan);
    playerFarmer.body.setPosition(window.getSize().x * 0.3f, startY + gridRows * tileSize + 80.f);

    // --- AI FARMER ---
    aiFarmer.body.setRadius(18.f);
    aiFarmer.body.setOrigin(18.f, 18.f);
    aiFarmer.body.setFillColor(sf::Color::Red);
    aiFarmer.body.setPosition(window.getSize().x * 0.7f, startY + gridRows * tileSize + 80.f);
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
                PauseGame = false;           // close popup, back to game
            }
        }
        return; // while popup is open, ignore other events
    }

        // --- PLANT / HARVEST on Space ---
    if (!PauseGame && e.type == sf::Event::KeyPressed &&
        e.key.code == sf::Keyboard::Space) {

        // Player interacts with the tile under them
        sf::Vector2f p = playerFarmer.body.getPosition();

        for (auto& tile : farm) {
            if (tile.rect.getGlobalBounds().contains(p)) {
                if (tile.state == TileState::Empty) {
                    // plant
                    tile.state = TileState::Planted;
                    tile.growthTimer = 0.f;
                    tile.rect.setFillColor(sf::Color(120, 80, 40)); // darker soil
                }
                else if (tile.state == TileState::Grown) {
                    // harvest
                    tile.state = TileState::Empty;
                    tile.growthTimer = 0.f;
                    tile.rect.setFillColor(sf::Color(90, 60, 30)); // back to soil
                    playerFarmer.score += 1;
                    std::cout << "Player score: " << playerFarmer.score << "\n";
                }
                break;
            }
        }
    }

}

void Game::update(float dt) {
    if (PauseGame) return; // don't update when paused

    // --- PLAYER MOVEMENT ---
    sf::Vector2f v(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  v.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) v.x += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    v.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  v.y += 1.f;
    playerFarmer.body.move(v * speed * dt);

    // --- AI MOVEMENT (super dumb version) ---
    // AI just wanders horizontally across the farm
    static float aiDir = 1.f;
    aiFarmer.body.move(aiDir * speed * 0.5f * dt, 0.f);

    // bounce AI at screen edges
    float x = aiFarmer.body.getPosition().x;
    if (x < 50.f || x > window.getSize().x - 50.f)
        aiDir *= -1.f;

    // --- GROW CROPS ---
    for (auto& tile : farm) {
        if (tile.state == TileState::Planted) {
            tile.growthTimer += dt;
            if (tile.growthTimer > 3.f) { // 3 seconds to grow
                tile.state = TileState::Grown;
                tile.rect.setFillColor(sf::Color(50, 200, 50)); // green = ready
            }
        }
    }

    // --- update global timer ---
    gameTimer -= dt;
    if (gameTimer < 0.f) gameTimer = 0.f; // stop at zero for now

    if (hasFont) {
        // update timer display
        timerText.setString(std::to_string(static_cast<int>(gameTimer)) + "s");

        // update score display
        playerScoreText.setString("You: " + std::to_string(playerFarmer.score));
        aiScoreText.setString("AI: " + std::to_string(aiFarmer.score));
    }

}

void Game::draw() {
    window.clear(sf::Color(20, 40, 60));

    window.draw(backButton.box); //show back to menu button 
    window.draw(backButton.sprite);

    window.draw(pauseButton.box); //show pause button
    window.draw(pauseButton.sprite);

    window.draw(board.box); //show info bar

    if (hasFont) {
        window.draw(playerScoreText);
        window.draw(aiScoreText);
        window.draw(timerText);
    }

    // --- FARM ---
    for (auto& tile : farm)
        window.draw(tile.rect);

    // --- FARMERS ---
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

    window.display();
}
