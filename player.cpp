#include "player.hpp"
#include <iostream>

PlayerAppearance gAppearance;  // actual global instance

PlayerSettings::PlayerSettings(sf::RenderWindow& window)
    : window(window)
{
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) {
        std::cerr << "[WARN] Font not found for PlayerSettings.\n";
    }

    // --- Palette init ---
    initPalette();
    applyAppearanceFromGlobal();

    // --- One simple "Back" button at bottom ---
    const sf::Vector2f size(200.f, 50.f);
    const float cx = window.getSize().x * 0.5f;
    sf::Vector2f pos(cx - size.x * 0.5f, window.getSize().y - 100.f);
    setupButton(buttons[0], "Back", pos);

    // --- Color boxes ---
    playerColorBox.setSize({40.f, 40.f});
    aiColorBox.setSize({40.f, 40.f});

    float midX = window.getSize().x * 0.5f;
    playerColorBox.setPosition(midX - 80.f, 220.f);
    aiColorBox.setPosition(midX + 40.f, 220.f);

    playerColorBox.setFillColor(palette[playerColorIndex]);
    aiColorBox.setFillColor(palette[aiColorIndex]);
}

void PlayerSettings::initPalette() {
    palette.clear();
    palette.push_back(sf::Color::Cyan);
    palette.push_back(sf::Color::Magenta);
    palette.push_back(sf::Color::Yellow);
    palette.push_back(sf::Color(0, 200, 0));   // green
    palette.push_back(sf::Color(200, 100, 0)); // orange-ish
    palette.push_back(sf::Color::White);
}

void PlayerSettings::applyAppearanceFromGlobal() {
    playerColorIndex = 0;
    aiColorIndex     = 1;

    for (std::size_t i = 0; i < palette.size(); ++i) {
        if (palette[i] == gAppearance.playerColor) playerColorIndex = static_cast<int>(i);
        if (palette[i] == gAppearance.aiColor)     aiColorIndex     = static_cast<int>(i);
    }
}

void PlayerSettings::updateGlobalAppearance() {
    gAppearance.playerColor = palette[playerColorIndex];
    gAppearance.aiColor     = palette[aiColorIndex];
}

void PlayerSettings::setupButton(Button& b, const std::string& text, sf::Vector2f p) {
    b.box.setSize({200.f, 50.f});
    b.box.setPosition(p);
    b.box.setFillColor(idle);

    if (hasFont) {
        b.label.setFont(font);
        b.label.setString(text);
        b.label.setCharacterSize(24);
        b.label.setFillColor(textColor);
        centerLabel(b);
    }
}

void PlayerSettings::centerLabel(Button& b) {
    auto tb = b.label.getLocalBounds();
    b.label.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    b.label.setPosition(b.box.getPosition() + b.box.getSize() * 0.5f);
}

void PlayerSettings::resetColors() {
    for (auto& b : buttons)
        b.box.setFillColor(idle);
}

void PlayerSettings::checkHover() {
    sf::Vector2i mp = sf::Mouse::getPosition(window);
    resetColors();
    for (auto& b : buttons) {
        if (b.contains(window, mp)) {
            b.box.setFillColor(hover);
            if (hasFont) centerLabel(b);
        }
    }
}

void PlayerSettings::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseMoved) checkHover();

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        sf::Vector2f mpf(static_cast<float>(mp.x), static_cast<float>(mp.y));

        // Buttons
        if (buttons[0].contains(window, mp)) {
            action = PlayerSetAction::Back;
        }

        // Color boxes
        if (playerColorBox.getGlobalBounds().contains(mpf)) {
            playerColorIndex = (playerColorIndex + 1) % static_cast<int>(palette.size());
            playerColorBox.setFillColor(palette[playerColorIndex]);
            updateGlobalAppearance();
        } else if (aiColorBox.getGlobalBounds().contains(mpf)) {
            aiColorIndex = (aiColorIndex + 1) % static_cast<int>(palette.size());
            aiColorBox.setFillColor(palette[aiColorIndex]);
            updateGlobalAppearance();
        }
    }

    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Q) {
        window.close(); //quit app
    }
}

void PlayerSettings::applySettings() {
    // For now everything already goes straight to gAppearance in updateGlobalAppearance().
    // You could keep this empty or use it later if you need "Apply"/"Cancel" logic.
}

void PlayerSettings::draw() {
    window.clear(bgColor);

    if (hasFont) {
        sf::Text title("Player & AI Appearance", font, 26);
        auto tb = title.getLocalBounds();
        title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        title.setPosition(window.getSize().x * 0.5f, 120.f);
        window.draw(title);

        sf::Text playerLabel("Player", font, 20);
        sf::Text aiLabel("AI", font, 20);

        playerLabel.setFillColor(sf::Color::White);
        aiLabel.setFillColor(sf::Color::White);

        playerLabel.setPosition(playerColorBox.getPosition().x, playerColorBox.getPosition().y - 28.f);
        aiLabel.setPosition(aiColorBox.getPosition().x, aiColorBox.getPosition().y - 28.f);

        window.draw(playerLabel);
        window.draw(aiLabel);
    }

    // Buttons
    for (auto& b : buttons) {
        window.draw(b.box);
        if (hasFont) window.draw(b.label);
    }

    // Color boxes
    window.draw(playerColorBox);
    window.draw(aiColorBox);

    window.display();
}
