#include "player.hpp"
#include "spriteLib.hpp"
#include "playerSave.hpp"
#include <iostream>
#include <algorithm>

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

    // --- previews ---



    // --- One simple "Back" button at bottom ---
    const sf::Vector2f size(200.f, 50.f);
    const float cx = window.getSize().x * 0.5f;
    sf::Vector2f pos(cx - size.x * 0.5f, window.getSize().y - 100.f);
    setupButton(buttons[0], "Back", pos);

    float midX = window.getSize().x * 0.5f;

    // Initialize previews
    updatePreviews();
}


void PlayerSettings::updateGlobalAppearance() {
    // Only player color is relevant for customization; AI uses fixed appearance
    // keep this function to match existing call sites but do nothing extra here
    (void)gAppearance;
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

// color swatches and related functions removed (sprites-only customization)


void PlayerSettings::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseMoved) checkHover();

    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        sf::Vector2f mpf(static_cast<float>(mp.x), static_cast<float>(mp.y));

        // Buttons
        if (buttons[0].contains(window, mp)) {
            action = PlayerSetAction::Back;
        }

        // color swatches removed (sprites-only customization)

        // (circle/sprite toggles removed)
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
                    // Persist immediately for the active player
                    PlayerSave::activePlayer.playerTextureIndex = gAppearance.playerTextureIndex;
                    PlayerSave::activePlayer.saveToFile();
                } else {
                    gAppearance.aiTextureIndex = btn.textureIndex;
                }
                updateSelectionHighlight();
                updatePreviews();
                break;
            }
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
    float previewY = window.getSize().y * 0.22f; // move previews upward so skins are visible below

    // --- PLAYER ---
    if (texCount > 0) {
        const sf::Texture& pTex = lib.getTexture(gAppearance.playerTextureIndex);
        playerPreviewSprite.setTexture(pTex);

        sf::Vector2u pSize = pTex.getSize();
        int cols = 4;
        int rows = 4;
        int frameW = pSize.x / cols;
        int frameH = pSize.y / rows;

        playerPreviewSprite.setTextureRect(sf::IntRect(0, 0, frameW, frameH));
        playerPreviewSprite.setOrigin(frameW / 2.f, frameH / 2.f);
        playerPreviewSprite.setScale(3.f, 3.f);
        playerPreviewSprite.setColor(sf::Color::White);
        playerPreviewSprite.setPosition(midX - 120.f, previewY);
    }

    // --- AI ---
    if (lib.hasAiTexture()) {
        const sf::Texture& aTex = lib.getAiTexture();
        aiPreviewSprite.setTexture(aTex);

        sf::Vector2u aSize = aTex.getSize();
        int cols = 4;
        int rows = 4;
        int frameW = aSize.x / cols;
        int frameH = aSize.y / rows;

        aiPreviewSprite.setTextureRect(sf::IntRect(0, 0, frameW, frameH));
        aiPreviewSprite.setOrigin(frameW / 2.f, frameH / 2.f);
        aiPreviewSprite.setScale(3.f, 3.f);
        aiPreviewSprite.setColor(sf::Color::White);
        aiPreviewSprite.setPosition(midX + 120.f, previewY);
    }
}

void PlayerSettings::recomputeLayout() {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // Back button at bottom center
    const sf::Vector2f size(200.f, 50.f);
    const float cx = winW * 0.5f;
    sf::Vector2f pos(cx - size.x * 0.5f, winH - 100.f);
    setupButton(buttons[0], "Back", pos);

    float midX = winW * 0.5f;

    // Previews: position them higher so the skin row sits below
    float previewY = winH * 0.22f;
    playerPreviewSprite.setPosition(midX - 120.f, previewY);
    aiPreviewSprite.setPosition(midX + 120.f, previewY);

    // Skin buttons: layout as a responsive grid (wrap to multiple rows)
    int playerCount = 0;
    for (auto &b : skinButtons) if (b.isForPlayer) ++playerCount;
    if (playerCount == 0) return;

    float boxSize = skinBoxSize;
    float spacing = skinSpacing;
    float margin = 80.f;

    // Determine how many columns fit comfortably
    int cols = static_cast<int>((winW - 2 * margin + spacing) / (boxSize + spacing));
    if (cols < 1) cols = 1;
    if (cols > playerCount) cols = playerCount;

    int rows = (playerCount + cols - 1) / cols;

    float totalW = cols * boxSize + (cols - 1) * spacing;
    float startX = midX - totalW / 2.f;
    float startYPlayer = winH * 0.58f - (rows - 1) * (boxSize + spacing) / 2.f; // center vertically a bit

    // place player grid
    int pIndex = 0;
    for (size_t k = 0; k < skinButtons.size(); ++k) {
        auto &btn = skinButtons[k];
        if (!btn.isForPlayer) continue;

        int r = pIndex / cols;
        int c = pIndex % cols;
        float x = startX + c * (boxSize + spacing);
        float y = startYPlayer + r * (boxSize + spacing);

        btn.box.setSize({boxSize, boxSize});
        btn.box.setPosition(x, y);
        btn.icon.setPosition(x + boxSize / 2.f, y + boxSize / 2.f);

        // label below the box
        if (hasFont) {
            auto tb = btn.label.getLocalBounds();
            btn.label.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
            btn.label.setPosition(x + boxSize / 2.f, y + boxSize + 12.f);
        }

        pIndex++;
    }

    // update layout helpers for draw
    skinRowY = startYPlayer;
    skinRowTotalWidth = totalW;

    updatePreviews();
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

    // Persist to active player save (if a player exists)
    PlayerSave::activePlayer.playerTextureIndex = gAppearance.playerTextureIndex;
    // Only persist player-specific choices. AI uses a fixed sprite and color.
    PlayerSave::activePlayer.playerColor = gAppearance.playerColor;

    // Try to write to disk
    PlayerSave::activePlayer.saveToFile();
}

void PlayerSettings::createSkinButtons() {
    auto& lib = PlayerSpriteLibrary::instance();
    int count = lib.getCount();

    skinButtons.clear();
    skinButtons.reserve(count);

    float startX = 200.f;
    float startYPlayer = 200.f;
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

        // Optional label under the icon (e.g. "Skin 1")
        if (hasFont) {
            playerBtn.label.setFont(font);
            playerBtn.label.setString(std::string("Skin ") + std::to_string(i + 1));
            playerBtn.label.setCharacterSize(14);
            playerBtn.label.setFillColor(sf::Color::White);
            // origin and position are set in recomputeLayout
        }

        skinButtons.push_back(playerBtn);
        // (AI buttons removed — AI uses a fixed sprite)
    }
}

void PlayerSettings::draw() {
    window.clear(bgColor);

    if (hasFont) {
        sf::Text title("Player Appearance", font, 28);
        auto tb = title.getLocalBounds();
        title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        title.setPosition(window.getSize().x * 0.5f, 80.f); // move title higher
        window.draw(title);

        sf::Text playerLabel("Player", font, 20);
        playerLabel.setFillColor(sf::Color::White);
        // center label above the skin row
        auto plb = playerLabel.getLocalBounds();
        playerLabel.setOrigin(plb.left + plb.width / 2.f, plb.top + plb.height / 2.f);
        playerLabel.setPosition(window.getSize().x * 0.5f, skinRowY - 36.f);
        window.draw(playerLabel);
    }

    // Big previews: sprites only
    window.draw(playerPreviewSprite);
    window.draw(aiPreviewSprite);

    // Buttons
    for (const auto& btn : skinButtons) {
        window.draw(btn.box);
        window.draw(btn.icon);
        if (hasFont) window.draw(btn.label);
    }
    for (auto& b : buttons) {
        window.draw(b.circle);
        if (hasFont) window.draw(b.label);
    }

    // (color customization removed)

    window.display();
}
