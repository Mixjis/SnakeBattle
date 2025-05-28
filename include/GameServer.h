#pragma once

#ifndef GAMESERVER_H
#define GAMESERVER_H


#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>
#include <random>

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#include "GameCommon.h"
#include "GameBase.h"
#include "API.h"

// Klasa serwera gry (host)
class EXPORT_API GameServer : public GameBase {
private:
    sf::TcpListener listener;
    std::vector<std::unique_ptr<sf::TcpSocket>> clientSockets;
    std::map<sf::TcpSocket*, int> socketToPlayerId;

    std::thread networkThread;
    std::atomic<bool> isRunning;
    std::mutex gameStateMutex;

    int nextPlayerId;
    sf::Int32 lastFullUpdateTime;

    // Inicjalizacja serwera
    bool initServer();

    // Inicjalizacja hosta
    void initLocalPlayer();

    // Wątek sieciowy do obsługi klientów
    void networkLoop();

    // Wysyłanie aktualizacji stanu gry do wszystkich klientów
    void sendUpdateToAll();

    void sendFullStateToAll();

    // Wysyłanie pełnego stanu gry do konkretnego klienta
    void sendFullState(sf::TcpSocket& socket);

    // Obsługa wejścia lokalnego gracza (host)
    void handleLocalInput();

public:
    GameServer();

    GameServer(sf::RenderWindow& existingWindow);

    ~GameServer();

    void run(sf::String nickname, sf::Color color) override;
};

#endif // GAMESERVER_H