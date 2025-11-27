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

    //PLAYER
    player.setRadius(20.f);
    player.setFillColor(sf::Color::Cyan);
    player.setOrigin(player.getRadius(), player.getRadius());
    player.setPosition(window.getSize().x * 0.5f, window.getSize().y * 0.5f);
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
}

void Game::update(float dt) {
    if (PauseGame) return;
    sf::Vector2f v(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  v.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) v.x += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    v.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  v.y += 1.f;
    player.move(v * speed * dt);
}

void Game::draw() {
    window.clear(sf::Color(20, 40, 60));

    window.draw(player); //show player

    window.draw(backButton.box); //show back to menu button 
    window.draw(backButton.sprite);

    window.draw(pauseButton.box); //show pause button
    window.draw(pauseButton.sprite);

    window.draw(board.box); //show info box

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
