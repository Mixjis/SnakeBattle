#include "GameBase.h"
#include "GameCommon.h"
#include <iostream>

// Funkcja do rysowania węża
void GameBase::drawSnake(const Snake& snake) {
    if (!snake.segments.empty() && snake.isAlive) {

        // SEGMENTY
        for (size_t i = snake.segments.size(); i-- > 1;) {
            sf::CircleShape baseLayer(SNAKE_SEGMENT_SIZE / 2.f * 0.9f);
            baseLayer.setOrigin(baseLayer.getRadius(), baseLayer.getRadius());
            baseLayer.setPosition(snake.segments[i].x, snake.segments[i].y);

            sf::Color baseColor(
                static_cast<sf::Uint8>(snake.color.r * 0.4f),
                static_cast<sf::Uint8>(snake.color.g * 0.4f),
                static_cast<sf::Uint8>(snake.color.b * 0.4f)
            );
            baseLayer.setFillColor(baseColor);

            sf::CircleShape centerLayer(SNAKE_SEGMENT_SIZE / 2.f * 0.7f);
            centerLayer.setOrigin(centerLayer.getRadius(), centerLayer.getRadius());
            centerLayer.setPosition(snake.segments[i].x, snake.segments[i].y);

            sf::Color centerColor(
                static_cast<sf::Uint8>(std::min(255.f, snake.color.r * 1.2f)),
                static_cast<sf::Uint8>(std::min(255.f, snake.color.g * 1.2f)),
                static_cast<sf::Uint8>(std::min(255.f, snake.color.b * 1.2f))
            );
            centerLayer.setFillColor(centerColor);

            if (snake.isBoosting) {
                baseLayer.setOutlineThickness(1.f);
                baseLayer.setOutlineColor(sf::Color(255, 255, 255, 100));
            }
            else {
                baseLayer.setOutlineThickness(1.f);
                baseLayer.setOutlineColor(sf::Color(0, 0, 0, 64));
            }
            window.draw(baseLayer);
            window.draw(centerLayer);
        }

        // GŁOWA 
        sf::CircleShape headBase(SNAKE_SEGMENT_SIZE / 2.f);
        headBase.setFillColor(sf::Color(
            static_cast<sf::Uint8>(snake.color.r * 0.5f),
            static_cast<sf::Uint8>(snake.color.g * 0.5f),
            static_cast<sf::Uint8>(snake.color.b * 0.5f)
        ));
        headBase.setOrigin(headBase.getRadius(), headBase.getRadius());
        headBase.setPosition(snake.segments[0].x, snake.segments[0].y);

        sf::CircleShape headCenter(SNAKE_SEGMENT_SIZE / 2.f * 0.8f);
        headCenter.setFillColor(sf::Color(
            static_cast<sf::Uint8>(std::min(255.f, snake.color.r * 1.1f)),
            static_cast<sf::Uint8>(std::min(255.f, snake.color.g * 1.1f)),
            static_cast<sf::Uint8>(std::min(255.f, snake.color.b * 1.1f))
        ));
        headCenter.setOrigin(headCenter.getRadius(), headCenter.getRadius());
        headCenter.setPosition(snake.segments[0].x, snake.segments[0].y);

        if (snake.isBoosting) {
            headBase.setOutlineThickness(1.f);
            headBase.setOutlineColor(sf::Color(255, 255, 255, 100));
        }
        else {
            headBase.setOutlineThickness(2.f);
            headBase.setOutlineColor(sf::Color(0, 0, 0, 64));
        }
        window.draw(headBase);
        window.draw(headCenter);

        // OCZY
        float radians = snake.directionAngle * 3.14159f / 180.f;
        float eyeOffsetX = std::cos(radians) * (SNAKE_SEGMENT_SIZE / 4.f);
        float eyeOffsetY = std::sin(radians) * (SNAKE_SEGMENT_SIZE / 4.f);
        float eyeSeparation = SNAKE_SEGMENT_SIZE / 4.f;

        sf::Vector2f headPos(snake.segments[0].x, snake.segments[0].y);

        sf::CircleShape leftEyeWhite(SNAKE_SEGMENT_SIZE / 5.f);
        leftEyeWhite.setFillColor(sf::Color::White);
        leftEyeWhite.setOrigin(leftEyeWhite.getRadius(), leftEyeWhite.getRadius());
        leftEyeWhite.setPosition(
            headPos.x + eyeOffsetX - std::sin(radians) * eyeSeparation,
            headPos.y + eyeOffsetY + std::cos(radians) * eyeSeparation
        );

        sf::CircleShape rightEyeWhite = leftEyeWhite;
        rightEyeWhite.setPosition(
            headPos.x + eyeOffsetX + std::sin(radians) * eyeSeparation,
            headPos.y + eyeOffsetY - std::cos(radians) * eyeSeparation
        );

        float pupilSize = SNAKE_SEGMENT_SIZE / 10.f;
        sf::CircleShape leftPupil(pupilSize);
        leftPupil.setFillColor(sf::Color::Black);
        leftPupil.setOrigin(pupilSize, pupilSize);
        leftPupil.setPosition(leftEyeWhite.getPosition().x + std::cos(radians) * 2,
            leftEyeWhite.getPosition().y + std::sin(radians) * 2);

        sf::CircleShape rightPupil = leftPupil;
        rightPupil.setPosition(rightEyeWhite.getPosition().x + std::cos(radians) * 2,
            rightEyeWhite.getPosition().y + std::sin(radians) * 2);

        window.draw(leftEyeWhite);
        window.draw(rightEyeWhite);
        window.draw(leftPupil);
        window.draw(rightPupil);
    }

    // NICKNAME
    if (!snake.snakeNickname.isEmpty() && snake.isAlive) {
        sf::Text nickText;
        nickText.setFont(font);
        nickText.setString(snake.snakeNickname);
        nickText.setCharacterSize(14);
        nickText.setFillColor(sf::Color::White);
        nickText.setOutlineColor(sf::Color::Black);
        nickText.setOutlineThickness(1.0f);

        sf::FloatRect textBounds = nickText.getLocalBounds();
        nickText.setOrigin(textBounds.width / 2.0f, textBounds.height + 10.0f);
        nickText.setPosition(snake.segments[0].x, snake.segments[0].y);

        window.draw(nickText);
    }
}

// rysowanie jedzenia
void GameBase::drawFood(const FoodItem& food) {
    if (food.isActive) {
        sf::CircleShape foodShape(FOOD_SIZE / 2.f);
        foodShape.setFillColor(food.color);
        foodShape.setOrigin(FOOD_SIZE / 2.f, FOOD_SIZE / 2.f);
        foodShape.setPosition(food.x, food.y);
        window.draw(foodShape);
    }
}

// rysowanie HUD
void GameBase::drawHUD(int localPlayerId) {
    const Snake* localSnake = nullptr;
    for (const auto& snake : gameState.snakes) {
        if (snake.playerId == localPlayerId) {
            localSnake = &snake;
            break;
        }
    }

    if (localSnake) {
        std::string scoreStr = "Score: " + std::to_string(localSnake->score);
        scoreText.setString(scoreStr);

        std::string status = localSnake->isAlive ? "" : "Spectating";
        statusText.setString(status);
    }
    else {
        scoreText.setString("Score: 0");
        statusText.setString("Spectating");
    }


    sf::RectangleShape back(sf::Vector2f(200.f, 250.f));
    back.setPosition(0.f, 0.f); 
    back.setFillColor(sf::Color(0, 0, 0,80));
    back.setOutlineThickness(3.f); // Border size
    back.setOutlineColor(sf::Color(64, 224, 208,70));

    window.draw(back);

    // Rysowanie tekstu
    window.draw(scoreText);
    window.draw(statusText);

    // Rysowanie tabeli wyników (top 5)
    std::vector<const Snake*> sortedSnakes;
    for (const auto& snake : gameState.snakes) {
        sortedSnakes.push_back(&snake);
    }

    // Sortowanie węży według wyniku
    std::sort(sortedSnakes.begin(), sortedSnakes.end(),
        [](const Snake* a, const Snake* b) { return a->score > b->score; });

    // Wyświetlanie top 5
    sf::Text leaderboardText;
    leaderboardText.setFont(font);
    leaderboardText.setCharacterSize(16);
    leaderboardText.setFillColor(sf::Color::White);
    leaderboardText.setPosition(10.f, 70.f);

    std::string leaderboard = "Leaderboard:\n";
    int count = 0;
    for (const auto* snake : sortedSnakes) {
        leaderboard += snake->snakeNickname + ": " +
            std::to_string(snake->score) + "" +
            (snake->isAlive ? "" : " (Spectating)") + "\n";

        if (++count >= 5) break;
    }

    leaderboardText.setString(leaderboard);
    window.draw(leaderboardText);
}

// Inicjalizacja tekstu
void GameBase::initText() {
    if (!font.loadFromFile("assets/Arimo.ttf")) {
        std::cerr << "Could not load font!" << std::endl;
        exit(1);
    }

    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10.f, 10.f);

    statusText.setFont(font);
    statusText.setCharacterSize(20);
    statusText.setFillColor(sf::Color::White);
    statusText.setPosition(10.f, 40.f);


}

// Konstruktory
GameBase::GameBase() : window(*new sf::RenderWindow()), lastUpdateTime(0) {
    window.create(sf::VideoMode(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT), "Snake Battle");
    window.setFramerateLimit(60);
    initText();

}

GameBase::GameBase(sf::RenderWindow& existingWindow) : window(existingWindow), lastUpdateTime(0) {
    window.setFramerateLimit(60);
    initText();

    if (!backgroundImage.loadFromFile("assets/hexagon.png")) {
        std::cerr << "Cannot load image !" << std::endl;
        exit(1);
    }
    
    bgSprite.setTexture(backgroundImage);

    if (!font.loadFromFile("assets/Arimo.ttf")) {
        std::cerr << "Cannot load font!" << std::endl;
        exit(1);
    }

    if (!gameMusic.openFromFile("assets/bgmusic2.mp3")) {
        std::cerr << " Cannot load music file!" << std::endl;
        exit(1);
    }

    gameMusic.setLoop(true);
}