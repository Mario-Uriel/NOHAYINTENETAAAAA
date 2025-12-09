#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 400;
const int GROUND_HEIGHT = 50;
const float GRAVITY = 0.6f;
const float JUMP_STRENGTH = -13.0f;

class Dino {
public:
    sf::RectangleShape body;
    sf::RectangleShape head;
    sf::RectangleShape eye;
    sf::RectangleShape arm;
    sf::RectangleShape tail;
    sf::RectangleShape leg1;
    sf::RectangleShape leg2;
    float velocityY;
    bool isJumping;
    bool isDucking;
    int animationFrame;
    sf::Clock animationClock;

    Dino(float x, float y) {
        // Cuerpo
        body.setSize(sf::Vector2f(50, 50));
        body.setPosition(sf::Vector2f(x, y));
        body.setFillColor(sf::Color(80, 80, 80));

        // Cabeza
        head.setSize(sf::Vector2f(35, 30));
        head.setFillColor(sf::Color(80, 80, 80));

        // Ojo
        eye.setSize(sf::Vector2f(4, 4));
        eye.setFillColor(sf::Color::White);

        // Brazo
        arm.setSize(sf::Vector2f(8, 25));
        arm.setFillColor(sf::Color(80, 80, 80));

        // Cola
        tail.setSize(sf::Vector2f(15, 12));
        tail.setFillColor(sf::Color(80, 80, 80));

        // Piernas
        leg1.setSize(sf::Vector2f(12, 25));
        leg1.setFillColor(sf::Color(80, 80, 80));

        leg2.setSize(sf::Vector2f(12, 25));
        leg2.setFillColor(sf::Color(80, 80, 80));

        velocityY = 0;
        isJumping = false;
        isDucking = false;
        animationFrame = 0;
    }

    void jump() {
        if (!isJumping && !isDucking) {
            velocityY = JUMP_STRENGTH;
            isJumping = true;
        }
    }

    void duck(bool shouldDuck) {
        if (!isJumping) {
            isDucking = shouldDuck;
        }
    }

    void update(float groundY) {
        // Aplicar gravedad
        if (isJumping) {
            velocityY += GRAVITY;
            body.move(sf::Vector2f(0, velocityY));

            // Verificar si tocó el suelo
            if (body.getPosition().y >= groundY) {
                body.setPosition(sf::Vector2f(body.getPosition().x, groundY));
                velocityY = 0;
                isJumping = false;
            }
        }

        // Animación de piernas
        if (animationClock.getElapsedTime().asSeconds() > 0.15f && !isJumping) {
            animationFrame = (animationFrame + 1) % 2;
            animationClock.restart();
        }

        float bodyX = body.getPosition().x;
        float bodyY = body.getPosition().y;

        if (isDucking) {
            // Posición agachado
            head.setPosition(sf::Vector2f(bodyX + 15, bodyY + 20));
            eye.setPosition(sf::Vector2f(bodyX + 40, bodyY + 25));
            arm.setPosition(sf::Vector2f(bodyX + 10, bodyY + 35));
            tail.setPosition(sf::Vector2f(bodyX - 10, bodyY + 25));
            
            leg1.setSize(sf::Vector2f(12, 15));
            leg2.setSize(sf::Vector2f(12, 15));
            leg1.setPosition(sf::Vector2f(bodyX + 10, bodyY + 50));
            leg2.setPosition(sf::Vector2f(bodyX + 30, bodyY + 50));
        } else {
            // Posición normal
            head.setPosition(sf::Vector2f(bodyX + 15, bodyY - 25));
            eye.setPosition(sf::Vector2f(bodyX + 40, bodyY - 18));
            arm.setPosition(sf::Vector2f(bodyX + 5, bodyY + 10));
            tail.setPosition(sf::Vector2f(bodyX - 10, bodyY + 15));

            leg1.setSize(sf::Vector2f(12, 25));
            leg2.setSize(sf::Vector2f(12, 25));

            // Actualizar posición de piernas con animación
            float legY = bodyY + body.getSize().y;
            if (isJumping) {
                leg1.setPosition(sf::Vector2f(bodyX + 8, legY));
                leg2.setPosition(sf::Vector2f(bodyX + 30, legY));
            } else {
                if (animationFrame == 0) {
                    leg1.setPosition(sf::Vector2f(bodyX + 8, legY));
                    leg2.setPosition(sf::Vector2f(bodyX + 30, legY - 5));
                } else {
                    leg1.setPosition(sf::Vector2f(bodyX + 8, legY - 5));
                    leg2.setPosition(sf::Vector2f(bodyX + 30, legY));
                }
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(tail);
        window.draw(body);
        window.draw(leg1);
        window.draw(leg2);
        window.draw(arm);
        window.draw(head);
        window.draw(eye);
    }

    sf::FloatRect getBounds() {
        // Crear hitbox más precisa
        sf::FloatRect bounds = body.getGlobalBounds();
        bounds.size.x -= 10; // Reducir un poco el ancho
        bounds.size.y -= 5;  // Reducir un poco la altura
        bounds.position.x += 5;
        bounds.position.y += 5;
        
        if (isDucking) {
            bounds.size.y = 40; // Altura reducida al agacharse
            bounds.position.y = body.getPosition().y + 25;
        }
        
        return bounds;
    }
};

class Obstacle {
public:
    sf::RectangleShape shape;
    float speed;
    bool isCactus;

    Obstacle(float x, float y, float width, float height, bool cactus = true) {
        shape.setSize(sf::Vector2f(width, height));
        shape.setPosition(sf::Vector2f(x, y));
        shape.setFillColor(sf::Color(34, 139, 34)); // Verde para cactus
        speed = 6.0f;
        isCactus = cactus;
    }

    void update() {
        shape.move(sf::Vector2f(-speed, 0));
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
        
        // Dibujar espinas del cactus
        if (isCactus) {
            float x = shape.getPosition().x;
            float y = shape.getPosition().y;
            float w = shape.getSize().x;
            float h = shape.getSize().y;
            
            // Espinas laterales
            sf::RectangleShape spike1(sf::Vector2f(8, 15));
            spike1.setPosition(sf::Vector2f(x - 5, y + h * 0.3f));
            spike1.setFillColor(sf::Color(34, 139, 34));
            
            sf::RectangleShape spike2(sf::Vector2f(8, 15));
            spike2.setPosition(sf::Vector2f(x + w - 3, y + h * 0.5f));
            spike2.setFillColor(sf::Color(34, 139, 34));
            
            window.draw(spike1);
            window.draw(spike2);
        }
    }

    bool isOffScreen() const {
        return shape.getPosition().x + shape.getSize().x < 0;
    }

    sf::FloatRect getBounds() const {
        // Hitbox más precisa
        sf::FloatRect bounds = shape.getGlobalBounds();
        bounds.size.x -= 8;
        bounds.size.y -= 8;
        bounds.position.x += 4;
        bounds.position.y += 4;
        return bounds;
    }
};

class Bird {
public:
    sf::RectangleShape body;
    sf::RectangleShape wing1;
    sf::RectangleShape wing2;
    sf::RectangleShape beak;
    float speed;
    int animationFrame;
    sf::Clock animationClock;
    float yPos;

    Bird(float x, float y) {
        yPos = y;
        body.setSize(sf::Vector2f(30, 15));
        body.setPosition(sf::Vector2f(x, y));
        body.setFillColor(sf::Color(64, 64, 64));

        wing1.setSize(sf::Vector2f(20, 8));
        wing1.setFillColor(sf::Color(64, 64, 64));

        wing2.setSize(sf::Vector2f(20, 8));
        wing2.setFillColor(sf::Color(64, 64, 64));

        beak.setSize(sf::Vector2f(8, 5));
        beak.setFillColor(sf::Color(255, 165, 0));

        speed = 7.0f;
        animationFrame = 0;
    }

    void update() {
        body.move(sf::Vector2f(-speed, 0));
        
        // Animación de alas
        if (animationClock.getElapsedTime().asSeconds() > 0.1f) {
            animationFrame = (animationFrame + 1) % 2;
            animationClock.restart();
        }

        float x = body.getPosition().x;
        float y = body.getPosition().y;

        beak.setPosition(sf::Vector2f(x + 30, y + 5));

        if (animationFrame == 0) {
            wing1.setPosition(sf::Vector2f(x + 5, y - 5));
            wing2.setPosition(sf::Vector2f(x + 5, y + 15));
        } else {
            wing1.setPosition(sf::Vector2f(x + 5, y - 2));
            wing2.setPosition(sf::Vector2f(x + 5, y + 12));
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(wing1);
        window.draw(wing2);
        window.draw(body);
        window.draw(beak);
    }

    bool isOffScreen() const {
        return body.getPosition().x + body.getSize().x < 0;
    }

    sf::FloatRect getBounds() const {
        sf::FloatRect bounds = body.getGlobalBounds();
        bounds.size.x -= 6;
        bounds.size.y -= 4;
        bounds.position.x += 3;
        bounds.position.y += 2;
        return bounds;
    }
};

class Cloud {
public:
    sf::CircleShape shape1;
    sf::CircleShape shape2;
    sf::CircleShape shape3;
    float speed;
    float x, y;

    Cloud(float posX, float posY) {
        x = posX;
        y = posY;
        shape1.setRadius(15);
        shape2.setRadius(20);
        shape3.setRadius(15);
        
        shape1.setFillColor(sf::Color(200, 200, 200));
        shape2.setFillColor(sf::Color(200, 200, 200));
        shape3.setFillColor(sf::Color(200, 200, 200));
        
        speed = 1.0f;
        updatePosition();
    }

    void updatePosition() {
        shape1.setPosition(sf::Vector2f(x, y));
        shape2.setPosition(sf::Vector2f(x + 15, y - 5));
        shape3.setPosition(sf::Vector2f(x + 35, y));
    }

    void update() {
        x -= speed;
        if (x + 65 < 0) {
            x = WINDOW_WIDTH;
        }
        updatePosition();
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape1);
        window.draw(shape2);
        window.draw(shape3);
    }
};

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Dino T-Rex Run");
    window.setFramerateLimit(60);

    float groundY = WINDOW_HEIGHT - GROUND_HEIGHT - 70;
    
    // Crear dino
    Dino dino(100, groundY);

    // Vector de obstáculos y aves
    std::vector<Obstacle> obstacles;
    std::vector<Bird> birds;
    sf::Clock obstacleClock;
    float nextObstacleTime = 2.0f;

    // Crear nubes
    std::vector<Cloud> clouds;
    clouds.push_back(Cloud(200, 50));
    clouds.push_back(Cloud(500, 80));

    // Suelo
    sf::RectangleShape ground(sf::Vector2f(WINDOW_WIDTH, GROUND_HEIGHT));
    ground.setPosition(sf::Vector2f(0, WINDOW_HEIGHT - GROUND_HEIGHT));
    ground.setFillColor(sf::Color(100, 100, 100));

    // Líneas del suelo
    std::vector<sf::RectangleShape> groundLines;
    for (int i = 0; i < 10; i++) {
        sf::RectangleShape line(sf::Vector2f(50, 3));
        line.setPosition(sf::Vector2f(i * 100, WINDOW_HEIGHT - GROUND_HEIGHT));
        line.setFillColor(sf::Color(80, 80, 80));
        groundLines.push_back(line);
    }

    // Puntaje
    int score = 0;
    sf::Clock scoreClock;
    sf::Font font;
    
    // Intentar cargar una fuente (opcional)
    bool fontLoaded = font.openFromFile("./assets/fonts/Minecraft.ttf");
    sf::Text scoreText(font);
    if (fontLoaded) {
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::Black);
        scoreText.setPosition(sf::Vector2f(WINDOW_WIDTH - 150, 20));
    }

    bool gameOver = false;
    bool gameStarted = false;

    while (window.isOpen()) {
        // Procesar eventos
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            if (event->is<sf::Event::KeyPressed>()) {
                const auto& keyEvent = event->getIf<sf::Event::KeyPressed>();
                if (!gameStarted) {
                    gameStarted = true;
                }
                if (!gameOver) {
                    if (keyEvent->code == sf::Keyboard::Key::Space || 
                        keyEvent->code == sf::Keyboard::Key::Up) {
                        dino.jump();
                    }
                } else {
                    // Reiniciar juego
                    gameOver = false;
                    gameStarted = true;
                    score = 0;
                    obstacles.clear();
                    birds.clear();
                    dino = Dino(100, groundY);
                    nextObstacleTime = 2.0f;
                }
            }

            if (event->is<sf::Event::KeyReleased>()) {
                const auto& keyEvent = event->getIf<sf::Event::KeyReleased>();
                if (keyEvent->code == sf::Keyboard::Key::Down) {
                    dino.duck(false);
                }
            }
        }

        // Control de agacharse
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) && !gameOver && gameStarted) {
            dino.duck(true);
        } else if (!gameOver && gameStarted) {
            dino.duck(false);
        }

        if (gameStarted && !gameOver) {
            // Actualizar dino
            dino.update(groundY);

            // Crear nuevos obstáculos y aves
            if (obstacleClock.getElapsedTime().asSeconds() > nextObstacleTime) {
                int enemyType = std::rand() % 10;
                
                if (enemyType < 3) {
                    // Ave (30% probabilidad)
                    int birdHeight = std::rand() % 2;
                    float birdY = birdHeight == 0 ? groundY - 20 : groundY - 60;
                    birds.push_back(Bird(WINDOW_WIDTH, birdY));
                } else if (enemyType < 6) {
                    // Cactus pequeño
                    obstacles.push_back(Obstacle(WINDOW_WIDTH, groundY + 35, 20, 40, true));
                } else if (enemyType < 8) {
                    // Cactus doble
                    obstacles.push_back(Obstacle(WINDOW_WIDTH, groundY + 25, 25, 50, true));
                    obstacles.push_back(Obstacle(WINDOW_WIDTH + 30, groundY + 25, 25, 50, true));
                } else {
                    // Cactus grande
                    obstacles.push_back(Obstacle(WINDOW_WIDTH, groundY + 15, 30, 60, true));
                }
                
                // Tiempo aleatorio para el siguiente obstáculo (más rápido conforme avanza)
                nextObstacleTime = 1.2f + static_cast<float>(std::rand() % 100) / 100.0f;
                if (score > 500) nextObstacleTime *= 0.8f;
                if (score > 1000) nextObstacleTime *= 0.8f;
                
                obstacleClock.restart();
            }

            // Actualizar obstáculos
            for (auto& obstacle : obstacles) {
                obstacle.update();
                
                // Verificar colisión con hitbox más precisa
                if (dino.getBounds().findIntersection(obstacle.getBounds()).has_value()) {
                    gameOver = true;
                }
            }

            // Actualizar aves
            for (auto& bird : birds) {
                bird.update();
                
                // Verificar colisión
                if (dino.getBounds().findIntersection(bird.getBounds()).has_value()) {
                    gameOver = true;
                }
            }

            // Eliminar obstáculos y aves fuera de pantalla
            obstacles.erase(
                std::remove_if(obstacles.begin(), obstacles.end(),
                    [](const Obstacle& o) { return o.isOffScreen(); }),
                obstacles.end()
            );

            birds.erase(
                std::remove_if(birds.begin(), birds.end(),
                    [](const Bird& b) { return b.isOffScreen(); }),
                birds.end()
            );

            // Actualizar nubes
            for (auto& cloud : clouds) {
                cloud.update();
            }

            // Actualizar líneas del suelo
            for (auto& line : groundLines) {
                line.move(sf::Vector2f(-5, 0));
                if (line.getPosition().x + line.getSize().x < 0) {
                    line.setPosition(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT - GROUND_HEIGHT));
                }
            }

            // Actualizar puntaje
            if (scoreClock.getElapsedTime().asSeconds() > 0.1f) {
                score++;
                scoreClock.restart();
            }

            if (fontLoaded) {
                scoreText.setString("Score: " + std::to_string(score));
            }
        }

        // Dibujar
        window.clear(sf::Color(247, 247, 247)); // Fondo gris claro

        // Dibujar nubes
        for (auto& cloud : clouds) {
            cloud.draw(window);
        }

        // Dibujar suelo
        window.draw(ground);
        for (auto& line : groundLines) {
            window.draw(line);
        }

        // Dibujar dino
        dino.draw(window);

        // Dibujar obstáculos
        for (auto& obstacle : obstacles) {
            obstacle.draw(window);
        }

        // Dibujar aves
        for (auto& bird : birds) {
            bird.draw(window);
        }

        // Dibujar puntaje
        if (fontLoaded) {
            window.draw(scoreText);
        }

        // Mensaje de game over
        if (gameOver) {
            sf::RectangleShape gameOverBg(sf::Vector2f(300, 100));
            gameOverBg.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 50));
            gameOverBg.setFillColor(sf::Color(0, 0, 0, 200));
            window.draw(gameOverBg);
            
            if (fontLoaded) {
                sf::Text gameOverText(font);
                gameOverText.setString("GAME OVER");
                gameOverText.setCharacterSize(30);
                gameOverText.setFillColor(sf::Color::White);
                gameOverText.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 90, WINDOW_HEIGHT / 2 - 40));
                window.draw(gameOverText);

                sf::Text restartText(font);
                restartText.setString("Press any key");
                restartText.setCharacterSize(20);
                restartText.setFillColor(sf::Color::White);
                restartText.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 70, WINDOW_HEIGHT / 2 + 10));
                window.draw(restartText);
            }
        }

        // Mensaje inicial
        if (!gameStarted && fontLoaded) {
            sf::Text startText(font);
            startText.setString("Press SPACE or UP to jump");
            startText.setCharacterSize(20);
            startText.setFillColor(sf::Color::Black);
            startText.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 140, WINDOW_HEIGHT / 2 - 40));
            window.draw(startText);

            sf::Text startText2(font);
            startText2.setString("Press DOWN to duck");
            startText2.setCharacterSize(20);
            startText2.setFillColor(sf::Color::Black);
            startText2.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 - 10));
            window.draw(startText2);
        }

        window.display();
    }

    return 0;
}
