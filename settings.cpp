#include "settings.hpp"
#include <iostream>

SettingsPage::SettingsPage(sf::RenderWindow& win) : window(win) {
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) std::cerr << "[WARN] Settings font not found.\n";

    toggleBox.setSize({80.f, 36.f});
    toggleBox.setPosition({window.getSize().x*0.5f - 40.f, 240.f});
    toggleBox.setFillColor(sf::Color(90, 90, 140));

    knob.setSize({34.f, 34.f});
    knob.setPosition(toggleBox.getPosition()+sf::Vector2f(1.f,1.f));
    knob.setFillColor(sf::Color::White);

    if (hasFont) {
        label.setFont(font);
        label.setString("Show FPS");
        label.setCharacterSize(24);
        label.setFillColor(sf::Color(230,230,230));
        auto b = label.getLocalBounds();
        label.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
        label.setPosition(window.getSize().x*0.5f, 190.f);
    }
}

void SettingsPage::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        auto mp = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        if (toggleBox.getGlobalBounds().contains(mp)) {
            fpsOn = !fpsOn;
            float x = toggleBox.getPosition().x + (fpsOn ? 45.f : 1.f);
            knob.setPosition({x, knob.getPosition().y});
            toggleBox.setFillColor(fpsOn ? sf::Color(120,180,120) : sf::Color(90,90,140));
        }
    }
}

void SettingsPage::draw() {
    window.clear(sf::Color(16, 24, 34));
    if (hasFont) window.draw(label);
    window.draw(toggleBox);
    window.draw(knob);
    window.display();
}
