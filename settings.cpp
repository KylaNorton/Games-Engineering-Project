#include "settings.hpp"
#include <iostream>


Settings::Settings(sf::RenderWindow& window) : window(window) {
    // load a font
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) {
        std::cerr << "[WARN] Font not found at res/fonts/Inter-Regular.ttf. Buttons will show without text.\n";
    }

    const sf::Vector2f size(320.f, 64.f);
    const float cx = window.getSize().x * 0.5f;
    auto pos = [&](float y){ return sf::Vector2f(cx - size.x * 0.5f, y); };

    setupButton(buttons[0], "Account", pos(160.f));
    setupButton(buttons[1], "Customise Player", pos(260.f));
    setupButton(buttons[2], "Game Tutorial", pos(360.f));
}

void Settings::setupButton(Button& b, const std::string& text, sf::Vector2f p) {
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

void Settings::centerLabel(Button& b) {
    // Center text inside the button rect
    auto tb = b.label.getLocalBounds();
    b.label.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    b.label.setPosition(b.box.getPosition() + b.box.getSize() * 0.5f);
}

void Settings::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseMoved)
        checkHover();

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        if (buttons[0].contains(window, mp)) action = SettingAction::Account;
        else if (buttons[1].contains(window, mp)) action = SettingAction::Customise;
        else if (buttons[2].contains(window, mp)) {
            action = SettingAction::Tuto;
            Tuto = true; // show tutorial popup
        }
    }

    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Q) {
        window.close(); //quit app
    }

    if (Tuto) {
        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::C) {
                Tuto = false; // close popup, back to game
            }
        }
        return; // while popup is open, ignore other events
    }
}

void Settings::checkHover() {
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

void Settings::resetColors() {
    for (auto& b : buttons)
        b.box.setFillColor(idle);
}

void Settings::draw() {
    window.clear(bgColor);
    for (auto& b : buttons) {
        window.draw(b.box);
        if (hasFont) window.draw(b.label);
    }

    // Tutorial popup
    if (Tuto && hasFont) {
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
        msg += "\nGood luck and have fun!";    

        text.setString(msg);
       
        sf::FloatRect tb = text.getLocalBounds();
        text.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        text.setPosition(window.getSize().x / 2.f, window.getSize().y / 2.f);
        window.draw(text);
    }

    window.display();
}

void Settings::recomputeLayout() {
    const sf::Vector2f size(320.f, 64.f);
    const float cx = window.getSize().x * 0.5f;
    auto pos = [&](float y){ return sf::Vector2f(cx - size.x * 0.5f, y); };

    setupButton(buttons[0], "Account", pos(160.f));
    setupButton(buttons[1], "Customise Player", pos(260.f));
    setupButton(buttons[2], "Game Tutorial", pos(360.f));
}