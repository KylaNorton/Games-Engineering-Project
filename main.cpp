#include <SFML/Graphics.hpp>
#include "menu.hpp"
#include "game.hpp"
#include "level.hpp"
#include "settings.hpp"
#include <iostream>

enum class Screen { Menu, Game, Level, Settings };

int main() {
  

    sf::RenderWindow window(sf::VideoMode(960, 540), "Overgrown");
    window.setVerticalSyncEnabled(true);

    Menu menu(window);
    Game game(window);
    LevelPage level(window);
    SettingsPage settings(window);
    Screen screen = Screen::Menu;

    sf::Clock clk;

    bool inMenu = true;

    while (window.isOpen()) {
        sf::Event e{};
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) window.close();

            switch (screen) {
                case Screen::Menu:
                    menu.handleEvent(e);
                    break;
                case Screen::Game:
                    game.handleEvent(e);
                    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) screen = Screen::Menu;
                    break;
                case Screen::Level:
                    level.handleEvent(e);
                    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) { level.reset(); screen = Screen::Menu; }
                    break;
                case Screen::Settings:
                    settings.handleEvent(e);
                    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) screen = Screen::Menu;
                    break;
            }
        }

        float dt = clk.restart().asSeconds();

        switch (screen) {
            case Screen::Menu: {
                menu.draw();
                // read menu action → change screen
                MenuAction a = menu.getAction();
                if (a != MenuAction::None) {
                    if (a == MenuAction::Start)     screen = Screen::Game;
                    else if (a == MenuAction::Level)   screen = Screen::Level;
                    else if (a == MenuAction::Settings)screen = Screen::Settings;
                    menu.clearAction();
                }
            } break;

            case Screen::Game: {
                game.update(dt);
                game.draw();
                
                GameAction b = game.getAction();
                if (b != GameAction::None) {
                    if (b == GameAction::Back) screen = Screen::Menu;
                    else if (b == GameAction::Pause) ;
                    game.clearAction();
                }
            }    break;

            case Screen::Level: {
                level.draw();
                // if a level selected, adjust game speed and go to menu
                auto d = level.chosen();
                if (d != Difficulty::None) {
                    if (d == Difficulty::Easy)   game.setSpeed(160.f);
                    if (d == Difficulty::Medium) game.setSpeed(220.f);
                    if (d == Difficulty::Hard)   game.setSpeed(300.f);
                    level.reset();
                    screen = Screen::Menu;
                }
            } break;

            case Screen::Settings: {
                settings.draw();
                // you can read settings.showFps() later to render FPS text in Game
            }   break;
            
        }
    }
    return 0;
}