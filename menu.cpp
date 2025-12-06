#include "menu.hpp"
#include <iostream>


Menu::Menu(sf::RenderWindow& window)
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

    setupButton(buttons[0], "Start",    pos(160.f));
    setupButton(buttons[1], "Level",    pos(260.f));
    setupButton(buttons[2], "Settings", pos(360.f));

    const sf::Vector2f quitSize(160.f, 48.f);
    sf::Vector2f quitPos(
        window.getSize().x - quitSize.x - 30.f, // 30 px from right
        window.getSize().y - quitSize.y - 30.f  // 30 px from bottom
    );

    setupButton(buttons[3], "Quit", quitPos);
    buttons[3].box.setSize(quitSize);  
    if (hasFont) centerLabel(buttons[3]);
}

void Menu::setupButton(Button& b, const std::string& text, sf::Vector2f p) {
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

void Menu::centerLabel(Button& b) {
    // Center text inside the button rect
    auto tb = b.label.getLocalBounds();
    b.label.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    b.label.setPosition(b.box.getPosition() + b.box.getSize() * 0.5f);
}

void Menu::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseMoved)
        checkHover();

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        if (buttons[0].contains(window, mp)) action = MenuAction::Start;
        else if (buttons[1].contains(window, mp)) action = MenuAction::Level;
        else if (buttons[2].contains(window, mp)) action = MenuAction::Settings;
        else if (buttons[3].contains(window, mp)) confirmQuit = true;
    }

    if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::Q) {
                window.close();                //quit app
            }
    }

    if (confirmQuit) {
        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::Y) {
                window.close();                // Yes → quit app
            } else if (e.key.code == sf::Keyboard::N || e.key.code == sf::Keyboard::Escape) {
                confirmQuit = false;           // No/Esc → close popup, back to menu
            }
        }
        return; // while popup is open, ignore other events
    }

}

void Menu::checkHover() {
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

void Menu::resetColors() {
    for (auto& b : buttons)
        b.box.setFillColor(idle);
}

void Menu::draw() {
    window.clear(bgColor);
    for (auto& b : buttons) {
        window.draw(b.box);
        if (hasFont) window.draw(b.label);
    }
    if (confirmQuit) {
        sf::RectangleShape overlay(sf::Vector2f(window.getSize()));
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);

        if (hasFont) {
            sf::Text t("Are you sure you want to quit?\nPress Y = Yes, N = No", font, 28);
            t.setFillColor(sf::Color::White);
            auto b = t.getLocalBounds();
            t.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
            t.setPosition(window.getSize().x/2.f, window.getSize().y/2.f);
            window.draw(t);
        }
    }

    window.display();
}