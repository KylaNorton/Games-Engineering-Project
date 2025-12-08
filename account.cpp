#include "account.hpp"
#include "PlayerSave.hpp"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <algorithm>

extern std::string CURRENT_PLAYER;

Account::Account(sf::RenderWindow& window) : window(window){
    // Load font
    hasFont = font.loadFromFile("res/fonts/Inter-Regular.ttf");
    if (!hasFont) {
        std::cerr << "[WARN] Font not found at res/fonts/Inter-Regular.ttf. "
                     "Buttons will show without text.\n";
    }

    // Setup "Create New Player" button in top right
    setupButton(createButton, "  New Player", {800.f, 20.f});
    createButton.box.setSize({140.f, 50.f});


    // Load all player buttons
    loadPlayerButtons();

    // Setup input text for new player name
    inputText.setFont(font);
    inputText.setCharacterSize(28);
    inputText.setFillColor(sf::Color::White);
    inputText.setPosition(250.f, 200.f);
    inputText.setString("");

    // Setup rename input text
    renameText.setFont(font);
    renameText.setCharacterSize(28);
    renameText.setFillColor(sf::Color::White);
    renameText.setPosition(250.f, 200.f);
    renameText.setString("");
}

void Account::loadPlayerButtons(){
    playerButtons.clear();

    std::vector<std::string> players = PlayerSave::loadPlayerList();

    for (const auto& filename : players) {
        PlayerButton pb;
        pb.playerFilename = filename;
        setupPlayerButton(pb, filename);
        playerButtons.push_back(pb);
    }

    // Reset scroll offset
    scrollOffset = 0.f;
}

void Account::recomputeLayout(){
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // left margin and content width
    listLeftX = std::max(40.f, winW * 0.12f);
    contentWidth = std::max(360.f, winW - listLeftX - 60.f);

    // list area height
    listStartY = std::max(80.f, winH * 0.18f);
    listMaxHeight = std::max(240.f, winH * 0.6f);

    // Create button (top-right)
    sf::Vector2f accSize(140.f, 50.f);
    sf::Vector2f accPos(winW - accSize.x - 30.f, 20.f);
    createButton.box.setPosition(accPos);
    createButton.box.setSize(accSize);
    if (hasFont) centerLabel(createButton);

    // Input text positions for entry/rename - center-ish
    inputText.setPosition(winW * 0.5f - 160.f, winH * 0.35f);
    renameText.setPosition(winW * 0.5f - 160.f, winH * 0.35f);

    // Resize each player button width and re-center label
    for (auto &pb : playerButtons) {
        pb.box.setSize({contentWidth, playerButtonHeight});
        centerPlayerLabel(pb);
    }

    // Update scroll bounds after layout change
    updateScrollPosition();
}

void Account::setupButton(Button& b, const std::string& text, sf::Vector2f pos){
    b.box.setSize(sf::Vector2f(120.f, 50.f));
    b.box.setPosition(pos);
    b.box.setFillColor(idle);
    b.box.setOutlineThickness(2.f);
    b.box.setOutlineColor(sf::Color::White);

    if (hasFont) {
        b.label.setFont(font);
        b.label.setString(text);
        b.label.setCharacterSize(16);
        b.label.setFillColor(textColor);
        centerLabel(b);
    }
}

void Account::setupPlayerButton(PlayerButton& pb, const std::string& name){
    pb.box.setSize({700.f, playerButtonHeight});
    pb.box.setFillColor(idle);
    pb.box.setOutlineThickness(2.f);
    pb.box.setOutlineColor(sf::Color::White);

    if (hasFont) {
        pb.label.setFont(font);
        pb.label.setString(name);
        pb.label.setCharacterSize(24);
        pb.label.setFillColor(textColor);
        centerPlayerLabel(pb);
    }
}

void Account::centerLabel(Button& b){
    sf::FloatRect t = b.label.getLocalBounds();
    b.label.setOrigin(t.left + t.width / 2.f, t.top + t.height / 2.f);

    sf::FloatRect r = b.box.getGlobalBounds();
    b.label.setPosition(r.left + r.width / 2.f, r.top + r.height / 2.f);
}

void Account::centerPlayerLabel(PlayerButton& pb){
    sf::FloatRect t = pb.label.getLocalBounds();
    pb.label.setOrigin(t.left + t.width / 2.f, t.top + t.height / 2.f);

    sf::FloatRect r = pb.box.getGlobalBounds();
    pb.label.setPosition(r.left + r.width / 2.f, r.top + r.height / 2.f);
}

void Account::positionPlayerLabelLeft(PlayerButton& pb){
    sf::FloatRect t = pb.label.getLocalBounds();
    // origin at left, vertically centered
    pb.label.setOrigin(t.left, t.top + t.height / 2.f);

    sf::FloatRect r = pb.box.getGlobalBounds();
    pb.label.setPosition(r.left + 12.f, r.top + r.height / 2.f - 2.f);
}

void Account::updateScrollPosition(){
    // Calculate total content height
    float totalHeight = playerButtons.size() * (playerButtonHeight + playerButtonGap);
    
    // Clamp scroll offset to valid range
    if (totalHeight > listMaxHeight) {
        float maxScroll = totalHeight - listMaxHeight;
        if (scrollOffset < 0.f) scrollOffset = 0.f;
        if (scrollOffset > maxScroll) scrollOffset = maxScroll;
    } else {
        scrollOffset = 0.f;
    }
}

void Account::handleEvent(const sf::Event& e){
    // Rename input mode
    if (renamingIndex != -1) {
        if (e.type == sf::Event::TextEntered) {
            if (e.text.unicode == '\b') {
                if (!renameInput.empty()) renameInput.pop_back();
            }
            else if (e.text.unicode == '\r') {
                // ENTER pressed -> perform rename
                if (!renameInput.empty() && renamingIndex >= 0 && renamingIndex < static_cast<int>(playerButtons.size())) {
                    auto trim = [](const std::string& s) {
                        size_t a = s.find_first_not_of(" \t\n\r");
                        if (a == std::string::npos) return std::string();
                        size_t b = s.find_last_not_of(" \t\n\r");
                        return s.substr(a, b - a + 1);
                    };

                    std::string oldFilename = trim(playerButtons[renamingIndex].playerFilename);
                    std::string oldPath = std::string("../../../save_files/") + oldFilename;
                    
                    // Extract old player name (x_save.txt)
                    std::string oldName = oldFilename;
                    const std::string suffix = "_save.txt";
                    if (oldName.size() > suffix.size() &&
                        oldName.substr(oldName.size() - suffix.size()) == suffix) {
                        oldName = oldName.substr(0, oldName.size() - suffix.size());
                    }

                    std::string newName = trim(renameInput);
                    std::string newFilename = newName + "_save.txt";
                    std::string newPath = std::string("../../../save_files/") + newFilename;

                    // Rename the file
                    if (std::rename(oldPath.c_str(), newPath.c_str()) == 0) {
                        std::cout << "Renamed file from " << oldPath << " to " << newPath << "\n";
                    } else {
                        std::cerr << "Failed to rename file\n";
                        renamingIndex = -1;
                        renameInput.clear();
                        renameText.setString("");
                        return;
                    }

                    // Update the Name: field inside the save file
                    std::ifstream saveIn(newPath);
                    std::vector<std::string> saveLines;
                    std::string saveLine;
                    while (std::getline(saveIn, saveLine)) {
                        if (saveLine.rfind("Name:", 0) == 0) {
                            // Replace the Name: line
                            saveLines.push_back(std::string("Name: ") + newName);
                        } else {
                            saveLines.push_back(saveLine);
                        }
                    }
                    saveIn.close();

                    std::ofstream saveOut(newPath);
                    for (auto &L : saveLines) saveOut << L << "\n";
                    saveOut.close();

                    std::cout << "Updated Name: field in save file to " << newName << "\n";

                    // Update players.txt
                    std::ifstream in("../../../players.txt");
                    std::vector<std::string> lines;
                    std::string line;
                    while (std::getline(in, line)) {
                        std::string trimmed = trim(line);
                        if (trimmed == oldFilename) {
                            lines.push_back(newFilename);
                        } else if (!line.empty()) {
                            lines.push_back(line);
                        }
                    }
                    in.close();

                    std::ofstream out("../../../players.txt");
                    for (auto &L : lines) out << L << "\n";
                    out.close();

                    std::cout << "Renamed player from " << oldName << " to " << newName << "\n";

                    // Reload and exit rename mode
                    loadPlayerButtons();
                    renamingIndex = -1;
                    renameInput.clear();
                    renameText.setString("");
                }
                return;
            }
            else if (e.text.unicode < 128 && std::isalnum(e.text.unicode)) {
                renameInput += static_cast<char>(e.text.unicode);
            }
            renameText.setString("New name: " + renameInput);
        }
        return;
    }

    // Text entry mode (create new player)
    if (enteringName) {
        if (e.type == sf::Event::TextEntered) {
            if (e.text.unicode == '\b') {
                if (!newNameInput.empty()) newNameInput.pop_back();
            }
            else if (e.text.unicode == '\r') {
                // ENTER pressed -> create player
                PlayerSave::createNewPlayer(newNameInput);
                // Set the new player as the current active player so scores go to them
                {
                    std::string trimmed = newNameInput;
                    // update global current player
                    CURRENT_PLAYER = trimmed;
                    PlayerSave::setActivePlayer(PlayerSave(trimmed));
                }
                loadPlayerButtons();
                enteringName = false;
                newNameInput.clear();
                inputText.setString("");
                wasNewPlayerCreated = true;  // Flag that a new player was created
                return;
            }
            else if (e.text.unicode < 128 && std::isalnum(e.text.unicode)) {
                newNameInput += static_cast<char>(e.text.unicode);
            }
            inputText.setString("Name: " + newNameInput);
        }
        return;
    }

    // Handle mouse move (update hover index)
    if (e.type == sf::Event::MouseMoved) {
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        sf::Vector2f world = window.mapPixelToCoords(mp);
        hoveredIndex = -1;

        float currentY = listStartY - scrollOffset;
        for (size_t i = 0; i < playerButtons.size(); ++i) {
            float y = currentY + i * (playerButtonHeight + playerButtonGap);
            sf::FloatRect r(listLeftX, y, playerButtons[i].box.getSize().x, playerButtonHeight);
            if (r.contains(world)) {
                hoveredIndex = static_cast<int>(i);
                break;
            }
        }
    }

    // Handle scroll wheel
    if (e.type == sf::Event::MouseWheelScrolled) {
        if (e.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
            scrollOffset -= e.mouseWheelScroll.delta * scrollSpeed;
            updateScrollPosition();
        }
        return;
    }

    // Button clicks
    if (e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mp = sf::Mouse::getPosition(window);
        sf::Vector2f world = window.mapPixelToCoords(mp);

        // Check "Create New Player" button
        if (createButton.contains(window, mp)) {
            enteringName = true;
            newNameInput.clear();
            inputText.setString("Name: ");
            return;
        }

        // Check player buttons (with scroll offset) and action buttons
        float currentY = listStartY - scrollOffset;
        for (size_t i = 0; i < playerButtons.size(); ++i) {
            float y = currentY + i * (playerButtonHeight + playerButtonGap);
            sf::FloatRect boxRect(listLeftX, y, playerButtons[i].box.getSize().x, playerButtonHeight);
            if (boxRect.contains(world)) {
                // Compute rename and delete button rects
                float delW = deleteButtonWidth;
                float renW = renameButtonWidth;
                float btnH = playerButtonHeight - 16.f;
                float delX = listLeftX + playerButtons[i].box.getSize().x - delW - deleteButtonPadding;
                float renX = delX - renW - 8.f;
                float btnY = y + (playerButtonHeight - btnH) / 2.f;

                sf::FloatRect renRect(renX, btnY, renW, btnH);
                sf::FloatRect delRect(delX, btnY, delW, btnH);

                if (renRect.contains(world)) {
                    // start rename 
                    renamingIndex = static_cast<int>(i);
                    renameInput.clear();
                    renameText.setString("New name: ");
                    return;
                }

                if (delRect.contains(world)) {
                    // delete the player
                    auto trim = [](const std::string& s) {
                        size_t a = s.find_first_not_of(" \t\n\r");
                        if (a == std::string::npos) return std::string();
                        size_t b = s.find_last_not_of(" \t\n\r");
                        return s.substr(a, b - a + 1);
                    };
                    std::string fn = trim(playerButtons[i].playerFilename);
                    std::string fullpath = std::string("../../../save_files/") + fn;
                    if (std::remove(fullpath.c_str()) == 0) {
                        std::cout << "Deleted file: " << fullpath << "\n";
                    } else {
                        std::cerr << "Failed to delete file: " << fullpath << "\n";
                    }

                    // Update players.txt
                    std::ifstream in("../../../players.txt");
                    std::vector<std::string> lines;
                    std::string line;
                    while (std::getline(in, line)) {
                        if (trim(line) != fn) lines.push_back(line);
                    }
                    in.close();

                    std::ofstream out("../../../players.txt");
                    for (auto &L : lines) out << L << "\n";
                    out.close();

                    // Reload buttons and exit
                    loadPlayerButtons();
                    hoveredIndex = -1;
                    return;
                }

                // Not clicking rename/delete -> select player
                std::string fn = playerButtons[i].playerFilename;
                auto trim = [](const std::string& s) {
                    size_t a = s.find_first_not_of(" \t\n\r");
                    if (a == std::string::npos) return std::string();
                    size_t b = s.find_last_not_of(" \t\n\r");
                    return s.substr(a, b - a + 1);
                };
                fn = trim(fn);
                std::string playerName = fn;
                const std::string suffix = "_save.txt";
                if (playerName.size() > suffix.size() &&
                    playerName.substr(playerName.size() - suffix.size()) == suffix) {
                    playerName = playerName.substr(0, playerName.size() - suffix.size());
                }

                CURRENT_PLAYER = playerName;
                PlayerSave::setActivePlayer(PlayerSave(playerName));
                std::cout << "Selected player: " << playerName << "\n";
                action = AccountAction::SelectPlayer;
                return;
            }
        }
    }
}


void Account::draw(){
    window.clear(bgColor);

    // Draw title
    if (hasFont) {
        sf::Text title("Select Player", font, 32);
        title.setFillColor(textColor);
        title.setPosition(50.f, 20.f);
        window.draw(title);
    }

    // Draw "Create New Player" button
    window.draw(createButton.box);
    if (hasFont) window.draw(createButton.label);


    // Update scroll position
    updateScrollPosition();

    // Draw scrollable player list
    float currentY = listStartY - scrollOffset;

    for (size_t i = 0; i < playerButtons.size(); ++i) {
        auto& pb = playerButtons[i];
        // Only draw if within visible list area
        if (currentY + playerButtonHeight >= listStartY && currentY < listStartY + listMaxHeight) {
            pb.box.setPosition(listLeftX, currentY);
            // Position label left inside the box
            if (hasFont) positionPlayerLabelLeft(pb);

            // Hover color
            if (hoveredIndex == static_cast<int>(i)) pb.box.setFillColor(hover);
            else pb.box.setFillColor(idle);

            window.draw(pb.box);
            if (hasFont) window.draw(pb.label);

            // Draw delete button and rename button when hovered
            if (hoveredIndex == static_cast<int>(i) && hasFont) {
                float delW = deleteButtonWidth;
                float renW = renameButtonWidth;
                float btnH = playerButtonHeight - 16.f;
                float delX = pb.box.getPosition().x + pb.box.getSize().x - delW - deleteButtonPadding;
                float renX = delX - renW - 8.f;
                float btnY = pb.box.getPosition().y + (playerButtonHeight - btnH) / 2.f;

                // Draw Rename button
                sf::RectangleShape renRect({renW, btnH});
                renRect.setPosition(renX, btnY);
                renRect.setFillColor(sf::Color(100, 150, 200));
                renRect.setOutlineThickness(2.f);
                renRect.setOutlineColor(sf::Color::White);
                window.draw(renRect);

                sf::Text renText("Rename", font, 16);
                renText.setFillColor(sf::Color::White);
                sf::FloatRect rt = renText.getLocalBounds();
                renText.setOrigin(rt.left + rt.width / 2.f, rt.top + rt.height / 2.f);
                renText.setPosition(renX + renW / 2.f, btnY + btnH / 2.f - 2.f);
                window.draw(renText);

                // Draw Delete button
                sf::RectangleShape delRect({delW, btnH});
                delRect.setPosition(delX, btnY);
                delRect.setFillColor(sf::Color(200, 40, 40));
                delRect.setOutlineThickness(2.f);
                delRect.setOutlineColor(sf::Color::White);
                window.draw(delRect);

                sf::Text delText("Delete", font, 18);
                delText.setFillColor(sf::Color::White);
                sf::FloatRect dt = delText.getLocalBounds();
                delText.setOrigin(dt.left + dt.width / 2.f, dt.top + dt.height / 2.f);
                delText.setPosition(delX + delW / 2.f, btnY + btnH / 2.f - 2.f);
                window.draw(delText);
            }
        }
        currentY += playerButtonHeight + playerButtonGap;
    }

    // Draw text input box if in name entry mode
    if (enteringName) {
        sf::RectangleShape box;
        box.setSize({400.f, 80.f});
        box.setFillColor(sf::Color(20, 20, 20));
        box.setOutlineColor(sf::Color::White);
        box.setOutlineThickness(2.f);

        // position box centered near inputText
        sf::Vector2f it = inputText.getPosition();
        box.setPosition(it.x - 12.f, it.y - box.getSize().y / 2.f);

        // Position inputText inside the box with left padding and vertical centering
        if (hasFont) {
            sf::FloatRect tb = inputText.getLocalBounds();
            inputText.setOrigin(tb.left, tb.top + tb.height / 2.f);
            inputText.setPosition(box.getPosition().x + 12.f, box.getPosition().y + box.getSize().y / 2.f);
        } else {
            // Fallback position if font isn't loaded
            inputText.setPosition(box.getPosition().x + 12.f, box.getPosition().y + 12.f);
        }

        window.draw(box);
        window.draw(inputText);
    }

    // Draw text input box if in rename mode
    if (renamingIndex >= 0) {
        sf::RectangleShape box;
        box.setSize({400.f, 80.f});
        box.setFillColor(sf::Color(20, 20, 20));
        box.setOutlineColor(sf::Color::White);
        box.setOutlineThickness(2.f);

        // position box centered near renameText
        sf::Vector2f rt = renameText.getPosition();
        box.setPosition(rt.x - 12.f, rt.y - box.getSize().y / 2.f);

        // Position renameText inside the box with left padding and vertical centering
        if (hasFont) {
            sf::FloatRect tb = renameText.getLocalBounds();
            renameText.setOrigin(tb.left, tb.top + tb.height / 2.f);
            renameText.setPosition(box.getPosition().x + 12.f, box.getPosition().y + box.getSize().y / 2.f);
        } else {
            // Fallback position if font isn't loaded
            renameText.setPosition(box.getPosition().x + 12.f, box.getPosition().y + 12.f);
        }

        window.draw(box);
        window.draw(renameText);
    }

    window.display();}

