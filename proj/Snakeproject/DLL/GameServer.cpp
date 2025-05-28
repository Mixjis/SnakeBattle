#include "GameServer.h"

// Inicjalizacja serwera
bool GameServer::initServer() {
    // Ustawianie portu nasłuchiwania
    if (listener.listen(DEFAULT_PORT) != sf::Socket::Done) {
        std::cerr << "Cannot listen on port " << DEFAULT_PORT << std::endl;
        return false;
    }

    listener.setBlocking(false);

    std::cout << "Server running on port " << DEFAULT_PORT << std::endl;
    std::cout << "Waiting for players..." << std::endl;

    // ustawienie początkowego stanu gry
    nextPlayerId = 1;
    initLocalPlayer();
    gameState.generateFood(20);

    return true;
};

// Inicjalizacja hosta
void GameServer::initLocalPlayer() {
    Snake localPlayer(0);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(100.f, DEFAULT_WINDOW_WIDTH - 100.f);
    std::uniform_real_distribution<float> yDist(100.f, DEFAULT_WINDOW_HEIGHT - 100.f);

    localPlayer.segments[0] = SnakeSegment(xDist(gen), yDist(gen));
    for (size_t i = 1; i < localPlayer.segments.size(); ++i) {
        localPlayer.segments[i] = SnakeSegment(
            localPlayer.segments[i - 1].x - SNAKE_SEGMENT_SIZE,
            localPlayer.segments[i - 1].y
        );
    }
    std::uniform_real_distribution<float> angleDist(0.f, 360.f);
    localPlayer.directionAngle = angleDist(gen);
    localPlayer.snakeNickname = snakeNickname;
    localPlayer.color = snakeColor;

    gameState.snakes.push_back(localPlayer);
};

// Wątek sieciowy do obsługi klientów
void GameServer::networkLoop() {
    sf::SocketSelector selector;
    selector.add(listener);

    for (auto& clientSocket : clientSockets) {
        selector.add(*clientSocket);
    }

    while (isRunning) {
        // Czekanie na aktywność na socketach
        if (selector.wait(sf::milliseconds(50))) {
            if (selector.isReady(listener)) {
                auto client = std::make_unique<sf::TcpSocket>();
                if (listener.accept(*client) == sf::Socket::Done) {

                    // Ustawienie nowego węża dla gracza
                    int playerId = nextPlayerId++;
                    socketToPlayerId[client.get()] = playerId;

                    Snake newPlayer(playerId);

                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_real_distribution<float> xDist(100.f, DEFAULT_WINDOW_WIDTH - 100.f);
                    std::uniform_real_distribution<float> yDist(100.f, DEFAULT_WINDOW_HEIGHT - 100.f);
                    std::uniform_real_distribution<float> angleDist(0.f, 360.f);

                    newPlayer.segments[0] = SnakeSegment(xDist(gen), yDist(gen));
                    for (size_t i = 1; i < newPlayer.segments.size(); ++i) {
                        newPlayer.segments[i] = SnakeSegment(
                            newPlayer.segments[i - 1].x - SNAKE_SEGMENT_SIZE,
                            newPlayer.segments[i - 1].y
                        );
                    }

                    newPlayer.directionAngle = angleDist(gen);

                    sf::Packet connectPacket;
                    if (client->receive(connectPacket) == sf::Socket::Done) {
                        PacketType type;
                        sf::Uint8 r, g, b;
                        sf::String nickname;
                        connectPacket >> type >> r >> g >> b >> nickname;
                        if (type == PacketType::CONNECT_REQUEST) {
                            newPlayer.color = sf::Color(r, g, b);
                            newPlayer.snakeNickname = nickname;
                        }
                    }

                    // Dodanie gracza do stanu gry
                    {
                        std::lock_guard<std::mutex> lock(gameStateMutex);
                        gameState.snakes.push_back(newPlayer);
                    }

                    sf::Packet acceptPacket;
                    acceptPacket << PacketType::CONNECT_ACCEPTED << playerId;
                    client->send(acceptPacket);

                    sendFullState(*client);

                    selector.add(*client);
                    clientSockets.push_back(std::move(client));

                    std::cout << "New player connected! ID: " << playerId << newPlayer.snakeNickname.toAnsiString() << std::endl;
                }
            }

            // Obsługa pakietów od klientów
            for (auto it = clientSockets.begin(); it != clientSockets.end();) {
                sf::TcpSocket& socket = **it;

                if (selector.isReady(socket)) {
                    sf::Packet packet;
                    sf::Socket::Status status = socket.receive(packet);

                    if (status == sf::Socket::Done) {
                        PacketType packetType;
                        packet >> packetType;

                        switch (packetType) {
                        case PacketType::PLAYER_INPUT: {
                            // Odbieranie wejścia od gracza
                            PlayerInput input;
                            packet >> input;


                            std::lock_guard<std::mutex> lock(gameStateMutex);
                            for (auto& snake : gameState.snakes) {
                                if (snake.playerId == input.playerId) {
                                    snake.directionAngle = input.directionAngle;
                                    snake.isBoosting = input.isBoosting;
                                    break;
                                }
                            }
                            break;
                        }
                        case PacketType::PING: {
                            // Odpowiedź na ping
                            sf::Packet pongPacket;
                            pongPacket << PacketType::PING << getCurrentTime();
                            socket.send(pongPacket);
                            break;
                        }
                        case PacketType::DISCONNECT: {
                            // rozłączenie, usunięcie gracza
                            int playerId = socketToPlayerId[&socket];
                            std::cout << "player " << playerId << " disconnected." << std::endl;


                            std::lock_guard<std::mutex> lock(gameStateMutex);
                            gameState.snakes.erase(
                                std::remove_if(gameState.snakes.begin(), gameState.snakes.end(),
                                    [playerId](const Snake& snake) { return snake.playerId == playerId; }),
                                gameState.snakes.end()
                            );

                            selector.remove(socket);
                            socketToPlayerId.erase(&socket);
                            it = clientSockets.erase(it);
                            continue;
                        }
                        default:
                            break;
                        }
                    }
                    else if (status == sf::Socket::Disconnected) {
                        // rozłączenie klienta
                        int playerId = socketToPlayerId[&socket];
                        std::cout << "player " << playerId << " disconnected. " << std::endl;

                        // Usunięcie gracza
                        std::lock_guard<std::mutex> lock(gameStateMutex);
                        gameState.snakes.erase(
                            std::remove_if(gameState.snakes.begin(), gameState.snakes.end(),
                                [playerId](const Snake& snake) { return snake.playerId == playerId; }),
                            gameState.snakes.end()
                        );

                        selector.remove(socket);
                        socketToPlayerId.erase(&socket);
                        it = clientSockets.erase(it);
                        continue;
                    }
                }

                ++it;
            }
        }

        // Wysyłanie aktualizacji stanu gry do wszystkich klientów
        sf::Int32 currentTime = getCurrentTime();
        if (currentTime - lastUpdateTime >= NETWORK_UPDATE_INTERVAL) {
            sendUpdateToAll();
            lastUpdateTime = currentTime;
        }

        if (currentTime - lastFullUpdateTime >= FULL_STATE_UPDATE_INTERVAL) {
            sendFullStateToAll();
            lastFullUpdateTime = currentTime;
        }
    }
};

// Wysyłanie aktualizacji stanu gry do wszystkich klientów
void GameServer::sendUpdateToAll() {
    std::lock_guard<std::mutex> lock(gameStateMutex);

    sf::Packet updatePacket;
    updatePacket << PacketType::GAME_STATE_UPDATE << gameState;

    for (auto& client : clientSockets) {
        client->send(updatePacket);
    }
};

void GameServer::sendFullStateToAll() {
    for (auto& client : clientSockets) {
        sendFullState(*client);
    }
};

// Wysyłanie pełnego stanu gry do konkretnego klienta
void GameServer::sendFullState(sf::TcpSocket& socket) {
    std::lock_guard<std::mutex> lock(gameStateMutex);

    sf::Packet fullStatePacket;
    fullStatePacket << PacketType::GAME_STATE_FULL << gameState;

    socket.send(fullStatePacket);
};

// Obsługa wejścia lokalnego gracza (host)
void GameServer::handleLocalInput() {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);

    Snake& localSnake = gameState.snakes[0];

    if (!localSnake.isAlive) return;

    float snakeX = localSnake.segments[0].x;
    float snakeY = localSnake.segments[0].y;

    float dx = mousePos.x - snakeX;
    float dy = mousePos.y - snakeY;

    float targetAngle = std::atan2(dy, dx) * 180.0f / 3.14159f;
    float angleDiff = targetAngle - localSnake.directionAngle;

    while (angleDiff > 180.0f) angleDiff -= 360.0f;
    while (angleDiff < -180.0f) angleDiff += 360.0f;

    float maxTurn = 10.0f;
    float turnAmount = std::max(-maxTurn, std::min(maxTurn, angleDiff));

    localSnake.directionAngle += turnAmount;

    while (localSnake.directionAngle > 360.0f) localSnake.directionAngle -= 360.0f;
    while (localSnake.directionAngle < 0.0f) localSnake.directionAngle += 360.0f;
    localSnake.isBoosting = sf::Mouse::isButtonPressed(sf::Mouse::Left);
};

GameServer::GameServer() : isRunning(false), nextPlayerId(1), lastFullUpdateTime(0) {
    window.create(sf::VideoMode(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT), "Snake Battle LAN - Host");
    window.setFramerateLimit(60);
};

GameServer::GameServer(sf::RenderWindow& existingWindow)
    : GameBase(existingWindow), isRunning(false), nextPlayerId(1), lastFullUpdateTime(0) {
    window.setTitle("Snake Battle");
};

GameServer::~GameServer() {
    // Zatrzymanie wątku sieciowego
    isRunning = false;
    if (networkThread.joinable()) {
        networkThread.join();
    }

    // Zamknięcie wszystkich socketów
    for (auto& socket : clientSockets) {
        socket->disconnect();
    }

    listener.close();
};

void GameServer::run(sf::String nickname, sf::Color color) {

	gameMusic.play();

    snakeNickname = nickname;
    snakeColor = color;

    if (!initServer()) {
        std::cerr << "Could not initialize server!" << std::endl;
        return;
    }

    // Uruchomienie wątku sieciowego
    isRunning = true;
    networkThread = std::thread(&GameServer::networkLoop, this);

    clock.restart();
    lastUpdateTime = getCurrentTime();
    lastFullUpdateTime = getCurrentTime();

    // Główna pętla gry
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                isRunning = false;
                window.close();
            }
        }

        float deltaTime = clock.restart().asSeconds();
        handleLocalInput();

        // Aktualizacja stanu gry
        {
            std::lock_guard<std::mutex> lock(gameStateMutex);
            gameState.update(deltaTime);
        }

        // Renderowanie
        window.clear(sf::Color(20, 20, 20));
        window.draw(bgSprite);
        std::lock_guard<std::mutex> lock(gameStateMutex);
        for (const auto& food : gameState.foodItems) {
            drawFood(food);
        }

        for (const auto& snake : gameState.snakes) {
            drawSnake(snake);
        }

        drawHUD(0);
        window.display();
    }


    isRunning = false;
    if (networkThread.joinable()) {
        networkThread.join();
    }

	gameMusic.stop();
};