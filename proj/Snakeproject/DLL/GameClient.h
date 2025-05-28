#pragma once

#ifndef GAMECLIENT_H
#define GAMECLIENT_H


#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#include "GameBase.h"
#include "GameCommon.h"
#include "API.h"

// Klasa klienta gry
class EXPORT_API GameClient : public GameBase {
private:
    sf::TcpSocket socket;
    std::thread networkThread;
    std::atomic<bool> isRunning;
    std::atomic<bool> isConnected;
    std::mutex gameStateMutex;

    int localPlayerId;
    std::string serverIp;

    // Prywatne metody
    bool connectToServer();
    void networkLoop();
    void handleInput();

public:
    // Konstruktory
    GameClient(sf::RenderWindow& existingWindow);
    GameClient(sf::RenderWindow& existingWindow, const std::string& ip);

    // Destruktor
    ~GameClient();

    // Publiczne metody
    void setServerIp(const std::string& ip);
    void run(sf::String nickname, sf::Color color) override;
    bool showConnectDialog();
};

#endif // GAMECLIENT_H