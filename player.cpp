#include "player.hpp"
#include "spriteLib.hpp"
#include <iostream>

PlayerAppearance gAppearance;  // actual global instance

PlayerSettings::PlayerSettings(sf::RenderWindow& window)
    : window(window)
{
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) {
        std::cerr << "[WARN] Font not found for PlayerSettings.\n";
    }

    // --- Sprite buttons ---
    createSkinButtons();
    updateSelectionHighlight();

    // --- Palette init ---
    initPalette();
    applyAppearanceFromGlobal();

    // --- Color swatches + preview sprites ---
    createColorSwatches();
    updateColorSwatchHighlights();
    setupModeButtons();



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

    updateModeButtonHighlights();
    updatePreviews();
}


void PlayerSettings::setupModeButtons() {
    float midX = window.getSize().x * 0.5f;

    auto setupToggle = [&](sf::RectangleShape& box, sf::Text& txt,
                           const std::string& label, sf::Vector2f pos)
    {
        box.setSize({80.f, 32.f});
        box.setPosition(pos);
        box.setFillColor(sf::Color(40, 40, 80));
        box.setOutlineThickness(2.f);
        box.setOutlineColor(sf::Color::White);

        if (hasFont) {
            txt.setFont(font);
            txt.setString(label);
            txt.setCharacterSize(16);
            txt.setFillColor(sf::Color::White);
            auto tb = txt.getLocalBounds();
            txt.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
            txt.setPosition(
                box.getPosition().x + box.getSize().x / 2.f,
                box.getPosition().y + box.getSize().y / 2.f
            );
        }
    };

    // Player toggles (above sprite/color rows)
    setupToggle(playerModeCircleBox, playerModeCircleText, "Circle",
                { midX - 160.f, 170.f });
    setupToggle(playerModeSpriteBox, playerModeSpriteText, "Sprite",
                { midX - 60.f,  170.f });

    // AI toggles
    setupToggle(aiModeCircleBox, aiModeCircleText, "Circle",
                { midX + 60.f,  170.f });
    setupToggle(aiModeSpriteBox, aiModeSpriteText, "Sprite",
                { midX + 160.f, 170.f });

    // Circle previews
    playerPreviewCircle.setRadius(18.f);
    playerPreviewCircle.setOrigin(18.f, 18.f);
    playerPreviewCircle.setFillColor(palette[playerColorIndex]);
    playerPreviewCircle.setPosition(midX - 120.f, 220.f);

    aiPreviewCircle.setRadius(18.f);
    aiPreviewCircle.setOrigin(18.f, 18.f);
    aiPreviewCircle.setFillColor(palette[aiColorIndex]);
    aiPreviewCircle.setPosition(midX + 120.f, 220.f);
}

void PlayerSettings::updateModeButtonHighlights() {
    auto setActive = [](sf::RectangleShape& box, bool active){
        box.setOutlineColor(active ? sf::Color::Yellow : sf::Color::White);
    };

    setActive(playerModeCircleBox, !gAppearance.playerUseSprite);
    setActive(playerModeSpriteBox, gAppearance.playerUseSprite);
    setActive(aiModeCircleBox, !gAppearance.aiUseSprite);
    setActive(aiModeSpriteBox, gAppearance.aiUseSprite);
}


void PlayerSettings::initPalette() {
    palette.clear();
    palette.push_back(sf::Color::Cyan);
    palette.push_back(sf::Color::Magenta);
    palette.push_back(sf::Color::Yellow);
    palette.push_back(sf::Color(85, 107, 47)); // green olive
    palette.push_back(sf::Color(200, 100, 0)); // orange
    palette.push_back(sf::Color(150, 75, 0)); // brown
    palette.push_back(sf::Color(128, 0, 128)); // purple
    palette.push_back(sf::Color(255, 192, 203)); // pink
    palette.push_back(sf::Color(165, 42, 42)); // brownish red
    palette.push_back(sf::Color(95, 158, 160)); // cadet blue
    palette.push_back(sf::Color(218, 165, 32)); // goldenrod
    palette.push_back(sf::Color(0, 0, 128)); // navy blue
    palette.push_back(sf::Color::White);
}

void PlayerSettings::applyAppearanceFromGlobal() {
    playerColorIndex = 0;
    aiColorIndex = 1;

    for (std::size_t i = 0; i < palette.size(); ++i) {
        if (palette[i] == gAppearance.playerColor) playerColorIndex = static_cast<int>(i);
        if (palette[i] == gAppearance.aiColor) aiColorIndex = static_cast<int>(i);
    }
}

void PlayerSettings::updateGlobalAppearance() {
    gAppearance.playerColor = palette[playerColorIndex];
    gAppearance.aiColor = palette[aiColorIndex];
}

void PlayerSettings::setupButton(Button& b, const std::string& text, sf::Vector2f p) {
    b.circle.setSize({200.f, 50.f});
    b.circle.setPosition(p);
    b.circle.setFillColor(idle);

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
    b.label.setPosition(b.circle.getPosition() + b.circle.getSize() * 0.5f);
}

void PlayerSettings::resetColors() {
    for (auto& b : buttons)
        b.circle.setFillColor(idle);
}

void PlayerSettings::checkHover() {
    sf::Vector2i mp = sf::Mouse::getPosition(window);
    resetColors();
    for (auto& b : buttons) {
        if (b.contains(window, mp)) {
            b.circle.setFillColor(hover);
            if (hasFont) centerLabel(b);
        }
    }
}

void PlayerSettings::createColorSwatches() {
    playerColorSwatches.clear();
    aiColorSwatches.clear();

    float swatchSize = 22.f;
    float spacing = 6.f;

    // Player row under "Player" label
    float startXPlayer = playerColorBox.getPosition().x - 20.f;
    float yPlayer = playerColorBox.getPosition().y + playerColorBox.getSize().y + 10.f;

    // AI row under "AI" label
    float startXAI = aiColorBox.getPosition().x - 20.f;
    float yAI = aiColorBox.getPosition().y + aiColorBox.getSize().y + 10.f;

    for (std::size_t i = 0; i < palette.size(); ++i) {
        sf::RectangleShape pSwatch;
        pSwatch.setSize({swatchSize, swatchSize});
        pSwatch.setFillColor(palette[i]);
        pSwatch.setOutlineThickness(0.f);
        pSwatch.setOutlineColor(sf::Color::White);
        pSwatch.setPosition(startXPlayer + i * (swatchSize + spacing), yPlayer);
        playerColorSwatches.push_back(pSwatch);

        sf::RectangleShape aSwatch = pSwatch;
        aSwatch.setPosition(startXAI + i * (swatchSize + spacing), yAI);
        aiColorSwatches.push_back(aSwatch);
    }
}

void PlayerSettings::updateColorSwatchHighlights() {
    for (std::size_t i = 0; i < playerColorSwatches.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == playerColorIndex);
        playerColorSwatches[i].setOutlineThickness(isSelected ? 3.f : 0.f);
    }
    for (std::size_t i = 0; i < aiColorSwatches.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == aiColorIndex);
        aiColorSwatches[i].setOutlineThickness(isSelected ? 3.f : 0.f);
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

        // Color swatches: player
        for (std::size_t i = 0; i < playerColorSwatches.size(); ++i) {
            if (playerColorSwatches[i].getGlobalBounds().contains(mpf)) {
                playerColorIndex = static_cast<int>(i);
                playerColorBox.setFillColor(palette[playerColorIndex]);
                updateGlobalAppearance();
                updateColorSwatchHighlights();
                updatePreviews();
                break;
            }
        }

        // Color swatches: AI
        for (std::size_t i = 0; i < aiColorSwatches.size(); ++i) {
            if (aiColorSwatches[i].getGlobalBounds().contains(mpf)) {
                aiColorIndex = static_cast<int>(i);
                aiColorBox.setFillColor(palette[aiColorIndex]);
                updateGlobalAppearance();
                updateColorSwatchHighlights();
                updatePreviews();
                break;
            }
        }

        // 0) Mode toggles (circle / sprite)
        if (playerModeCircleBox.getGlobalBounds().contains(mpf)) {
            gAppearance.playerUseSprite = false;
            updateModeButtonHighlights();
            updatePreviews();
        } else if (playerModeSpriteBox.getGlobalBounds().contains(mpf)) {
            gAppearance.playerUseSprite = true;
            updateModeButtonHighlights();
            updatePreviews();
        }

        if (aiModeCircleBox.getGlobalBounds().contains(mpf)) {
            gAppearance.aiUseSprite = false;
            updateModeButtonHighlights();
            updatePreviews();
        } else if (aiModeSpriteBox.getGlobalBounds().contains(mpf)) {
            gAppearance.aiUseSprite = true;
            updateModeButtonHighlights();
            updatePreviews();
        }
    }

    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Q) {
        window.close(); //quit app
    }

     if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mousePos =
            window.mapPixelToCoords({e.mouseButton.x, e.mouseButton.y});

        for (auto& btn : skinButtons) {
            if (btn.box.getGlobalBounds().contains(mousePos)) {
                if (btn.isForPlayer) {
                    gAppearance.playerTextureIndex = btn.textureIndex;
                } else {
                    gAppearance.aiTextureIndex = btn.textureIndex;
                }
                updateSelectionHighlight();
                break;
            }
            if (btn.isForPlayer) {
                gAppearance.playerTextureIndex = btn.textureIndex;
            } else {
                gAppearance.aiTextureIndex = btn.textureIndex;
            }
            updateSelectionHighlight();
            updatePreviews();
            break;
        }
    }
}

void PlayerSettings::updatePreviews() {
    auto& lib = PlayerSpriteLibrary::instance();
    int texCount = lib.getCount();

    // Clamp texture indices if textures exist
    if (texCount > 0) {
        if (gAppearance.playerTextureIndex < 0 ||
            gAppearance.playerTextureIndex >= texCount)
            gAppearance.playerTextureIndex = 0;

        if (gAppearance.aiTextureIndex < 0 ||
            gAppearance.aiTextureIndex >= texCount)
            gAppearance.aiTextureIndex = std::min(1, texCount - 1);
    }

    float midX     = window.getSize().x * 0.5f;
    float previewY = 230.f;

    // --- PLAYER ---
    if (gAppearance.playerUseSprite && texCount > 0) {
        const sf::Texture& pTex = lib.getTexture(gAppearance.playerTextureIndex);
        playerPreviewSprite.setTexture(pTex);

        sf::Vector2u pSize = pTex.getSize();
        int cols   = 4;
        int rows   = 4;
        int frameW = pSize.x / cols;
        int frameH = pSize.y / rows;

        playerPreviewSprite.setTextureRect(sf::IntRect(0, 0, frameW, frameH));
        playerPreviewSprite.setOrigin(frameW / 2.f, frameH / 2.f);
        playerPreviewSprite.setScale(3.f, 3.f);
        playerPreviewSprite.setColor(sf::Color::White); 
        playerPreviewSprite.setPosition(midX - 120.f, previewY);
    }

    // Circle preview color
    playerPreviewCircle.setFillColor(palette[playerColorIndex]);
    playerPreviewCircle.setPosition(midX - 120.f, previewY);

    // --- AI ---
    if (gAppearance.aiUseSprite && texCount > 0) {
        const sf::Texture& aTex = lib.getTexture(gAppearance.aiTextureIndex);
        aiPreviewSprite.setTexture(aTex);

        sf::Vector2u aSize = aTex.getSize();
        int cols   = 4;
        int rows   = 4;
        int frameW = aSize.x / cols;
        int frameH = aSize.y / rows;

        aiPreviewSprite.setTextureRect(sf::IntRect(0, 0, frameW, frameH));
        aiPreviewSprite.setOrigin(frameW / 2.f, frameH / 2.f);
        aiPreviewSprite.setScale(3.f, 3.f);
        aiPreviewSprite.setColor(sf::Color::White);
        aiPreviewSprite.setPosition(midX + 120.f, previewY);
    }

    aiPreviewCircle.setFillColor(palette[aiColorIndex]);
    aiPreviewCircle.setPosition(midX + 120.f, previewY);
}

void PlayerSettings::updateSelectionHighlight() {
    for (auto& btn : skinButtons) {
        bool selected = (btn.isForPlayer &&
                         btn.textureIndex == gAppearance.playerTextureIndex)
                     || (!btn.isForPlayer &&
                         btn.textureIndex == gAppearance.aiTextureIndex);

        if (selected) {
            btn.box.setOutlineColor(sf::Color::Yellow);
            btn.box.setOutlineThickness(4.f);
        } else {
            btn.box.setOutlineColor(sf::Color::White);
            btn.box.setOutlineThickness(2.f);
        }
    }
}

void PlayerSettings::applySettings() {
    updateGlobalAppearance();
}

void PlayerSettings::createSkinButtons() {
    auto& lib = PlayerSpriteLibrary::instance();
    int count = lib.getCount();

    skinButtons.clear();
    skinButtons.reserve(count * 2); // player row + AI row

    float startX = 200.f;
    float startYPlayer = 200.f;
    float startYAI     = 350.f;
    float spacing = 20.f;
    float boxSize = 64.f;

    for (int i = 0; i < count; ++i) {
        // --- PLAYER row ---
        SkinButton playerBtn;
        playerBtn.textureIndex = i;
        playerBtn.isForPlayer  = true;

        playerBtn.box.setSize({boxSize, boxSize});
        playerBtn.box.setFillColor(sf::Color(50, 50, 50));
        playerBtn.box.setOutlineThickness(2.f);
        playerBtn.box.setOutlineColor(sf::Color::White);
        playerBtn.box.setPosition(startX + i * (boxSize + spacing), startYPlayer);

        // Use only one frame from the sheet
        const sf::Texture& tex = lib.getTexture(i);
        playerBtn.icon.setTexture(tex);

        sf::Vector2u texSize = tex.getSize();
        int cols   = 4;                 // 4 frames per row
        int rows   = 4;                 // 4 rows (down, right, up, left)
        int frameW = texSize.x / cols;
        int frameH = texSize.y / rows;

        // Take frame (0,0): facing down, first frame
        playerBtn.icon.setTextureRect(sf::IntRect(0, 0, frameW, frameH));
        playerBtn.icon.setOrigin(frameW / 2.f, frameH / 2.f);

        float scale = std::min(
            boxSize / static_cast<float>(frameW),
            boxSize / static_cast<float>(frameH)
        ) * 0.9f;
        playerBtn.icon.setScale(scale, scale);
        playerBtn.icon.setPosition(
            playerBtn.box.getPosition().x + boxSize / 2.f,
            playerBtn.box.getPosition().y + boxSize / 2.f
        );

        skinButtons.push_back(playerBtn);

        // --- AI row ---
        SkinButton aiBtn = playerBtn;
        aiBtn.isForPlayer = false;
        aiBtn.box.setPosition(startX + i * (boxSize + spacing), startYAI);
        aiBtn.icon.setPosition(
            aiBtn.box.getPosition().x + boxSize / 2.f,
            aiBtn.box.getPosition().y + boxSize / 2.f
        );

        skinButtons.push_back(aiBtn);
    }
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

    // Big previews: draw circle or sprite depending on mode
    if (gAppearance.playerUseSprite) {
        window.draw(playerPreviewSprite);
    } else {
        window.draw(playerPreviewCircle);
    }

    if (gAppearance.aiUseSprite) {
        window.draw(aiPreviewSprite);
    } else {
        window.draw(aiPreviewCircle);
    }

    // Buttons
    for (const auto& btn : skinButtons) {
        window.draw(btn.box);
        window.draw(btn.icon);
    }
    for (auto& b : buttons) {
        window.draw(b.circle);
        if (hasFont) window.draw(b.label);
    }

    // Color boxes
    window.draw(playerColorBox);
    window.draw(aiColorBox);

    // Color boxes
    window.draw(playerColorBox);
    window.draw(aiColorBox);

    // Color swatches
    for (const auto& r : playerColorSwatches)
        window.draw(r);
    for (const auto& r : aiColorSwatches)
        window.draw(r);

    // Mode toggles
    window.draw(playerModeCircleBox);
    window.draw(playerModeSpriteBox);
    window.draw(aiModeCircleBox);
    window.draw(aiModeSpriteBox);
    if (hasFont) {
        window.draw(playerModeCircleText);
        window.draw(playerModeSpriteText);
        window.draw(aiModeCircleText);
        window.draw(aiModeSpriteText);
    }

    // Color boxes & swatches
    window.draw(playerColorBox);
    window.draw(aiColorBox);
    for (const auto& r : playerColorSwatches) window.draw(r);
    for (const auto& r : aiColorSwatches) window.draw(r);

    window.display();
}
