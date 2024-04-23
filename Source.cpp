#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>

class GameObject {

public:
    GameObject(sf::Shape* objectShape) : shape(objectShape) {}
    sf::Shape* shape;
    virtual void draw(sf::RenderTarget& target) = 0;

    sf::Vector2f getPosition() const {
        return shape->getPosition();
    }

    sf::Vector2f getSize() const {
        if (dynamic_cast<sf::RectangleShape*>(shape))
            return dynamic_cast<sf::RectangleShape*>(shape)->getSize();
        else if (dynamic_cast<sf::CircleShape*>(shape)) {
            float radius = dynamic_cast<sf::CircleShape*>(shape)->getRadius();
            return sf::Vector2f(radius * 2, radius * 2);
        }
        return sf::Vector2f();
    }
};


class Paddle : public GameObject {
private:
    float speed;

public:
    Paddle();
    Paddle(float width, float height, float paddleSpeed)
        : GameObject(new sf::RectangleShape(sf::Vector2f(width, height))), speed(paddleSpeed) {}

    void setPosition(float x, float y) {
        shape->setPosition(x, y);
    }

    void moveUp() {
        if (shape->getPosition().y > 0)
            shape->move(0, -speed);
    }

    void moveDown(float screenHeight) {
        if (shape->getPosition().y + getSize().y < screenHeight)
            shape->move(0, speed);
    }

    void draw(sf::RenderTarget& target){
        target.draw(*shape);
    }
};

class Ball : public GameObject {
private:
    sf::Vector2f velocity;

public:
    Ball();
    Ball(float radius, const sf::Vector2f& ballVelocity)
        : GameObject(new sf::CircleShape(radius)), velocity(ballVelocity) {
        dynamic_cast<sf::CircleShape*>(shape)->setRadius(radius);
        dynamic_cast<sf::CircleShape*>(shape)->setOrigin(radius, radius);
    }

    void setPosition(float x, float y) {
        shape->setPosition(x, y);
    }

    void update() {
        shape->move(velocity);
    }

    void reverseX() {
        velocity.x = -velocity.x;
    }

    void reverseY() {
        velocity.y = -velocity.y;
    }

    sf::Vector2f getVelocity() const {
        return velocity;
    }

    void draw(sf::RenderTarget& target) override{
        target.draw(*shape);
    }
};

class Score : public GameObject {
private:
    sf::Text scoreText;
    int scorePaddle1;
    int scorePaddle2;

public:
    Score();
    Score(const sf::Font& font) : GameObject(nullptr), scorePaddle1(0), scorePaddle2(0) {
        scoreText.setFont(font);
        scoreText.setCharacterSize(40);
        scoreText.setFillColor(sf::Color::White);
    }

    void updateScore(int paddle1Score, int paddle2Score) {
        scorePaddle1 = paddle1Score;
        scorePaddle2 = paddle2Score;

        scoreText.setString("Blue: " + std::to_string(scorePaddle1) + "    Red: " + std::to_string(scorePaddle2));
    }

    void setPosition(float x, float y) {
        scoreText.setPosition(x, y);
    }

    void draw(sf::RenderTarget& target){
        target.draw(scoreText);
    }
};

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    const float paddleWidth = 20.0f;
    const float paddleHeight = 80.0f;
    const float paddleSpeed = 5.0f;
    const float ballRadius = 10.0f;
    const sf::Vector2f ballVelocity(5.0f, 5.0f);

    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Ping Pong");
    window.setFramerateLimit(60);

    // Load font for score display
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "Failed to load font file!" << std::endl;
        return 1;
    }

    Paddle paddle1(paddleWidth, paddleHeight, paddleSpeed);
    paddle1.shape->setFillColor(sf::Color::Blue);
    paddle1.setPosition(20.0f, screenHeight / 2 - paddleHeight / 2);

    Paddle paddle2(paddleWidth, paddleHeight, paddleSpeed);
    paddle2.shape->setFillColor(sf::Color::Red);
    paddle2.setPosition(screenWidth - paddleWidth - 20.0f, screenHeight / 2 - paddleHeight / 2);

    Ball ball(ballRadius, ballVelocity);
    ball.shape->setFillColor(sf::Color::Green);
    ball.setPosition(screenWidth / 2 - ballRadius, screenHeight / 2 - ballRadius);

    Score score(font);
    score.updateScore(0, 0);
    score.setPosition(20.0f, 20.0f);

    bool gameRunning = true;
    int scorePaddle1 = 0;
    int scorePaddle2 = 0;

    GameObject* gameObjects[] = { &paddle1, &paddle2, &ball, &score };

    while (window.isOpen() && gameRunning) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            paddle1.moveUp();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            paddle1.moveDown(screenHeight);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            paddle2.moveUp();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            paddle2.moveDown(screenHeight);

        ball.update();

        // Collisions with paddles
        if (ball.getPosition().x <= paddle1.getPosition().x + paddle1.getSize().x &&
            ball.getPosition().y + ball.getSize().y >= paddle1.getPosition().y &&
            ball.getPosition().y <= paddle1.getPosition().y + paddle1.getSize().y) {
            ball.reverseX();
        }

        if (ball.getPosition().x + ball.getSize().x >= paddle2.getPosition().x &&
            ball.getPosition().y + ball.getSize().y >= paddle2.getPosition().y &&
            ball.getPosition().y <= paddle2.getPosition().y + paddle2.getSize().y) {
            ball.reverseX();
        }

        // Collisions with walls
        if (ball.getPosition().y <= 0 || ball.getPosition().y + ball.getSize().y >= screenHeight) {
            ball.reverseY();
        }

        // Ball goes out of bounds
        if (ball.getPosition().x < 0) {
            scorePaddle2++;
            ball.setPosition(screenWidth / 2 - ballRadius, screenHeight / 2 - ballRadius);
            score.updateScore(scorePaddle1, scorePaddle2);
        }

        if (ball.getPosition().x > screenWidth) {
            scorePaddle1++;
            ball.setPosition(screenWidth / 2 - ballRadius, screenHeight / 2 - ballRadius);
            score.updateScore(scorePaddle1, scorePaddle2);
        }

        // Check game over condition
        if (scorePaddle1 >= 3 || scorePaddle2 >= 3) {
            gameRunning = false;
            std::string winner = (scorePaddle1 >= 3) ? "Blue" : "Red";
            std::cout << "Game Over! " << winner << " wins!" << std::endl;
        }

        window.clear();

        for (auto gameObject : gameObjects) {
            gameObject->draw(window);
        }

        window.display();
    }

    return 0;
}