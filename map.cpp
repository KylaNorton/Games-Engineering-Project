#include "map.hpp"
#include <iostream>

Map::Map(sf::RenderWindow& window): window(window) {
    // Load font
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) {
        std::cerr << "[WARN] Font not found at res/fonts/Inter-Regular.ttf. "
                     "Buttons will show without text.\n";
    }

    // Setup 4 level buttons in a 2x2 grid
    setupButton(buttons[0], "Level 1", {200.f, 150.f});
    setupButton(buttons[1], "Level 2", {520.f, 150.f});
    setupButton(buttons[2], "Level 3", {200.f, 320.f});
    setupButton(buttons[3], "Level 4", {520.f, 320.f});
}

void Map::setupButton(Button& b, const std::string& text, sf::Vector2f pos)
{
    b.box.setSize(sf::Vector2f(250.f, 100.f));
    b.box.setPosition(pos);
    b.box.setFillColor(idle);
    b.box.setOutlineThickness(2.f);
    b.box.setOutlineColor(sf::Color::White);

    if (hasFont) {
        b.label.setFont(font);
        b.label.setString(text);
        b.label.setCharacterSize(28);
        b.label.setFillColor(textColor);
        centerLabel(b);
    }
}

void Map::centerLabel(Button& b) {
    sf::FloatRect t = b.label.getLocalBounds();
    b.label.setOrigin(t.left + t.width / 2.f, t.top + t.height / 2.f);

    sf::FloatRect r = b.box.getGlobalBounds();
    b.label.setPosition(r.left + r.width / 2.f, r.top + r.height / 2.f);
}

void Map::resetColors() {
    for (auto& b : buttons) {
        b.box.setFillColor(idle);
    }
}

void Map::checkHover() {
    sf::Vector2i mp = sf::Mouse::getPosition(window);
    resetColors();
    for (auto& b : buttons) {
        if (b.contains(window, mp))
            b.box.setFillColor(hover);
    }
}

void Map::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseMoved) {
        checkHover();
    }

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        
        for (size_t i = 0; i < buttons.size(); ++i) {
            if (buttons[i].contains(window, mp)) {
                std::cout << "Selected: " << buttons[i].label.getString().toAnsiString() << "\n";
                
                // Set action based on which level was clicked
                switch (i) {
                    case 0: action = MapAction::Level1; break;
                    case 1: action = MapAction::Level2; break;
                    case 2: action = MapAction::Level3; break;
                    case 3: action = MapAction::Level4; break;
                    default: action = MapAction::None; break;
                }
            }
        }
    }
}

void Map::draw() {
    window.clear(bgColor);

    // Draw title
    if (hasFont) {
        sf::Text title("Select Level", font, 40);
        title.setFillColor(textColor);
        title.setPosition(350.f, 30.f);
        window.draw(title);
    }

    // Draw all buttons
    for (auto& b : buttons) {
        window.draw(b.box);
        if (hasFont)
            window.draw(b.label);
    }

    window.display();
}
