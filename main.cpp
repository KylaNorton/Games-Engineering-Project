#include <SFML/Graphics.hpp>
#include "menu.hpp"
#include "game.hpp"
#include "level.hpp"
#include "settings.hpp"
#include "map.hpp"
#include "gameSettings.hpp"
#include "levelSettings.hpp"
#include "account.hpp"
#include "scores.hpp"
#include "player.hpp"
#include "spriteLib.hpp"
#include <iostream>

enum class Screen { Menu, Game, Level, Settings, Map, Scores, Account, GameSettings, LevelSettings, Player };

std::string CURRENT_PLAYER = "";
int currentLevel = 1;   // for now: always level 1, later change from LevelSettings / Map



int main() {

    sf::RenderWindow window(sf::VideoMode(960, 540), "Overgrown");
    window.setVerticalSyncEnabled(true);

    Menu menu(window);
    Level level(window);
    Settings settings(window);
    Map map(window);
    Scores scores(window);
    Account account(window);
    GameSettings gameSettings(window);
    LevelSettings levelSettings(window);
    PlayerSettings player(window);
    //PlayerSpriteLibrary spriteLib(window);
    Screen screen = Screen::Menu;

    PlayerSpriteLibrary::instance().load({
        "res/sprites/sprite1.png",
        "res/sprites/sprite2.png",
        "res/sprites/sprite3.png",
        "res/sprites/sprite4.png",
        "res/sprites/sprite5.png",
        "res/sprites/sprite6.png",
        "res/sprites/sprite7.png",
        "res/sprites/sprite8.png",
        "res/sprites/sprite9.png",
        "res/sprites/sprite10.png"
    });

    // init default appearance
    gAppearance.playerColor = sf::Color::Cyan;
    gAppearance.aiColor = sf::Color::Red;
    gAppearance.playerTextureIndex = 0;
    gAppearance.aiTextureIndex     = 1;

    Game* game = nullptr;  // pointer so we can reset the game when starting a new one
    
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
                    game->handleEvent(e);
                    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) screen = Screen::GameSettings;
                    break;
                case Screen::GameSettings:
                    gameSettings.handleEvent(e);
                    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
                        screen = Screen::Menu;
                        gameSettings.clearAction();
                    }
                    break;
                case Screen::Account:
                    account.handleEvent(e);
                    break;
                case Screen::Level:
                    level.handleEvent(e);
                    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) { 
                        level.reset(); screen = Screen::Menu; 
                    }
                    break;
                case Screen::LevelSettings:
                    levelSettings.handleEvent(e);
                    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) {
                        screen = Screen::Menu;
                        levelSettings.clearAction();
                    }
                    break;
                case Screen::Map:
                    map.handleEvent(e);
                    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) screen = Screen::Account;
                    break;
                case Screen::Scores:
                    scores.handleEvent(e);
                    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) screen = Screen::LevelSettings;
                    break;
                case Screen::Settings:
                    settings.handleEvent(e);
                    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) screen = Screen::Menu;
                    break;
                case Screen::Player:
                    player.handleEvent(e);
                    if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::Escape) screen = Screen::Settings;
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
                    if (a == MenuAction::Start)     screen = Screen::Account;
                    else if (a == MenuAction::Level)   screen = Screen::LevelSettings;
                    else if (a == MenuAction::Settings)screen = Screen::Settings;
                    menu.clearAction();
                }
            } break;

            case Screen::Game: {
                if (!game) {
                    game = new Game(window, currentLevel);
                }
                game->update(dt);
                game->draw();
                            
                GameAction b = game->getAction();
                if (b != GameAction::None) {
                    if (b == GameAction::Back) screen = Screen::GameSettings;
                    else if (b == GameAction::Play) {   // "Play again" from end popup
                        if (game) {
                            delete game;
                            game = nullptr;
                        }
                        game = new Game(window, currentLevel);
                        screen = Screen::Game;
                    }
                    game->clearAction();
                }
            } break;

            case Screen::GameSettings: {
                gameSettings.draw();

                GameSetAction a = gameSettings.getAction();
                if (a != GameSetAction::None) {
                    if (a == GameSetAction::NewGame) {
                        if (game) {
                            delete game;
                            game = nullptr;
                        }
                        game = new Game(window, currentLevel); 
                        screen = Screen::Game;
                    }
                    else if (a == GameSetAction::PlayAgain) {
                        if (game) {
                            delete game;
                            game = nullptr;
                        }
                        game = new Game(window, currentLevel);
                        screen = Screen::Game;
                    }
                    else if (a == GameSetAction::Account) {
                        screen = Screen::Account;
                    }
                    gameSettings.clearAction();
                }
            } break;

            case Screen::Account: {
                account.draw();

                AccountAction a = account.getAction();
                if (a != AccountAction::None) {
                    if (a == AccountAction::SelectPlayer) {
                        // Player selected an existing account → go to Map
                        screen = Screen::Map;
                    }
                    account.clearAction();
                }

                // If a new player was just created → go to Level 1
                if (account.newPlayerCreated()) {
                    currentLevel = 1;
                    if (game) {
                        delete game;
                        game = nullptr;
                    }
                    game = new Game(window, currentLevel);
                    game->setGamePaused(true); // start paused for tutorial
                    game->setTutorial(true); //show tuto for new players
                    screen = Screen::Game;
                    account.resetNewPlayerFlag();
                }
            } break;

            case Screen::Map: {
                map.draw();

                MapAction a = map.getAction();
                if (a != MapAction::None) {
                    // Determine which level was selected
                    if (a == MapAction::Level1) currentLevel = 1;
                    else if (a == MapAction::Level2) currentLevel = 2;
                    else if (a == MapAction::Level3) currentLevel = 3;
                    else if (a == MapAction::Level4) currentLevel = 4;

                    // Create a new game instance with the selected level
                    if (game) {
                        delete game;
                        game = nullptr;
                    }
                    game = new Game(window, currentLevel);
                    screen = Screen::Game;
                    map.clearAction();
                }
            } break;

            case Screen::Scores: {
                scores.draw();
             } break;
            
            case Screen::LevelSettings: {
                levelSettings.draw();

                LevelAction a = levelSettings.getAction();
                if (a != LevelAction::None) {
                    if (a == LevelAction::Map) {
                        // later: reset game state if needed
                        screen = Screen::Map;
                    }
                    else if (a == LevelAction::Scores) {
                        // same for now: go to Game
                        screen = Screen::Scores;
                    }
                    levelSettings.clearAction();
                }
            } break;

            case Screen::Level: {
                level.draw();
                // if a level selected, adjust game speed and go to menu
                auto d = level.chosen();
                    if (d != Difficulty::None) {
                    if (d == Difficulty::Easy)   game->setSpeed(160.f);
                    if (d == Difficulty::Medium) game->setSpeed(220.f);
                    if (d == Difficulty::Hard)   game->setSpeed(300.f);
                    level.reset();
                    screen = Screen::Menu;
                }
            } break;

            case Screen::Settings: {
                settings.draw();
                // read menu action → change screen
                SettingAction a = settings.getAction();
                if (a != SettingAction::None) {
                    if (a == SettingAction::Account) screen = Screen::Account;
                    else if (a == SettingAction::Customise) screen = Screen::Player;
                    else if (a == SettingAction::Tuto); //screen = Screen::Tutorial;
                    settings.clearAction();
                }
            }   break;

            case Screen::Player: {
               player.draw();
                auto a = player.getAction();
                if (a == PlayerSetAction::Back) {
                    screen = Screen::Settings;   // or Menu
                    player.clearAction();
                }
            } break;
            
        }
    }
    return 0;
}