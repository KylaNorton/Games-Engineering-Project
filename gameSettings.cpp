#include "gameSettings.hpp"



GameSettings::GameSettings(sf::RenderWindow& window)
    : window(window)
{
    // load a font
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) {
        std::cerr << "[WARN] Font not found at res/fonts/Inter-Regular.ttf. "
                     "Buttons will show without text.\n";
    }

    const sf::Vector2f size(320.f, 64.f);
    const float cx = window.getSize().x * 0.5f;
    auto pos = [&](float y){ return sf::Vector2f(cx - size.x * 0.5f, y); };

    // Center buttons
    setupButton(buttons[0], "New Game",   pos(180.f));
    setupButton(buttons[1], "Play Again", pos(270.f));

    // Account button (top-right, smaller)
    const sf::Vector2f accSize(160.f, 40.f);
    sf::Vector2f accPos(
        window.getSize().x - accSize.x - 30.f,
        30.f
    );
    setupButton(buttons[2], "Account", accPos);
    buttons[2].box.setSize(accSize);
    if (hasFont) centerLabel(buttons[2]);
}

void GameSettings::setupButton(Button& b, const std::string& text, sf::Vector2f p) {
    // Rectangle
    b.box.setSize({320.f, 64.f});
    b.box.setPosition(p);
    b.box.setFillColor(idle);

    // Label (only if font loaded)
    if (hasFont) {
        b.label.setFont(font);
        b.label.setString(text);
        b.label.setCharacterSize(28);
        b.label.setFillColor(textColor);
        centerLabel(b);
    }
}

void GameSettings::centerLabel(Button& b) {
    // Center text inside the button rect
    auto tb = b.label.getLocalBounds();
    b.label.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    b.label.setPosition(b.box.getPosition() + b.box.getSize() * 0.5f);
}

void GameSettings::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseMoved)
        checkHover();

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        if (buttons[0].contains(window, mp)) action = GameSetAction::NewGame;
        else if (buttons[1].contains(window, mp)) action = GameSetAction::PlayAgain;
        else if (buttons[2].contains(window, mp)) action = GameSetAction::Account;
    }

    if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::Q) {
                window.close();                //quit app
            }
    }
}

void GameSettings::checkHover() {
    sf::Vector2i mp = sf::Mouse::getPosition(window);
    resetColors();
    for (auto& b : buttons) {
        if (b.contains(window, mp)) {
            b.box.setFillColor(hover);
            // keep label centered in case size/pos changes later
            if (hasFont) centerLabel(b);
        }
    }
}

void GameSettings::resetColors() {
    for (auto& b : buttons)
        b.box.setFillColor(idle);
}

void GameSettings::draw() {
    window.clear(bgColor);
    for (auto& b : buttons) {
        window.draw(b.box);
        if (hasFont) window.draw(b.label);
    }
    window.display();
}

void GameSettings::recomputeLayout() {
    const sf::Vector2f size(320.f, 64.f);
    const float cx = window.getSize().x * 0.5f;
    setupButton(buttons[0], "New Game",   sf::Vector2f(cx - size.x * 0.5f, 180.f));
    setupButton(buttons[1], "Play Again", sf::Vector2f(cx - size.x * 0.5f, 260.f));

    // Account button top-right
    const sf::Vector2f accSize(160.f, 40.f);
    sf::Vector2f accPos(window.getSize().x - accSize.x - 30.f, 30.f);
    setupButton(buttons[2], "Account", accPos);
    buttons[2].box.setSize(accSize);
    if (hasFont) centerLabel(buttons[2]);
}