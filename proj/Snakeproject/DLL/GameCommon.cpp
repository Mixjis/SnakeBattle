#include "GameCommon.h"
#include <chrono>
#include <algorithm>

// Static member definitions
std::random_device Snake::massDropRd;
std::mt19937 Snake::massDropGen(Snake::massDropRd());

// Snake class implementation
Snake::Snake(int id)
    : playerId(id), directionAngle(0.f), speed(DEFAULT_SPEED), isBoosting(false),
    color(sf::Color::Red), isAlive(true), score(0), boostMassDropTimer(0.f)
{
    // Inicjalizacja początkowych segmentów
    float startX = DEFAULT_WINDOW_WIDTH / 2.f;
    float startY = DEFAULT_WINDOW_HEIGHT / 2.f;

    //początkowy kierunek
    directionAngle = rand() % 360;
    float radians = directionAngle * 3.14159f / 180.f;
    segments.push_back(SnakeSegment(startX, startY));

    for (int i = 1; i < SNAKE_INITIAL_SIZE; ++i) {
        float offsetX = -std::cos(radians) * SNAKE_SEGMENT_SIZE * i;
        float offsetY = -std::sin(radians) * SNAKE_SEGMENT_SIZE * i;

        segments.push_back(SnakeSegment(startX + offsetX, startY + offsetY));
    }
};

// Funkcja do tworzenia jedzenia podczas sprintu
FoodItem Snake::createFoodFromSegment(const SnakeSegment& segment, int foodId) {
    std::uniform_real_distribution<float> offsetDist(-5.f, 5.f);

    FoodItem food(foodId,
        segment.x + offsetDist(massDropGen),
        segment.y + offsetDist(massDropGen));

    food.color = sf::Color(
        static_cast<sf::Uint8>(color.r * 0.8f),
        static_cast<sf::Uint8>(color.g * 0.8f),
        static_cast<sf::Uint8>(color.b * 0.8f)
    );

    return food;
};

bool Snake::canBoost() const {
    return segments.size() >= MIN_SEGMENTS_FOR_BOOST;
};

// Funkcja do aktualizacji pozycji węża
void Snake::update(float deltaTime, std::vector<FoodItem>& foodItems) {
    if (!isAlive) return;
    if (isBoosting && !canBoost()) {
        isBoosting = false;
    }

    speed = (isBoosting ? BOOST_SPEED : DEFAULT_SPEED);

    //wyrzucanie masy podczas sprintu
    if (isBoosting && canBoost()) {
        boostMassDropTimer += deltaTime;

        if (boostMassDropTimer >= BOOST_MASS_DROP_INTERVAL) {
            boostMassDropTimer = 0.f;


            if (segments.size() > MIN_SEGMENTS_FOR_BOOST) {
                SnakeSegment droppedSegment = segments.back();
                segments.pop_back();

                int foodId = static_cast<int>(foodItems.size());
                FoodItem droppedFood = createFoodFromSegment(droppedSegment, foodId);
                foodItems.push_back(droppedFood);

                if (score > 0) {
                    score--;
                }
            }
        }
    }
    else {
        boostMassDropTimer = 0.f;
    }

    // Obliczenie ruchu
    float distance = speed * deltaTime;

    // Ruch głowy węża
    float radians = directionAngle * 3.14159f / 180.f;
    float dx = std::cos(radians) * distance;
    float dy = std::sin(radians) * distance;

    SnakeSegment oldHead = segments[0];

    float oldHeadX = segments[0].x;
    float oldHeadY = segments[0].y;

    segments[0].x += dx;
    segments[0].y += dy;

    const float segmentDistance = 5.0f;

    // Aktualizacja pozostałych segmentów
    for (size_t i = 1; i < segments.size(); ++i) {

        float dx = segments[i - 1].x - segments[i].x;
        float dy = segments[i - 1].y - segments[i].y;

        float segDist = std::sqrt(dx * dx + dy * dy);

        if (segDist > 0.1f) {
            float scale = 1.0f - (segmentDistance / segDist);
            segments[i].x += dx * scale;
            segments[i].y += dy * scale;
        }
    }
};

// Funkcja do dodawania nowego segmentu
void Snake::grow() {
    if (segments.size() < 2) return;

    SnakeSegment& tail = segments[segments.size() - 1];
    SnakeSegment& secondToLast = segments[segments.size() - 2];

    float dx = tail.x - secondToLast.x;
    float dy = tail.y - secondToLast.y;

    segments.push_back(SnakeSegment(tail.x + dx, tail.y + dy));
    score++;
};

// Sprawdzenie kolizji z jedzeniem
bool Snake::checkFoodCollision(const FoodItem& food) const {
    if (!isAlive || !food.isActive) return false;

    float dx = segments[0].x - food.x;
    float dy = segments[0].y - food.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    return distance < (SNAKE_SEGMENT_SIZE + FOOD_SIZE) / 2.f;
};

// Sprawdzenie kolizji z innym wężem
bool Snake::checkSnakeCollision(const Snake& other) const {
    if (!isAlive || !other.isAlive || this == &other) return false;

    for (size_t i = 0; i < other.segments.size(); ++i) {
        float dx = segments[0].x - other.segments[i].x;
        float dy = segments[0].y - other.segments[i].y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance < SNAKE_SEGMENT_SIZE * 0.8f) {
            return true;
        }
    }

    return false;
};

// Sprawdzenie kolizji węża z granicą
bool Snake::checkWallCollision() const {

    if (segments[0].x < 0 || segments[0].x > DEFAULT_WINDOW_WIDTH ||
        segments[0].y < 0 || segments[0].y > DEFAULT_WINDOW_HEIGHT) {
        return true;
    }
    return false;
};

// GameState class implementation
GameState::GameState() : gameTime(0) {
    if (!eatBuffer.loadFromFile("assets/eating.ogg")) {
        std::cerr << "Could not load eating.ogg!" << std::endl;
    }
    else {
        eatSound.setBuffer(eatBuffer);
    }
};

// Generowanie jedzenia
void GameState::generateFood(int count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(20.f, DEFAULT_WINDOW_WIDTH - 20.f);
    std::uniform_real_distribution<float> yDist(20.f, DEFAULT_WINDOW_HEIGHT - 20.f);

    for (int i = 0; i < count; ++i) {
        if (foodItems.size() >= MAX_FOOD_ITEMS) break;

        int id = static_cast<int>(foodItems.size());
        FoodItem food(id, xDist(gen), yDist(gen));
        food.color = sf::Color(
            50 + rand() % 200,
            50 + rand() % 200,
            50 + rand() % 200
        );

        foodItems.push_back(food);
    }
};

void GameState::generateFoodFromDeadSnake(const Snake& deadSnake) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> offsetDist(-8.f, 8.f);

    for (size_t i = 0; i < deadSnake.segments.size(); i += 2) {
        const auto& segment = deadSnake.segments[i];
        int id = static_cast<int>(foodItems.size());
        FoodItem food(id, segment.x + offsetDist(gen), segment.y + offsetDist(gen));

        food.color = sf::Color(
            std::min(255, static_cast<int>(deadSnake.color.r * 1.2f)),
            std::min(255, static_cast<int>(deadSnake.color.g * 1.2f)),
            std::min(255, static_cast<int>(deadSnake.color.b * 1.2f))
        );

        foodItems.push_back(food);
    }
};

// Sprawdzenie kolizji i aktualizacja stanu gry
void GameState::checkCollisions() {

    std::vector<Snake*> snakesToGenerateFood;

    // granica
    for (auto& snake : snakes) {
        if (!snake.isAlive) continue;

        if (snake.checkWallCollision()) {
            snake.isAlive = false;
            snakesToGenerateFood.push_back(&snake);
            continue;
        }
    }

    // zebranie jedzenia
    for (auto& snake : snakes) {
        if (!snake.isAlive) continue;

        for (auto& food : foodItems) {
            if (food.isActive && snake.checkFoodCollision(food)) {
                food.isActive = false;
                eatSound.play();
                snake.grow();
            }
        }
    }

    // stuknięcie w inne węże
    for (auto& snake : snakes) {
        if (!snake.isAlive) continue;

        for (auto& otherSnake : snakes) {
            if (snake.checkSnakeCollision(otherSnake)) {
                snake.isAlive = false;
                snakesToGenerateFood.push_back(&snake);
                break;
            }
        }
    }

    // przetworzenie jedzenia

    for (auto snakePtr : snakesToGenerateFood) {
        generateFoodFromDeadSnake(*snakePtr);
    }

    int eatenCount = 0;
    for (auto& food : foodItems) {
        if (!food.isActive) {
            eatenCount++;
        }
    }

    foodItems.erase(
        std::remove_if(foodItems.begin(), foodItems.end(),
            [](const FoodItem& food) { return !food.isActive; }),
        foodItems.end()
    );

    if (eatenCount > 0) {
        generateFood(eatenCount);
    }
};

// Aktualizacja stanu gry
void GameState::update(float deltaTime) {
    gameTime += static_cast<sf::Int32>(deltaTime * 1000);

    for (auto& snake : snakes) {
        snake.update(deltaTime, foodItems);
    }

    checkCollisions();

    if (foodItems.size() < MAX_FOOD_ITEMS / 2) {
        generateFood(5);
    }
};

// Przeciążenia operatorów dla sf::Packet do serializacji struktur danych

sf::Packet& operator <<(sf::Packet& packet, const PacketType& type) {
    return packet << static_cast<sf::Int32>(type);
}

sf::Packet& operator >>(sf::Packet& packet, PacketType& type) {
    sf::Int32 temp;
    packet >> temp;
    type = static_cast<PacketType>(temp);
    return packet;
};

//SnakeSegment

sf::Packet& operator <<(sf::Packet& packet, const SnakeSegment& segment) {
    return packet << segment.x << segment.y;
};

sf::Packet& operator >>(sf::Packet& packet, SnakeSegment& segment) {
    return packet >> segment.x >> segment.y;
};

//FoodItem

sf::Packet& operator <<(sf::Packet& packet, const FoodItem& food) {
    return packet << food.id << food.x << food.y << food.isActive
        << food.color.r << food.color.g << food.color.b;
};

sf::Packet& operator >>(sf::Packet& packet, FoodItem& food) {
    packet >> food.id >> food.x >> food.y >> food.isActive;
    sf::Uint8 r, g, b;
    packet >> r >> g >> b;
    food.color = sf::Color(r, g, b);
    return packet;
};

//Snake

sf::Packet& operator <<(sf::Packet& packet, const Snake& snake) {
    packet << snake.playerId << snake.directionAngle << snake.speed
        << snake.isBoosting << snake.isAlive << snake.score
        << snake.color.r << snake.color.g << snake.color.b << snake.snakeNickname
        << snake.boostMassDropTimer;

    sf::Uint32 segmentCount = static_cast<sf::Uint32>(snake.segments.size());
    packet << segmentCount;

    for (const auto& segment : snake.segments) {
        packet << segment;
    }

    return packet;
};

sf::Packet& operator >>(sf::Packet& packet, Snake& snake) {
    packet >> snake.playerId >> snake.directionAngle >> snake.speed
        >> snake.isBoosting >> snake.isAlive >> snake.score;

    sf::Uint8 r, g, b;
    packet >> r >> g >> b;
    snake.color = sf::Color(r, g, b);

    packet >> snake.snakeNickname;
    packet >> snake.boostMassDropTimer;

    sf::Uint32 segmentCount;
    packet >> segmentCount;

    snake.segments.resize(segmentCount);
    for (sf::Uint32 i = 0; i < segmentCount; ++i) {
        packet >> snake.segments[i];
    }

    return packet;
};

// Gamestate

sf::Packet& operator <<(sf::Packet& packet, const GameState& state) {
    packet << state.gameTime;

    sf::Uint32 snakeCount = static_cast<sf::Uint32>(state.snakes.size());
    packet << snakeCount;

    for (const auto& snake : state.snakes) {
        packet << snake;
    }

    sf::Uint32 foodCount = static_cast<sf::Uint32>(state.foodItems.size());
    packet << foodCount;

    for (const auto& food : state.foodItems) {
        packet << food;
    }

    return packet;
};

sf::Packet& operator >>(sf::Packet& packet, GameState& state) {
    packet >> state.gameTime;

    sf::Uint32 snakeCount;
    packet >> snakeCount;

    state.snakes.resize(snakeCount);
    for (sf::Uint32 i = 0; i < snakeCount; ++i) {
        packet >> state.snakes[i];
    }

    sf::Uint32 foodCount;
    packet >> foodCount;

    state.foodItems.resize(foodCount);
    for (sf::Uint32 i = 0; i < foodCount; ++i) {
        packet >> state.foodItems[i];
    }

    return packet;
};

//PlayerInput

sf::Packet& operator <<(sf::Packet& packet, const PlayerInput& input) {
    return packet << input.playerId << input.directionAngle << input.isBoosting << input.inputTime;
};

sf::Packet& operator >>(sf::Packet& packet, PlayerInput& input) {
    return packet >> input.playerId >> input.directionAngle >> input.isBoosting >> input.inputTime;
};

// Funkcje pomocnicze

float distance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
};

sf::Int32 getCurrentTime() {
    return static_cast<sf::Int32>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
        );
};