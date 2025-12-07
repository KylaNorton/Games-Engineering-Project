#include "spriteLib.hpp"
#include <stdexcept>

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
            throw std::runtime_error("Failed to load player texture: " + file);
        }
        // Optional: smooth the sprites
        tex.setSmooth(true);
        textures.push_back(std::move(tex));
    }
}

const sf::Texture& PlayerSpriteLibrary::getTexture(int index) const {
    if (index < 0 || index >= static_cast<int>(textures.size())) {
        throw std::out_of_range("Invalid player texture index");
    }
    return textures[index];
}
