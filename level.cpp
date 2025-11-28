#include "level.hpp"
#include <iostream>

static void centerText(sf::Text& t, const sf::RectangleShape& box) {
    auto b = t.getLocalBounds();
    t.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
    t.setPosition(box.getPosition() + box.getSize()*0.5f);
}

static void setup(sf::Font& f, bool hasFont, Level::Btn& b,
                  const char* label, sf::Vector2f pos, sf::Vector2f size,
                  const sf::Color& idle, const sf::Color& text) {
    b.box.setSize(size);
    b.box.setPosition(pos);
    b.box.setFillColor(idle);
    if (hasFont) {
        b.label.setFont(f);
        b.label.setString(label);
        b.label.setCharacterSize(28);
        b.label.setFillColor(text);
        centerText(b.label, b.box);
    }
}

Level::Level(sf::RenderWindow& win) : window(win) {
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) std::cerr << "[WARN] LevelPage font not found.\n";

    const sf::Vector2f size(260.f, 64.f);
    float cx = window.getSize().x * 0.5f;
    auto pos = [&](float y){ return sf::Vector2f(cx - size.x*0.5f, y); };

    setup(font, hasFont, easy,   "Easy",   pos(160.f), size, idle, text);
    setup(font, hasFont, medium, "Medium", pos(250.f), size, idle, text);
    setup(font, hasFont, hard,   "Hard",   pos(340.f), size, idle, text);
}

void Level::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseMoved) {
        auto mp = sf::Mouse::getPosition(window);
        for (auto* b : {&easy,&medium,&hard}) {
            b->box.setFillColor(b->box.getGlobalBounds()
                .contains(window.mapPixelToCoords(mp)) ? hover : idle);
        }
    }
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        auto mp = sf::Mouse::getPosition(window);
        if (easy.box.getGlobalBounds().contains(window.mapPixelToCoords(mp)))   choice = Difficulty::Easy;
        else if (medium.box.getGlobalBounds().contains(window.mapPixelToCoords(mp))) choice = Difficulty::Medium;
        else if (hard.box.getGlobalBounds().contains(window.mapPixelToCoords(mp)))   choice = Difficulty::Hard;
    }
}

void Level::draw() {
    window.clear(sf::Color(18,18,28));
    for (auto* b : {&easy,&medium,&hard}) {
        window.draw(b->box);
        if (hasFont) window.draw(b->label);
    }
    window.display();
}
