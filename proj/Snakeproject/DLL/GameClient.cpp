#include "GameClient.h"

// Konstruktor bez IP
GameClient::GameClient(sf::RenderWindow& existingWindow)
    : GameBase(existingWindow), isRunning(false), isConnected(false), localPlayerId(-1) {
    // Aktualizacja tytułu okna
    window.setTitle("Slither.io LAN - Klient");
}

// Konstruktor z IP
GameClient::GameClient(sf::RenderWindow& existingWindow, const std::string& ip)
    : GameBase(existingWindow), isRunning(false), isConnected(false), localPlayerId(-1), serverIp(ip) {
    // Aktualizacja tytułu okna
    window.setTitle("Slither.io LAN - Klient");
}

// Destruktor
GameClient::~GameClient() {
    // Wysłanie komunikatu o rozłączeniu
    if (isConnected) {
        sf::Packet disconnectPacket;
        disconnectPacket << PacketType::DISCONNECT << localPlayerId;
        socket.send(disconnectPacket);
    }

    isRunning = false;
    if (networkThread.joinable()) {
        networkThread.join();
    }
    socket.disconnect();
}

// Ustawienie IP serwera
void GameClient::setServerIp(const std::string& ip) {
    serverIp = ip;
}

// Połączenie z serwerem
bool GameClient::connectToServer() {
    if (serverIp.empty()) {
        std::cout << "Enter your server IP address: ";
        std::cin >> serverIp;
    }

    std::cout << "Connecting to the server " << serverIp << ":" << DEFAULT_PORT << "..." << std::endl;
    std::cout << "Connecting as: " << snakeNickname.toAnsiString() << std::endl;

    sf::Socket::Status status = socket.connect(serverIp, DEFAULT_PORT, sf::seconds(5));
    if (status != sf::Socket::Done) {
        std::cerr << "Cannot connect to server!" << std::endl;
        return false;
    }

    socket.setBlocking(false);
    sf::Packet connectPacket;
    connectPacket << PacketType::CONNECT_REQUEST;
    connectPacket << snakeColor.r << snakeColor.g << snakeColor.b;
    connectPacket << snakeNickname;

    if (socket.send(connectPacket) != sf::Socket::Done) {
        std::cerr << "Failed to send connect request!" << std::endl;
        return false;
    }

    // Oczekiwanie na odpowiedź serwera
    sf::Clock timeout;
    while (timeout.getElapsedTime().asSeconds() < 5) {
        sf::Packet response;
        status = socket.receive(response);

        if (status == sf::Socket::Done) {
            PacketType responseType;
            response >> responseType;

            if (responseType == PacketType::CONNECT_ACCEPTED) {
                response >> localPlayerId;
                std::cout << "Connected to server! Your ID: " << localPlayerId << std::endl;
                isConnected = true;
                return true;
            }
        }

        sf::sleep(sf::milliseconds(10));
    }

    std::cerr << "Timed out waiting for server response!" << std::endl;
    return false;
}

// Wątek sieciowy do komunikacji z serwerem
void GameClient::networkLoop() {
    sf::Clock pingClock;
    sf::Int32 lastPingTime = 0;

    while (isRunning && isConnected) {
        // Wysyłanie pingów
        sf::Int32 currentTime = getCurrentTime();
        if (currentTime - lastPingTime > 1000) {
            sf::Packet pingPacket;
            pingPacket << PacketType::PING << currentTime;
            socket.send(pingPacket);
            lastPingTime = currentTime;
        }

        // Odbieranie danych z serwera
        sf::Packet packet;
        sf::Socket::Status status = socket.receive(packet);

        if (status == sf::Socket::Done) {
            PacketType packetType;
            packet >> packetType;

            switch (packetType) {
            case PacketType::GAME_STATE_FULL:
            case PacketType::GAME_STATE_UPDATE: {
                // Aktualizacja stanu gry
                std::lock_guard<std::mutex> lock(gameStateMutex);
                packet >> gameState;
                break;
            }
            case PacketType::PING: {
                // Odpowiedź na ping
                sf::Int32 serverTime;
                packet >> serverTime;
                sf::Int32 pingValue = getCurrentTime() - serverTime;
                break;
            }
            default:
                break;
            }
        }
        else if (status == sf::Socket::Disconnected) {
            std::cout << "Disconnected from the server." << std::endl;
            isConnected = false;
            break;
        }

        sf::sleep(sf::milliseconds(10));
    }
}

// Obsługa wejścia gracza
void GameClient::handleInput() {
    Snake* localSnake = nullptr;
    {
        std::lock_guard<std::mutex> lock(gameStateMutex);
        for (auto& snake : gameState.snakes) {
            if (snake.playerId == localPlayerId) {
                localSnake = &snake;
                break;
            }
        }
    }

    if (!localSnake || !localSnake->isAlive) return;

    // Pozycja myszy
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

    // Kąt między głową węża a pozycją myszy
    float snakeX, snakeY;
    {
        std::lock_guard<std::mutex> lock(gameStateMutex);
        if (localSnake->segments.empty()) return;
        snakeX = localSnake->segments[0].x;
        snakeY = localSnake->segments[0].y;
    }

    float dx = mousePos.x - snakeX;
    float dy = mousePos.y - snakeY;

    float targetAngle = std::atan2(dy, dx) * 180.0f / 3.14159f;

    // Normalizacja kąta
    while (targetAngle > 360.0f) targetAngle -= 360.0f;
    while (targetAngle < 0.0f) targetAngle += 360.0f;

    // Ograniczenie prędkości skrętu
    float currentAngle = localSnake->directionAngle;
    float angleDiff = targetAngle - currentAngle;

    while (angleDiff > 180.0f) angleDiff -= 360.0f;
    while (angleDiff < -180.0f) angleDiff += 360.0f;

    float maxTurn = 10.0f;
    float turnAmount = std::max(-maxTurn, std::min(maxTurn, angleDiff));

    float newAngle = currentAngle + turnAmount;
    while (newAngle > 360.0f) newAngle -= 360.0f;
    while (newAngle < 0.0f) newAngle += 360.0f;

    // Przyspieszenie
    bool isBoosting = sf::Mouse::isButtonPressed(sf::Mouse::Left);

    // Wysyłanie danych wejściowych do serwera
    sf::Packet inputPacket;
    inputPacket << PacketType::PLAYER_INPUT;
    inputPacket << PlayerInput(localPlayerId, newAngle, isBoosting, getCurrentTime());
    socket.send(inputPacket);
}

// Uruchamianie klienta, łączenie z serwerem
void GameClient::run(sf::String nickname, sf::Color color) {

    gameMusic.play();

    this->snakeNickname = nickname;
    this->snakeColor = color;

    if (!connectToServer()) {
        std::cerr << "Cannot connect to server!" << std::endl;
        return;
    }

    isRunning = true;
    networkThread = std::thread(&GameClient::networkLoop, this);

    clock.restart();

    // Pętla gry
    socket.setBlocking(false);
    while (window.isOpen() && isConnected) {
        // Obsługa zdarzeń
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                isRunning = false;
                window.close();
            }
        }

        float deltaTime = clock.restart().asSeconds();
        handleInput();

        // Rysowanie gry
        window.clear(sf::Color(20, 20, 20));

        window.draw(bgSprite);
        std::lock_guard<std::mutex> lock(gameStateMutex);

        for (const auto& food : gameState.foodItems) {
            drawFood(food);
        }

        for (const auto& snake : gameState.snakes) {
            drawSnake(snake);
        }

        drawHUD(localPlayerId);

        window.display();
    }

    isRunning = false;
    if (networkThread.joinable()) {
        networkThread.join();
    }

    // Wyświetlenie komunikatu o rozłączeniu
    if (window.isOpen() && !isConnected) {
        sf::Text disconnectMessage;
        disconnectMessage.setFont(font);
        disconnectMessage.setString("Lost connection to server");
        disconnectMessage.setCharacterSize(32);
        disconnectMessage.setFillColor(sf::Color::Red);
        disconnectMessage.setPosition(
            (window.getSize().x / 2.f) - (disconnectMessage.getGlobalBounds().width / 2.f),
            (window.getSize().y / 2.f) - (disconnectMessage.getGlobalBounds().height / 2.f)
        );

        // Wyświetlenie komunikatu
        window.clear(sf::Color(20, 20, 20));
        window.draw(disconnectMessage);
        window.display();

        // Krótkie oczekiwanie
        sf::sleep(sf::seconds(3));
    }

	gameMusic.stop();

}

// Dodatkowe okno dialogowe do wprowadzenia IP
bool GameClient::showConnectDialog() {
    sf::RenderWindow dialogWindow(
        sf::VideoMode(400, 200),
        "Enter the IP address of the server",
        sf::Style::Titlebar | sf::Style::Close
    );

    sf::Text ipText;
    ipText.setFont(font);
    ipText.setString("IP server address:");
    ipText.setCharacterSize(20);
    ipText.setFillColor(sf::Color::White);
    ipText.setPosition(20.f, 30.f);

    // Pole tekstowe
    sf::RectangleShape textBox(sf::Vector2f(360.f, 40.f));
    textBox.setPosition(20.f, 60.f);
    textBox.setFillColor(sf::Color(50, 50, 50));
    textBox.setOutlineThickness(2.f);
    textBox.setOutlineColor(sf::Color(100, 100, 100));

    // Tekst do wprowadzenia IP
    sf::Text inputText;
    inputText.setFont(font);
    inputText.setString("127.0.0.1");
    inputText.setCharacterSize(20);
    inputText.setFillColor(sf::Color::White);
    inputText.setPosition(30.f, 70.f);

    // Przycisk połączenia
    sf::RectangleShape connectButton(sf::Vector2f(150.f, 40.f));
    connectButton.setPosition(125.f, 130.f);
    connectButton.setFillColor(sf::Color(70, 70, 200));

    // Tekst przycisku
    sf::Text buttonText;
    buttonText.setFont(font);
    buttonText.setString("Connect");
    buttonText.setCharacterSize(20);
    buttonText.setFillColor(sf::Color::White);
    buttonText.setPosition(
        connectButton.getPosition().x + (connectButton.getSize().x / 2.f) - (buttonText.getGlobalBounds().width / 2.f),
        connectButton.getPosition().y + (connectButton.getSize().y / 2.f) - (buttonText.getGlobalBounds().height / 2.f) - 5.f
    );

    bool isActive = false;

    while (dialogWindow.isOpen()) {
        sf::Event event;
        while (dialogWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                dialogWindow.close();
                return false;
            }

            // Kliknięcie w pole tekstowe
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos = dialogWindow.mapPixelToCoords(sf::Mouse::getPosition(dialogWindow));
                isActive = textBox.getGlobalBounds().contains(mousePos);
            }

            // Wprowadzanie tekstu
            if (event.type == sf::Event::TextEntered && isActive) {
                if (event.text.unicode == 8) { // Backspace
                    std::string str = inputText.getString();
                    if (!str.empty()) {
                        str.pop_back();
                        inputText.setString(str);
                    }
                }
                else if (event.text.unicode >= 32 && event.text.unicode < 127) {
                    std::string str = inputText.getString();
                    str += static_cast<char>(event.text.unicode);
                    inputText.setString(str);
                }
            }

            // Kliknięcie przycisku
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2f mousePos = dialogWindow.mapPixelToCoords(sf::Mouse::getPosition(dialogWindow));
                if (connectButton.getGlobalBounds().contains(mousePos)) {
                    if (!inputText.getString().isEmpty()) {
                        serverIp = inputText.getString();
                        dialogWindow.close();
                        return true;
                    }
                }
            }

            // Zmiana koloru przycisku przy najechaniu
            if (event.type == sf::Event::MouseMoved) {
                sf::Vector2f mousePos = dialogWindow.mapPixelToCoords(sf::Mouse::getPosition(dialogWindow));
                if (connectButton.getGlobalBounds().contains(mousePos)) {
                    connectButton.setFillColor(sf::Color(100, 100, 220));
                }
                else {
                    connectButton.setFillColor(sf::Color(70, 70, 200));
                }
            }

            // Enter
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return) {
                if (!inputText.getString().isEmpty()) {
                    serverIp = inputText.getString();
                    dialogWindow.close();
                    return true;
                }
            }
        }

        // Renderowanie
        dialogWindow.clear(sf::Color(30, 30, 30));
        dialogWindow.draw(ipText);
        dialogWindow.draw(textBox);
        dialogWindow.draw(inputText);
        dialogWindow.draw(connectButton);
        dialogWindow.draw(buttonText);
        dialogWindow.display();
    }

    return false;
}