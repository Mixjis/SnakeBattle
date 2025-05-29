#pragma once

#ifndef GAMEMENU_H
#define GAMEMENU_H


#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <iostream>
#include "Button.h"
#include "API.h"

enum class GameMode {
    NONE,
    HOST,
    CLIENT
};

class EXPORT_API GameMenu {
private:
    sf::RenderWindow window;
    sf::Font font;
    Button* hostButton;
    Button* clientButton;

    Button* leftColorButton;
    Button* rightColorButton;
    sf::RectangleShape colorPreview;
    

    sf::Text titleText;
    GameMode selectedMode;
    static std::vector<sf::Color> availableColors;

    bool nicknameActive = false;
    sf::Text nicknameLabel;
    sf::RectangleShape nicknameBox;
    sf::Text nicknameText;
    sf::Music menuMusic;

    void initNicknameInput();
    void handleNicknameInput(const sf::Event& event);
    void drawNicknameInput();

public:
    sf::String nicknameInput;
    sf::Color selectedColor;
    int selectedColorIndex;

    GameMenu();
    ~GameMenu();

    sf::RenderWindow& getWindow();
    GameMode run();

    void saveSettings();
    void loadSettings();
};

#endif // GAMEMENU_H