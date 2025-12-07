#include "spriteLib.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>

PlayerSpriteLibrary& PlayerSpriteLibrary::instance() {
    static PlayerSpriteLibrary lib;
    return lib;
}

void PlayerSpriteLibrary::load(const std::vector<std::string>& textureFiles) {
    textures.clear();
    textures.reserve(textureFiles.size());

    for (const auto& file : textureFiles) {
        sf::Texture tex;
        if (!tex.loadFromFile(file)) {
            std::cerr << "[WARN] Failed to load player texture: " << file << "\n";
            continue; // skip missing/invalid files instead of throwing
        }
        // Optional: smooth the sprites
        tex.setSmooth(true);
        textures.push_back(std::move(tex));
    }
}

void PlayerSpriteLibrary::loadAiTexture(const std::string& path) {
    // Try exact path first, then common extensions
    const std::vector<std::string> exts = {"", ".png", ".jpg", ".jpeg"};
    sf::Texture tex;
    bool loaded = false;
    for (const auto& ext : exts) {
        std::string tryPath = path + ext;
        if (tex.loadFromFile(tryPath)) {
            loaded = true;
            break;
        }
    }

    if (!loaded) {
        std::cerr << "[WARN] Failed to load AI texture: " << path << " (tried common extensions)\n";
        aiTextureLoaded = false;
        return;
    }

    tex.setSmooth(true);
    aiTexture = std::move(tex);
    aiTextureLoaded = true;
}

bool PlayerSpriteLibrary::hasAiTexture() const {
    return aiTextureLoaded;
}

const sf::Texture& PlayerSpriteLibrary::getAiTexture() const {
    if (!aiTextureLoaded) throw std::out_of_range("AI texture not loaded");
    return aiTexture;
}

const sf::Texture& PlayerSpriteLibrary::getTexture(int index) const {
    if (index < 0 || index >= static_cast<int>(textures.size())) {
        throw std::out_of_range("Invalid player texture index");
    }
    return textures[index];
}
