#include "player.hpp"
#include "spriteLib.hpp"
#include "playerSave.hpp"
#include <iostream>
#include <algorithm>

PlayerAppearance gAppearance;  // global

PlayerSettings::PlayerSettings(sf::RenderWindow& window)
    : window(window)
{
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) {
        std::cerr << "[WARN] Font not found for PlayerSettings.\n";
    }

    // Create the skin thumbnails (max 10)
    createSkinButtons();
    updateSelectionHighlight();

    // Back button
    const sf::Vector2f size(200.f, 50.f);
    const float cx = window.getSize().x * 0.5f;
    sf::Vector2f pos(cx - size.x * 0.5f, window.getSize().y - 100.f);
    setupButton(buttons[0], "Back", pos);

    recomputeLayout();
    updatePreviews();
}

// Only here for compatibility with the rest of the code
void PlayerSettings::updateGlobalAppearance() {
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

void PlayerSettings::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseMoved) checkHover();

    if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2i mp = sf::Mouse::getPosition(window);

        // Back button
        if (buttons[0].contains(window, mp)) {
            action = PlayerSetAction::Back;
        }

        // Skin thumbnails
        sf::Vector2f mousePos =
            window.mapPixelToCoords({e.mouseButton.x, e.mouseButton.y});

        for (auto& btn : skinButtons) {
            if (btn.box.getGlobalBounds().contains(mousePos)) {
                gAppearance.playerTextureIndex = btn.textureIndex;

                // Persist immediately
                PlayerSave::activePlayer.playerTextureIndex = gAppearance.playerTextureIndex;
                PlayerSave::activePlayer.saveToFile();

                updateSelectionHighlight();
                updatePreviews();
                break;
            }
        }
    }

    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Q) {
        window.close(); // quit app
    }
}

void PlayerSettings::updatePreviews() {
    auto& lib = PlayerSpriteLibrary::instance();
    int texCount = lib.getCount();
    if (texCount <= 0) return;

    // Clamp index
    if (gAppearance.playerTextureIndex < 0 ||
        gAppearance.playerTextureIndex >= texCount)
        gAppearance.playerTextureIndex = 0;

    const sf::Texture& pTex = lib.getTexture(gAppearance.playerTextureIndex);
    playerPreviewSprite.setTexture(pTex);

    sf::Vector2u pSize = pTex.getSize();

    // Use the whole texture (single-frame sprite)
    int frameW = static_cast<int>(pSize.x);
    int frameH = static_cast<int>(pSize.y);

    playerPreviewSprite.setTextureRect(sf::IntRect(0, 0, frameW, frameH));
    playerPreviewSprite.setOrigin(frameW / 2.f, frameH / 2.f);

    // Scale up nicely
    float targetHeight = 120.f; // pixels on screen (tweak if you want bigger)
    float scale = 1.f;
    if (frameH > 0)
        scale = targetHeight / static_cast<float>(frameH);

    playerPreviewSprite.setScale(scale, scale);

    // Position is set in recomputeLayout()
}

void PlayerSettings::recomputeLayout() {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // Back button lower, closer to bottom
    const sf::Vector2f size(200.f, 50.f);
    const float cx = winW * 0.5f;
    sf::Vector2f pos(cx - size.x * 0.5f, winH - 70.f); // was -100.f
    setupButton(buttons[0], "Back", pos);

    float midX = winW * 0.5f;

    // Preview a bit lower in the screen
    float previewY = winH * 0.32f;  // was 0.25f
    playerPreviewSprite.setPosition(midX, previewY);

    // Layout for the skin grid
    int skinCount = static_cast<int>(skinButtons.size());
    if (skinCount == 0) return;

    float boxSize = skinBoxSize;
    float spacing = skinSpacing;
    float margin  = 80.f;

    // At most 5 columns
    int maxColumns = 5;
    int cols = std::min(maxColumns, skinCount);

    // Ensure they fit in the window
    int colsByWidth =
        static_cast<int>((winW - 2 * margin + spacing) / (boxSize + spacing));
    cols = std::min(cols, std::max(1, colsByWidth));

    int rows = (skinCount + cols - 1) / cols;

    float totalW = cols * boxSize + (cols - 1) * spacing;
    float startX = midX - totalW / 2.f;

    // Skin grid globally lower on the screen
    float startY = winH * 0.62f;        // was 0.5f
    startY -= (rows - 1) * (boxSize + spacing) / 2.f;

    // Place buttons
    for (int i = 0; i < skinCount; ++i) {
        auto& btn = skinButtons[i];
        int r = i / cols;
        int c = i % cols;

        float x = startX + c * (boxSize + spacing);
        float y = startY + r * (boxSize + spacing);

        btn.box.setSize({boxSize, boxSize});
        btn.box.setPosition(x, y);

        btn.icon.setPosition(x + boxSize / 2.f, y + boxSize / 2.f);

        if (hasFont) {
            auto tb = btn.label.getLocalBounds();
            btn.label.setOrigin(tb.left + tb.width / 2.f,
                                tb.top  + tb.height / 2.f);
            btn.label.setPosition(x + boxSize / 2.f, y + boxSize + 14.f);
        }
    }

    skinRowY          = startY;
    skinRowTotalWidth = totalW;
}


void PlayerSettings::updateSelectionHighlight() {
    for (auto& btn : skinButtons) {
        bool selected = (btn.textureIndex == gAppearance.playerTextureIndex);
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

    PlayerSave::activePlayer.playerTextureIndex = gAppearance.playerTextureIndex;
    PlayerSave::activePlayer.playerColor        = gAppearance.playerColor;
    PlayerSave::activePlayer.saveToFile();
}

void PlayerSettings::createSkinButtons() {
    auto& lib = PlayerSpriteLibrary::instance();
    int total = lib.getCount();

    int count = std::min(total, 10); // only first 10 PNGs

    skinButtons.clear();
    skinButtons.reserve(count);

    float boxSize = 64.f;
    skinBoxSize   = boxSize;
    skinSpacing   = 20.f;

    for (int i = 0; i < count; ++i) {
        SkinButton btn;
        btn.textureIndex = i;
        btn.isForPlayer  = true;

        btn.box.setSize({boxSize, boxSize});
        btn.box.setFillColor(sf::Color(50, 50, 50));
        btn.box.setOutlineThickness(2.f);
        btn.box.setOutlineColor(sf::Color::White);

        const sf::Texture& tex = lib.getTexture(i);
        btn.icon.setTexture(tex);

        sf::Vector2u texSize = tex.getSize();

        // Use the whole texture for each skin
        int frameW = static_cast<int>(texSize.x);
        int frameH = static_cast<int>(texSize.y);

        btn.icon.setTextureRect(sf::IntRect(0, 0, frameW, frameH));
        btn.icon.setOrigin(frameW / 2.f, frameH / 2.f);

        // Scale to fit inside the square box
        float scale = std::min(
            boxSize / static_cast<float>(frameW),
            boxSize / static_cast<float>(frameH)
        ) * 0.9f;
        btn.icon.setScale(scale, scale);

        if (hasFont) {
            btn.label.setFont(font);
            btn.label.setString("Skin " + std::to_string(i + 1));
            btn.label.setCharacterSize(14);
            btn.label.setFillColor(sf::Color::White);
        }

        skinButtons.push_back(btn);
    }
}

void PlayerSettings::draw() {
    window.clear(bgColor);

    if (hasFont) {
        // Title
        sf::Text title("Customize your farmer", font, 28);
        auto tb = title.getLocalBounds();
        title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        title.setPosition(window.getSize().x * 0.5f, 60.f);
        window.draw(title);

        // Preview label
        sf::Text previewLabel("Preview", font, 20);
        auto plb = previewLabel.getLocalBounds();
        previewLabel.setOrigin(plb.left + plb.width / 2.f, plb.top + plb.height / 2.f);
        previewLabel.setPosition(playerPreviewSprite.getPosition().x,
                                 playerPreviewSprite.getPosition().y - 90.f);
        window.draw(previewLabel);

        // Skins label
        sf::Text skinsLabel("Choose your skin", font, 20);
        auto slb = skinsLabel.getLocalBounds();
        skinsLabel.setOrigin(slb.left + slb.width / 2.f, slb.top + slb.height / 2.f);
        skinsLabel.setPosition(window.getSize().x * 0.5f, skinRowY - 48.f);
        window.draw(skinsLabel);
    }

    // Big preview
    window.draw(playerPreviewSprite);

    // Skin thumbnails
    for (const auto& btn : skinButtons) {
        window.draw(btn.box);
        window.draw(btn.icon);
        if (hasFont) window.draw(btn.label);
    }

    // Back button
    for (auto& b : buttons) {
        window.draw(b.circle);
        if (hasFont) window.draw(b.label);
    }

    window.display();
}
