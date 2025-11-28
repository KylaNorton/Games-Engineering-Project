#include "map.hpp"

Map::Map(sf::RenderWindow& window): window(window) {
    //empty for now
}

void Map::handleEvent(const sf::Event& e) {
    // empty for now
}

void Map::draw() {
    window.clear(sf::Color(10, 30, 50)); // only a background
    window.display();
}
