#include "scores.hpp"

Scores::Scores(sf::RenderWindow& window) : window(window) {
    // constructor body
}

void Scores::handleEvent(const sf::Event& e) {
    // empty for now
}

void Scores::draw() {
    window.clear(sf::Color(10, 10, 30)); // different color per page if you want
    window.display();
}
