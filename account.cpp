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
    // Add the "Create New Player" button at bottom
    setupButton(buttons[playerNames.size()], "Create New Player", {100.f, 450.f});
    setupButton(buttons.back(), "Back", {100.f, 500.f});

    inputText.setFont(font);
    inputText.setCharacterSize(32);
    inputText.setFillColor(sf::Color::White);
    inputText.setPosition(300.f, 200.f);
    inputText.setString("");
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

void Account::handleEvent(const sf::Event& e) {

    // ------------------
    // Text Entry Mode
    // ------------------
    if (enteringName) {

        if (e.type == sf::Event::TextEntered) {

            if (e.text.unicode == '\b') {
                if (!newNameInput.empty()) newNameInput.pop_back();
            }
            else if (e.text.unicode == '\r') {
                // ENTER pressed → create player
                PlayerSave::createNewPlayer(newNameInput);
                playerNames = PlayerSave::loadPlayerList(); // refresh buttons
                enteringName = false;
                newNameInput.clear();
                inputText.setString("");
                return;
            }
            else if (e.text.unicode < 128 && std::isalnum(e.text.unicode)) {
                newNameInput += static_cast<char>(e.text.unicode);
            }
            inputText.setString("Name: " + newNameInput);
        }
        return; // Ignore button clicks while typing
    }


    // ------------------
    // Button Click Mode
    // ------------------
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        
        sf::Vector2i mp = sf::Mouse::getPosition(window);

        // Loop through buttons
        for (size_t i = 0; i < buttons.size(); i++) {

            if (buttons[i].contains(window, mp)) {

                // -------------------
                // STEP 6 — Create Player button
                // -------------------
                if (buttons[i].label.getString() == "Create New Player") {
                    enteringName = true;
                    newNameInput.clear();
                    inputText.setString("Name: ");
                    return;
                }

                // SELECT AN EXISTING PLAYER
                if (i < playerNames.size()) {
                    std::cout << playerNames[i] << " is now playing\n";
                }

                // BACK BUTTON
                if (buttons[i].label.getString() == "Back") {
                    action = AccountAction::None;
                }
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

    if (enteringName) {
    sf::RectangleShape box;
    box.setSize({400.f, 80.f});
    box.setFillColor(sf::Color(20, 20, 20));
    box.setOutlineColor(sf::Color::White);
    box.setOutlineThickness(2.f);
    box.setPosition(250.f, 180.f);

    window.draw(box);
    window.draw(inputText);
}

    window.display();
}

