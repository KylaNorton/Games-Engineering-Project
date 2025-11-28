#include "account.hpp"

Account::Account(sf::RenderWindow& window) : window(window) {
    // constructor body
}

void Account::handleEvent(const sf::Event& e) {
    // empty for now
}

void Account::draw() {
    window.clear(sf::Color(10, 10, 30)); // different color per page if you want
    window.display();
}

