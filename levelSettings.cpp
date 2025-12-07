#include "levelSettings.hpp"
#include <iostream>


LevelSettings::LevelSettings(sf::RenderWindow& window) : window(window) {
    // load a font
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) {
        std::cerr << "[WARN] Font not found at res/fonts/Inter-Regular.ttf. "
                     "Buttons will show without text.\n";
    }

    const sf::Vector2f size(320.f, 64.f);
    const float cx = window.getSize().x * 0.5f;
    auto pos = [&](float y){ return sf::Vector2f(cx - size.x * 0.5f, y); };

    setupButton(buttons[0], "Map",    pos(180.f));
    setupButton(buttons[1], "Scores",    pos(270.f));
}

void LevelSettings::setupButton(Button& b, const std::string& text, sf::Vector2f p) {
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

void LevelSettings::centerLabel(Button& b) {
    // Center text inside the button rect
    auto tb = b.label.getLocalBounds();
    b.label.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    b.label.setPosition(b.box.getPosition() + b.box.getSize() * 0.5f);
}

void LevelSettings::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseMoved)
        checkHover();

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        if (buttons[0].contains(window, mp)) action = LevelAction::Map;
        else if (buttons[1].contains(window, mp)) action = LevelAction::Scores;
    }
}

void LevelSettings::checkHover() {
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

void LevelSettings::resetColors() {
    for (auto& b : buttons)
        b.box.setFillColor(idle);
}

void LevelSettings::draw() {
    window.clear(bgColor);
    for (auto& b : buttons) {
        window.draw(b.box);
        if (hasFont) window.draw(b.label);
    }
    window.display();
}

void LevelSettings::recomputeLayout() {
    const sf::Vector2f size(320.f, 64.f);
    const float cx = window.getSize().x * 0.5f;
    auto pos = [&](float y){ return sf::Vector2f(cx - size.x * 0.5f, y); };

    setupButton(buttons[0], "Map",    pos(180.f));
    setupButton(buttons[1], "Scores", pos(270.f));
}