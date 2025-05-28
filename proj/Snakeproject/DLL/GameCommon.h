#pragma once

#ifndef GAMECOMMON_H
#define GAMECOMMON_H

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <random>

constexpr float SNAKE_SEGMENT_SIZE = 20.f;
constexpr float SNAKE_INITIAL_SIZE = 5;
constexpr float DEFAULT_SPEED = 100.0f;
constexpr float BOOST_SPEED = 200.0f;
constexpr float SPEED_DECREASE_RATE = 0.01f;
constexpr int MAX_FOOD_ITEMS = 100;
constexpr float FOOD_SIZE = 10.f;
constexpr int DEFAULT_WINDOW_WIDTH = 1200;
constexpr int DEFAULT_WINDOW_HEIGHT = 800;
constexpr float DIRECTION_CHANGE_SPEED = 0.1f;
constexpr float BOOST_MASS_DROP_INTERVAL = 0.6f;
constexpr int MIN_SEGMENTS_FOR_BOOST = 6;

// Stałe sieciowe
constexpr unsigned short DEFAULT_PORT = 53000;
constexpr sf::Int32 NETWORK_UPDATE_INTERVAL = 30; // ms
constexpr sf::Int32 FULL_STATE_UPDATE_INTERVAL = 1000; // ms

// Typy pakietów
enum class PacketType {
    CONNECT_REQUEST = 1,
    CONNECT_ACCEPTED = 2,
    GAME_STATE_FULL = 3,
    GAME_STATE_UPDATE = 4,
    PLAYER_INPUT = 5,
    DISCONNECT = 6,
    PING = 7
};

//segment węża
struct SnakeSegment {
    float x, y;

    SnakeSegment(float x = 0.f, float y = 0.f) : x(x), y(y) {}
};

//punkt jedzenia
struct FoodItem {
    int id;
    float x, y;
    bool isActive;
    sf::Color color;

    FoodItem(int id = 0, float x = 0.f, float y = 0.f)
        : id(id), x(x), y(y), isActive(true), color(sf::Color::Green) {
    }
};

//Wąż gracza
struct Snake {
    float prevDirectionAngle = 0.f;
    float angleInterpolationAlpha = 1.0f;

    int playerId;
    sf::String snakeNickname;
    std::vector<SnakeSegment> segments;
    float directionAngle;
    float speed;
    bool isBoosting;
    sf::Color color;
    bool isAlive;
    int score;

    float boostMassDropTimer;
    static std::random_device massDropRd;
    static std::mt19937 massDropGen;

    Snake(int id = 0);

    // Funkcja do tworzenia jedzenia podczas sprintu
    FoodItem createFoodFromSegment(const SnakeSegment& segment, int foodId);

    bool canBoost() const;

    // Funkcja do aktualizacji pozycji węża
    void update(float deltaTime, std::vector<FoodItem>& foodItems);

    // Funkcja do dodawania nowego segmentu
    void grow();

    // Sprawdzenie kolizji z jedzeniem
    bool checkFoodCollision(const FoodItem& food) const;

    // Sprawdzenie kolizji z innym wężem
    bool checkSnakeCollision(const Snake& other) const;

    // Sprawdzenie kolizji węża z granicą
    bool checkWallCollision() const;
};

//Stan gry
struct GameState {
    std::vector<Snake> snakes;
    std::vector<FoodItem> foodItems;
    sf::Int32 gameTime;

    sf::SoundBuffer eatBuffer;
    sf::Sound eatSound;

    GameState();

    // Generowanie jedzenia
    void generateFood(int count);

    void generateFoodFromDeadSnake(const Snake& deadSnake);

    // Sprawdzenie kolizji i aktualizacja stanu gry
    void checkCollisions();

    // Aktualizacja stanu gry
    void update(float deltaTime);
};

// Dane wprowadzone przez gracza
struct PlayerInput {
    int playerId;
    float directionAngle;
    bool isBoosting;
    sf::Int32 inputTime;

    PlayerInput(int id = 0, float angle = 0.f, bool boost = false, sf::Int32 time = 0)
        : playerId(id), directionAngle(angle), isBoosting(boost), inputTime(time) {
    }
};

// Przeciążenia operatorów dla sf::Packet do serializacji struktur danych

sf::Packet& operator <<(sf::Packet& packet, const PacketType& type);
sf::Packet& operator >>(sf::Packet& packet, PacketType& type);

//SnakeSegment
sf::Packet& operator <<(sf::Packet& packet, const SnakeSegment& segment);
sf::Packet& operator >>(sf::Packet& packet, SnakeSegment& segment);

//FoodItem
sf::Packet& operator <<(sf::Packet& packet, const FoodItem& food);
sf::Packet& operator >>(sf::Packet& packet, FoodItem& food);

//Snake
sf::Packet& operator <<(sf::Packet& packet, const Snake& snake);
sf::Packet& operator >>(sf::Packet& packet, Snake& snake);

// Gamestate
sf::Packet& operator <<(sf::Packet& packet, const GameState& state);
sf::Packet& operator >>(sf::Packet& packet, GameState& state);

//PlayerInput
sf::Packet& operator <<(sf::Packet& packet, const PlayerInput& input);
sf::Packet& operator >>(sf::Packet& packet, PlayerInput& input);

// Funkcje pomocnicze
float distance(float x1, float y1, float x2, float y2);
sf::Int32 getCurrentTime();

#endif // GAMECOMMON_H