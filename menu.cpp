#include "menu.hpp"
#include <iostream>

Menu::Menu(sf::RenderWindow& window)
    : window(window), action(MenuAction::None),
      bgColor(30, 20, 50),
      idle(126, 92, 210),
      hover(146, 112, 230)
{
    const sf::Vector2f btnSize(320.f, 64.f);
    const float centerX = window.getSize().x * 0.5f;
    auto centerXPos = [&](float y) { return sf::Vector2f(centerX - btnSize.x * 0.5f, y); };

    setupButton(buttons[0], centerXPos(160.f));
    setupButton(buttons[1], centerXPos(260.f));
    setupButton(buttons[2], centerXPos(360.f));
}

void Menu::setupButton(Button& b, sf::Vector2f pos) {
    b.box.setSize({320.f, 64.f});
    b.box.setPosition(pos);
    b.box.setFillColor(idle);
}

void Menu::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseMoved)
        checkHover();

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        if (buttons[0].contains(window, mp)) action = MenuAction::Start;
        else if (buttons[1].contains(window, mp)) action = MenuAction::Level;
        else if (buttons[2].contains(window, mp)) action = MenuAction::Settings;
    }
}

void Menu::checkHover() {
    sf::Vector2i mp = sf::Mouse::getPosition(window);
    resetColors();
    for (auto& b : buttons)
        if (b.contains(window, mp))
            b.box.setFillColor(hover);
}

void Menu::resetColors() {
    for (auto& b : buttons)
        b.box.setFillColor(idle);
}

void Menu::draw() {
    window.clear(bgColor);
    for (auto& b : buttons)
        window.draw(b.box);
    window.display();
}
