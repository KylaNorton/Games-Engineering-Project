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

    // Load a dedicated AI texture (single file). Path may be given without extension.
    void loadAiTexture(const std::string& path);

    // Query whether an AI texture was loaded
    bool hasAiTexture() const;

    // Get the AI texture (only valid if hasAiTexture() is true)
    const sf::Texture& getAiTexture() const;

    const sf::Texture& getTexture(int index) const;
    int getCount() const { return static_cast<int>(textures.size()); }

private:
    PlayerSpriteLibrary() = default;

    std::vector<sf::Texture> textures;
    sf::Texture aiTexture;
    bool aiTextureLoaded = false;
};
