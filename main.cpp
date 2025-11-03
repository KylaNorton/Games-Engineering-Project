#include <SFML/Graphics.hpp>
#include "menu.hpp"
#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode(960, 540), "Amalgame");
    window.setVerticalSyncEnabled(true);

    Menu menu(window);
    bool inMenu = true;

    while (window.isOpen()) {
        sf::Event e{};
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed)
                window.close();

            if (inMenu)
                menu.handleEvent(e);
            else if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
                // Go back to menu when Esc pressed
                inMenu = true;
                menu.clearAction(); // Clear *only when returning* to avoid relaunching right away
            }
        }

        if (inMenu) {
            menu.draw();

            MenuAction act = menu.getAction();
            if (act != MenuAction::None) {
                // Go to the chosen page
                inMenu = false;
                std::cout << "→ " 
                          << (act == MenuAction::Start ? "Start"
                              : act == MenuAction::Level ? "Level" 
                              : "Settings")
                          << " selected\n";

                // ⚠ clear AFTER leaving menu, so we don't auto-return
                menu.clearAction();
            }
        } else {
            // Dummy game screen
            window.clear(sf::Color(20, 40, 60));
            window.display();
        }
    }

    return 0;
}
