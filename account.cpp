#include "account.hpp"
#include "PlayerSave.hpp"
#include <iostream>

// Global variable defined somewhere (main.cpp)
extern std::string CURRENT_PLAYER;

Account::Account(sf::RenderWindow& window) : window(window)
{
    // load a font
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) {
        std::cerr << "[WARN] Font not found at res/fonts/Inter-Regular.ttf. "
                     "Buttons will show without text.\n";
    }
    // Load all saved player names
    std::vector<std::string> players = PlayerSave::loadPlayerList();

    // Prepare buttons vector  
    buttons.resize(players.size());

    float startY = 150.f;
    float gap = 80.f;

    for (size_t i = 0; i < players.size(); i++) {
        sf::Vector2f pos(200.f, startY + i * gap);

        setupButton(buttons[i], players[i], pos);
    }
}


void Account::setupButton(Button& b, const std::string& text, sf::Vector2f pos)
{
    b.box.setSize(sf::Vector2f(350.f, 60.f));
    b.box.setPosition(pos);
    b.box.setFillColor(idle);
    b.box.setOutlineThickness(2.f);
    b.box.setOutlineColor(sf::Color::White);

    if (hasFont) {
        b.label.setFont(font);
        b.label.setString(text);
        b.label.setCharacterSize(24);
        b.label.setFillColor(textColor);
        centerLabel(b);
    }
}


void Account::centerLabel(Button& b) {
    sf::FloatRect t = b.label.getLocalBounds();
    b.label.setOrigin(t.left + t.width / 2.f, t.top + t.height / 2.f);

    sf::FloatRect r = b.box.getGlobalBounds();
    b.label.setPosition(r.left + r.width / 2.f, r.top + r.height / 2.f);
}


void Account::resetColors() {
    for (auto& b : buttons) {
        b.box.setFillColor(idle);
    }
}

void Account::checkHover() {
    sf::Vector2i mp = sf::Mouse::getPosition(window);
    for (auto& b : buttons) {
        if (b.contains(window, mp))
            b.box.setFillColor(hover);
    }
}

void Account::handleEvent(const sf::Event& e)
{
    if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2i mp = sf::Mouse::getPosition(window);

        for (size_t i = 0; i < buttons.size(); i++)
        {
            if (buttons[i].contains(window, mp))
            {
                // Print the stored button label text
                if (hasFont)
                    std::cout << buttons[i].label.getString().toAnsiString()
                              << " is now playing" << std::endl;
                else
                    std::cout << "Button " << i << " clicked" << std::endl;
            }
        }
    }
}


void Account::draw()
{
    window.clear(bgColor);

    for (auto& b : buttons) {
        window.draw(b.box);
        if (hasFont)
            window.draw(b.label);
    }

    window.display();
}

