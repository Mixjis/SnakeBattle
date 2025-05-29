#include "GameMenu.h"
#include <windows.h>
#include <fstream>

std::vector<sf::Color> GameMenu::availableColors = {
    sf::Color::Red, sf::Color::Blue, sf::Color::Yellow, sf::Color::Magenta,
    sf::Color::Cyan, sf::Color(255, 165, 0), sf::Color(0, 255, 0), sf::Color(255, 0, 255),
	sf::Color(255, 192, 203), sf::Color(128, 0, 128), sf::Color(0, 128, 128)
};

void GameMenu::initNicknameInput() {
    //nicknameInput = "";
    nicknameLabel.setFont(font);
    nicknameLabel.setString("Nickname:");
    nicknameLabel.setCharacterSize(24);
    nicknameLabel.setFillColor(sf::Color::White);
    nicknameLabel.setPosition(window.getSize().x / 2.f - 150.f, 150.f);

    nicknameBox.setSize(sf::Vector2f(300.f, 40.f));
    nicknameBox.setPosition(window.getSize().x / 2.f - 150.f, 180.f);
    nicknameBox.setFillColor(sf::Color(50, 50, 50));
    nicknameBox.setOutlineThickness(2.f);
    nicknameBox.setOutlineColor(sf::Color::White);

    nicknameText.setFont(font);
    nicknameText.setCharacterSize(22);
    nicknameText.setFillColor(sf::Color::White);
    nicknameText.setPosition(window.getSize().x / 2.f - 140.f, 185.f);
}

void GameMenu::handleNicknameInput(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        nicknameActive = nicknameBox.getGlobalBounds().contains(mousePos);
    }
    if (event.type == sf::Event::TextEntered && nicknameActive) {
        if (event.text.unicode == 8) { // Backspace
            if (!nicknameInput.isEmpty()) nicknameInput.erase(nicknameInput.getSize() - 1, 1);
        }
        else if (event.text.unicode >= 32 && event.text.unicode < 127 && nicknameInput.getSize() < 16) {
            nicknameInput += static_cast<sf::Uint32>(event.text.unicode);
        }
    }
}

void GameMenu::drawNicknameInput() {
    window.draw(nicknameLabel);
    window.draw(nicknameBox);
    nicknameText.setString(nicknameInput);
    window.draw(nicknameText);
}
void GameMenu::saveSettings() {
    try {
        std::ofstream file("settings.txt");
        if (file) {
            std::string nicknameStr = nicknameInput.toAnsiString();
            file << nicknameStr << "\n";
            file << selectedColorIndex << "\n";
            file.close();
            //std::cout << "SAVED: " << nicknameInput.toAnsiString() << selectedColorIndex<< "\n";
		}
		else throw std::runtime_error("Cannot open settings.txt");
    }
	catch (const std::exception& e) {
		std::cerr << "Error saving settings: " << e.what() << std::endl;
	}
}

void GameMenu::loadSettings() {
    std::ifstream file("settings.txt");
    try {
        if (file) {
            std::string nicknameStr;
            std::getline(file, nicknameStr);
            nicknameInput = sf::String(nicknameStr);

            /*int r, g, b, a;
            file >> r >> g >> b >> a;
            selectedColor = sf::Color(r, g, b, a);*/
            file >> selectedColorIndex;
            selectedColor = availableColors[selectedColorIndex];
            file.close();
            //std::cout << "Loaded: " << nicknameInput.toAnsiString() << selectedColorIndex << "\n";
        }
        else throw std::runtime_error("Cannot open settings.txt");
    }
	catch (const std::exception& e) {
		std::cerr << "Error loading settings: " << e.what() << std::endl;

		nicknameInput = sf::String("");
		selectedColorIndex = 0;
		selectedColor = availableColors[selectedColorIndex];
	}
}

GameMenu::GameMenu() : selectedMode(GameMode::NONE) {

    loadSettings();

    // Tworzenie okna
    window.create(sf::VideoMode(1200, 800), "Snake Battle", sf::Style::Close | sf::Style::Titlebar);

    //Muzyka Menu
    try {
        if (!menuMusic.openFromFile("assets/bgmusic1.mp3")) throw std::runtime_error("Cannot open music file");
        else menuMusic.setLoop(true);
    }
	catch (const std::exception& e) {
		std::cerr << "Error loading music file: " << e.what() << std::endl;
		exit(1);
	}
    

    // Ładowanie czcionki Menu
	try {
		if (!font.loadFromFile("assets/Pixel.ttf")) throw std::runtime_error("Cannot load font");
        else titleText.setLetterSpacing(1.5f);
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading font: " << e.what() << std::endl;
        exit(1);
    }
    

    // Tytuł Menu
    titleText.setFont(font);
    titleText.setString("Snake Battle!");
    titleText.setCharacterSize(100);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(
        (window.getSize().x / 2.f) - (titleText.getGlobalBounds().width / 2.f),
        30.f
    );

    // Tworzenie przycisków
    hostButton = new Button(
        window.getSize().x / 2.f - 150.f, 250.f,
        300.f, 50.f, font, "Host Game",
        sf::Color(70, 70, 200), sf::Color(100, 100, 220), sf::Color(50, 50, 150)
    );

    clientButton = new Button(
        window.getSize().x / 2.f - 150.f, 350.f,
        300.f, 50.f, font, "Join Game",
        sf::Color(70, 70, 200), sf::Color(100, 100, 220), sf::Color(50, 50, 150)
    );

    //wybor koloru

    leftColorButton = new Button(
        window.getSize().x / 2.f - 140.f, 500.f, 60.f, 60.f, font, "<",
        sf::Color(70, 70, 200), sf::Color(100, 100, 220), sf::Color(50, 50, 150)
    );
    rightColorButton = new Button(
        window.getSize().x / 2.f + 80.f, 500.f, 60.f, 60.f, font, ">",
        sf::Color(70, 70, 200), sf::Color(100, 100, 220), sf::Color(50, 50, 150)
    );

    //selectedColorIndex = 0;
    //selectedColor = availableColors[0];
    // Podgląd koloru
    colorPreview.setSize(sf::Vector2f(60, 60));
    colorPreview.setOrigin(30, 0);
    colorPreview.setPosition(window.getSize().x / 2.f, 500.f);
    colorPreview.setFillColor(availableColors[selectedColorIndex]);
    colorPreview.setOutlineThickness(4.f);
    colorPreview.setOutlineColor(sf::Color(50, 50, 50));

    //Ustawienie funkcji dla przycisków

    leftColorButton->setOnClick([this]() {
        selectedColorIndex = (selectedColorIndex - 1 + availableColors.size()) % availableColors.size();
        colorPreview.setFillColor(availableColors[selectedColorIndex]);
        this->selectedColor = availableColors[selectedColorIndex];
        });

    rightColorButton->setOnClick([this]() {
        selectedColorIndex = (selectedColorIndex + 1) % availableColors.size();
        colorPreview.setFillColor(availableColors[selectedColorIndex]);
        this->selectedColor = availableColors[selectedColorIndex];
        });

    hostButton->setOnClick([this]() {
        this->selectedMode = GameMode::HOST;
        std::cout << "Wybrano tryb: Host" << std::endl;
        });

    clientButton->setOnClick([this]() {
        this->selectedMode = GameMode::CLIENT;
        std::cout << "Wybrano tryb: Client" << std::endl;
        });
}

sf::RenderWindow& GameMenu::getWindow() {
    return window;
}

GameMenu::~GameMenu() {
    delete leftColorButton;
    delete rightColorButton;
    delete hostButton;
    delete clientButton;
}

GameMode GameMenu::run() {
    sf::Clock clock;
    menuMusic.play();
    initNicknameInput();

    while (window.isOpen() && selectedMode == GameMode::NONE) {
        
        sf::Event event;

        while (window.pollEvent(event)) {

            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // Sprawdzenie kliknięć przycisków
            hostButton->checkClick(event, window);
            clientButton->checkClick(event, window);
            rightColorButton->checkClick(event, window);
            leftColorButton->checkClick(event, window);
            handleNicknameInput(event);
        }

        // Aktualizacja przycisków
        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        sf::Vector2f worldPos = window.mapPixelToCoords(mousePosition);

        hostButton->update(worldPos);
        clientButton->update(worldPos);
        clientButton->update(worldPos);
        leftColorButton->update(worldPos);
        rightColorButton->update(worldPos);


        // Renderowanie
        window.clear(sf::Color(30, 30, 30));

        window.draw(titleText);

        hostButton->render(window);
        clientButton->render(window);
        leftColorButton->render(window);
        rightColorButton->render(window);
        window.draw(colorPreview);
        drawNicknameInput();

        window.display();
    }

    saveSettings();
    menuMusic.stop();
    return selectedMode;
}