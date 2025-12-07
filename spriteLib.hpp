#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// ------------------------------------------------------------------
// Player sprite library (for different skins) 
// ------------------------------------------------------------------

class PlayerSpriteLibrary {
public:
    static PlayerSpriteLibrary& instance();

    // Load all skins once at the beginning of the program
    void load(const std::vector<std::string>& textureFiles);

    const sf::Texture& getTexture(int index) const;
    int getCount() const { return static_cast<int>(textures.size()); }

private:
    PlayerSpriteLibrary() = default;

    std::vector<sf::Texture> textures;
};
