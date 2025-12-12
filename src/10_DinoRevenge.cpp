#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 600;
const int GROUND_HEIGHT = 50;
const float GRAVITY = 0.5f;
const float JUMP_STRENGTH = -12.0f;

class Projectile {
public:
    sf::CircleShape shape;
    sf::CircleShape glow;
    sf::Vector2f velocity;
    bool active;
    int direction; // 1 = derecha, -1 = izquierda

    Projectile(float x, float y, int dir) {
        direction = dir;
        shape.setRadius(6);
        shape.setPosition(sf::Vector2f(x, y));
        shape.setFillColor(sf::Color(255, 150, 0));
        shape.setOutlineColor(sf::Color(255, 200, 0));
        shape.setOutlineThickness(2);
        
        // Efecto de brillo
        glow.setRadius(10);
        glow.setPosition(sf::Vector2f(x - 4, y - 4));
        glow.setFillColor(sf::Color(255, 100, 0, 100));
        
        velocity = sf::Vector2f(15.0f * direction, 0.0f); // Disparo recto
        active = true;
    }

    void update() {
        shape.move(velocity);
        glow.setPosition(sf::Vector2f(shape.getPosition().x - 4, shape.getPosition().y - 4));

        // Desactivar si sale de la pantalla
        if (shape.getPosition().x > WINDOW_WIDTH + 20 || 
            shape.getPosition().x < -20 ||
            shape.getPosition().y > WINDOW_HEIGHT) {
            active = false;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (active) {
            window.draw(glow);
            window.draw(shape);
        }
    }

    sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }
};

class Dino {
public:
    sf::Texture walkTexture;
    sf::Sprite sprite;
    float velocityY;
    float x, y;
    bool isJumping;
    bool isDucking;
    int animationFrame;
    int facingDirection; // 1 = derecha, -1 = izquierda
    sf::Clock animationClock;
    sf::Clock shootClock;
    int numFrames;
    float spriteScale;

    Dino(float startX, float startY) : sprite(walkTexture) {
        x = startX;
        y = startY;
        facingDirection = 1;
        numFrames = 4;
        spriteScale = 0.6f; // Ajustado para estar al nivel de los cactus
        
        // Cargar textura
        if (!walkTexture.loadFromFile("assets/images/PIKACHU (2) (1).png")) {
            // Error al cargar textura
        }
        
        sprite.setTexture(walkTexture);
        sprite.setScale(sf::Vector2f(spriteScale, spriteScale));
        
        // Configurar el primer frame
        sf::Vector2u texSize = walkTexture.getSize();
        int frameWidth = texSize.x / numFrames;
        sprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(frameWidth, texSize.y)));
        
        // Establecer origen en la base completa (100% abajo)
        sprite.setOrigin(sf::Vector2f(frameWidth / 2.0f, texSize.y));

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

    void moveLeft() {
        x -= 5;
        if (x < 20) x = 20;
        facingDirection = -1;
    }

    void moveRight() {
        x += 5;
        if (x > WINDOW_WIDTH - 100) x = WINDOW_WIDTH - 100;
        facingDirection = 1;
    }

    void update(float groundY) {
        // Aplicar gravedad
        if (isJumping) {
            // Si está presionando abajo mientras salta, acelerar la caída
            if (isDucking) {
                velocityY += GRAVITY * 2.5f; // Acelerar caída
            } else {
                velocityY += GRAVITY;
            }
            y += velocityY;

            // Verificar si tocó el suelo
            if (y >= groundY) {
                y = groundY;
                velocityY = 0;
                isJumping = false;
            }
        }

        // Configurar sprite (siempre usa walkTexture)
        sprite.setTexture(walkTexture);
        sprite.setScale(sf::Vector2f(spriteScale, spriteScale));
        sf::Vector2u texSize = walkTexture.getSize();
        int frameWidth = texSize.x / numFrames;
        sprite.setOrigin(sf::Vector2f(frameWidth / 2.0f, texSize.y)); // Origen en la base 100%
        
        // Animación de frames
        if (animationClock.getElapsedTime().asSeconds() > 0.12f && !isJumping) {
            animationFrame = (animationFrame + 1) % numFrames;
            sprite.setTextureRect(sf::IntRect(sf::Vector2i(animationFrame * frameWidth, 0), sf::Vector2i(frameWidth, texSize.y)));
            animationClock.restart();
        }
        
        // Voltear sprite según dirección
        if (facingDirection == -1) {
            sprite.setScale(sf::Vector2f(-spriteScale, spriteScale));
        }
        
        // Actualizar posición del sprite
        sprite.setPosition(sf::Vector2f(x, y));
    }

    bool canShoot() {
        return shootClock.getElapsedTime().asSeconds() > 0.25f;
    }

    void resetShootClock() {
        shootClock.restart();
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    sf::FloatRect getBounds() const {
        sf::FloatRect bounds = sprite.getGlobalBounds();
        // Ajustar hitbox para ser más precisa (centrar en el personaje)
        float newWidth = bounds.size.x * 0.6f;
        float newHeight = bounds.size.y * 0.7f;
        bounds.position.x += (bounds.size.x - newWidth) / 2.0f;
        bounds.position.y += (bounds.size.y - newHeight) / 2.0f;
        bounds.size.x = newWidth;
        bounds.size.y = newHeight;
        return bounds;
    }

    sf::Vector2f getShootPosition() const {
        sf::FloatRect bounds = sprite.getGlobalBounds();
        float shootY = y - (bounds.size.y * 0.5f);
        if (facingDirection == 1) {
            return sf::Vector2f(bounds.position.x + bounds.size.x, shootY);
        } else {
            return sf::Vector2f(bounds.position.x, shootY);
        }
    }

    int getFacingDirection() const {
        return facingDirection;
    }
};

class Enemy {
public:
    sf::RectangleShape body;
    sf::RectangleShape spike1;
    sf::RectangleShape spike2;
    sf::RectangleShape spike3;
    sf::CircleShape weak_spot;
    float speed;
    int health;
    bool isFlying;
    float baseY;
    sf::Clock hoverClock;
    sf::Clock damageClock;
    bool showDamage;

    Enemy(float startX, float startY, bool flying = false) {
        isFlying = flying;
        baseY = startY;
        showDamage = false;
        
        if (flying) {
            // Ave enemiga mejorada
            body.setSize(sf::Vector2f(40, 25));
            body.setFillColor(sf::Color(80, 40, 0));
            body.setOutlineColor(sf::Color(50, 25, 0));
            body.setOutlineThickness(2);
            
            spike1.setSize(sf::Vector2f(28, 12));
            spike1.setFillColor(sf::Color(60, 30, 0));
            
            spike2.setSize(sf::Vector2f(28, 12));
            spike2.setFillColor(sf::Color(60, 30, 0));
            
            // Pico
            spike3.setSize(sf::Vector2f(12, 6));
            spike3.setFillColor(sf::Color(255, 165, 0));
            
            health = 2;
        } else {
            // Cactus enemigo - tamaño ajustado para coincidir con el personaje
            body.setSize(sf::Vector2f(30, 50));
            body.setFillColor(sf::Color(40, 120, 40));
            body.setOutlineColor(sf::Color(30, 80, 30));
            body.setOutlineThickness(2);
            
            spike1.setSize(sf::Vector2f(10, 18));
            spike1.setFillColor(sf::Color(40, 120, 40));
            
            spike2.setSize(sf::Vector2f(10, 18));
            spike2.setFillColor(sf::Color(40, 120, 40));
            
            spike3.setSize(sf::Vector2f(8, 12));
            spike3.setFillColor(sf::Color(40, 120, 40));
            
            health = 3;
        }
        
        // Punto débil
        weak_spot.setRadius(4);
        weak_spot.setFillColor(sf::Color::Red);
        
        body.setPosition(sf::Vector2f(startX, startY));
        speed = 3.5f;
    }

    void update() {
        body.move(sf::Vector2f(-speed, 0));
        
        if (isFlying) {
            // Efecto de vuelo ondulante más suave
            float offset = std::sin(hoverClock.getElapsedTime().asSeconds() * 2.5f) * 8.0f;
            body.setPosition(sf::Vector2f(body.getPosition().x, baseY + offset));
            
            // Alas con animación
            float x = body.getPosition().x;
            float y = body.getPosition().y;
            float wingOffset = std::sin(hoverClock.getElapsedTime().asSeconds() * 10.0f) * 3.0f;
            
            spike1.setPosition(sf::Vector2f(x + 6, y - 10 + wingOffset));
            spike2.setPosition(sf::Vector2f(x + 6, y + 25 - wingOffset));
            spike3.setPosition(sf::Vector2f(x + 40, y + 10)); // Pico
            weak_spot.setPosition(sf::Vector2f(x + 15, y + 8));
        } else {
            // Espinas del cactus - ajustadas al tamaño
            float x = body.getPosition().x;
            float y = body.getPosition().y;
            spike1.setPosition(sf::Vector2f(x - 8, y + 12));
            spike2.setPosition(sf::Vector2f(x + body.getSize().x - 2, y + 25));
            spike3.setPosition(sf::Vector2f(x + body.getSize().x / 2 - 4, y - 8));
            weak_spot.setPosition(sf::Vector2f(x + 13, y + 20));
        }
        
        // Efecto de daño
        if (showDamage && damageClock.getElapsedTime().asSeconds() > 0.1f) {
            showDamage = false;
        }
    }

    void takeDamage() {
        health--;
        showDamage = true;
        damageClock.restart();
        
        // Cambiar color cuando recibe daño
        sf::Color currentColor = body.getFillColor();
        if (health == 2) {
            body.setFillColor(sf::Color(currentColor.r + 30, currentColor.g + 30, currentColor.b));
        } else if (health == 1) {
            body.setFillColor(sf::Color(currentColor.r + 50, currentColor.g + 50, currentColor.b));
        }
    }

    bool isDead() const {
        return health <= 0;
    }

    void draw(sf::RenderWindow& window) {
        if (showDamage) {
            // Parpadeo al recibir daño
            sf::Color flashColor = sf::Color::White;
            sf::Color originalColor = body.getFillColor();
            body.setFillColor(flashColor);
            window.draw(body);
            body.setFillColor(originalColor);
        } else {
            window.draw(body);
        }
        
        window.draw(spike1);
        window.draw(spike2);
        window.draw(spike3);
        
        // Mostrar punto débil si tiene poca vida
        if (health == 1) {
            window.draw(weak_spot);
        }
    }

    bool isOffScreen() const {
        return body.getPosition().x + body.getSize().x < -50;
    }

    sf::FloatRect getBounds() const {
        sf::FloatRect bounds = body.getGlobalBounds();
        bounds.size.x -= 8;
        bounds.size.y -= 8;
        bounds.position.x += 4;
        bounds.position.y += 4;
        return bounds;
    }
};

class Explosion {
public:
    std::vector<sf::CircleShape> particles;
    sf::Clock lifeClock;
    bool active;

    Explosion(float x, float y) {
        active = true;
        for (int i = 0; i < 8; i++) {
            sf::CircleShape particle(4);
            particle.setPosition(sf::Vector2f(x, y));
            particle.setFillColor(sf::Color(255, 150 + std::rand() % 100, 0));
            particles.push_back(particle);
        }
    }

    void update() {
        float time = lifeClock.getElapsedTime().asSeconds();
        if (time > 0.5f) {
            active = false;
            return;
        }

        for (size_t i = 0; i < particles.size(); i++) {
            float angle = (i * 45.0f) * 3.14159f / 180.0f;
            float speed = 3.0f;
            particles[i].move(sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed));
            
            // Fade out
            sf::Color color = particles[i].getFillColor();
            color.a = static_cast<unsigned char>(255 * (1.0f - time * 2.0f));
            particles[i].setFillColor(color);
        }
    }

    void draw(sf::RenderWindow& window) {
        if (active) {
            for (auto& particle : particles) {
                window.draw(particle);
            }
        }
    }
};

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Dino Revenge");
    window.setFramerateLimit(60);

    // Cargar imagen de fondo
    sf::Texture backgroundTexture;
    bool backgroundLoaded = backgroundTexture.loadFromFile("./assets/images/fondo.png");
    backgroundTexture.setRepeated(true);
    sf::Sprite backgroundSprite(backgroundTexture);
    sf::Sprite backgroundSprite2(backgroundTexture);
    
    float bgScrollSpeed = 2.0f;
    float bgX1 = 0.0f;
    float bgX2 = 0.0f;
    
    if (backgroundLoaded) {
        // Escalar el fondo manteniendo la proporción
        sf::Vector2u texSize = backgroundTexture.getSize();
        float scaleY = static_cast<float>(WINDOW_HEIGHT) / texSize.y;
        float scaleX = scaleY; // Mantener proporción
        backgroundSprite.setScale(sf::Vector2f(scaleX, scaleY));
        backgroundSprite2.setScale(sf::Vector2f(scaleX, scaleY));
        
        // Calcular el ancho escalado
        float scaledWidth = texSize.x * scaleX;
        bgX2 = scaledWidth;
        
        backgroundSprite.setPosition(sf::Vector2f(bgX1, 0));
        backgroundSprite2.setPosition(sf::Vector2f(bgX2, 0));
    }

    // Posición del suelo (donde el dino camina)
    // WINDOW_HEIGHT = 600, GROUND_HEIGHT = 50
    // El personaje debe estar exactamente sobre el rectángulo del suelo
    float groundY = WINDOW_HEIGHT - GROUND_HEIGHT; // 550
    
    Dino dino(100, groundY);
    std::vector<Projectile> projectiles;
    std::vector<Enemy> enemies;
    std::vector<Explosion> explosions;
    
    sf::Clock enemyClock;
    float nextEnemyTime = 2.0f;

    // Suelo
    sf::RectangleShape ground(sf::Vector2f(WINDOW_WIDTH, GROUND_HEIGHT));
    ground.setPosition(sf::Vector2f(0, WINDOW_HEIGHT - GROUND_HEIGHT));
    ground.setFillColor(sf::Color(100, 100, 100));

    // Puntaje y estadísticas
    int score = 0;
    int enemiesKilled = 0;
    sf::Clock scoreClock;
    sf::Font font;
    
    bool fontLoaded = font.openFromFile("./assets/fonts/Minecraft.ttf");
    sf::Text scoreText(font);
    sf::Text killsText(font);
    sf::Text healthText(font);
    
    if (fontLoaded) {
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(sf::Color::Black);
        scoreText.setPosition(sf::Vector2f(20, 20));

        killsText.setCharacterSize(20);
        killsText.setFillColor(sf::Color::Red);
        killsText.setPosition(sf::Vector2f(20, 50));

        healthText.setCharacterSize(20);
        healthText.setFillColor(sf::Color::White);
        healthText.setPosition(sf::Vector2f(20, 80));
    }

    bool gameOver = false;
    bool gameStarted = false;
    int playerHealth = 3;

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
                        keyEvent->code == sf::Keyboard::Key::Up || 
                        keyEvent->code == sf::Keyboard::Key::W) {
                        dino.jump();
                    }
                    
                    // Disparar hacia adelante con X o Ctrl
                    if ((keyEvent->code == sf::Keyboard::Key::X || 
                         keyEvent->code == sf::Keyboard::Key::LControl) && dino.canShoot()) {
                        sf::Vector2f shootPos = dino.getShootPosition();
                        projectiles.push_back(Projectile(shootPos.x, shootPos.y, dino.getFacingDirection()));
                        dino.resetShootClock();
                    }
                    
                    // Disparar hacia atrás con Z
                    if (keyEvent->code == sf::Keyboard::Key::Z && dino.canShoot()) {
                        sf::Vector2f shootPos = dino.getShootPosition();
                        projectiles.push_back(Projectile(shootPos.x, shootPos.y, -dino.getFacingDirection()));
                        dino.resetShootClock();
                    }
                } else {
                    // Reiniciar juego
                    gameOver = false;
                    gameStarted = true;
                    score = 0;
                    enemiesKilled = 0;
                    playerHealth = 3;
                    enemies.clear();
                    projectiles.clear();
                    explosions.clear();
                    dino = Dino(100, groundY);
                    nextEnemyTime = 2.0f;
                }
            }

            if (event->is<sf::Event::KeyReleased>()) {
                const auto& keyEvent = event->getIf<sf::Event::KeyReleased>();
                if (keyEvent->code == sf::Keyboard::Key::Down || 
                    keyEvent->code == sf::Keyboard::Key::S) {
                    dino.duck(false);
                }
            }
        }

        if (gameStarted && !gameOver) {
            // Actualizar scroll del fondo
            if (backgroundLoaded) {
                bgX1 -= bgScrollSpeed;
                bgX2 -= bgScrollSpeed;
                
                // Obtener el ancho escalado
                sf::Vector2u texSize = backgroundTexture.getSize();
                float scaleY = static_cast<float>(WINDOW_HEIGHT) / texSize.y;
                float scaledWidth = texSize.x * scaleY;
                
                // Reposicionar cuando sale de la pantalla
                if (bgX1 + scaledWidth <= 0) {
                    bgX1 = bgX2 + scaledWidth;
                }
                if (bgX2 + scaledWidth <= 0) {
                    bgX2 = bgX1 + scaledWidth;
                }
                
                backgroundSprite.setPosition(sf::Vector2f(bgX1, 0));
                backgroundSprite2.setPosition(sf::Vector2f(bgX2, 0));
            }
            
            // Control de agacharse
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
                dino.duck(true);
            } else {
                dino.duck(false);
            }

            // Controles de movimiento
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
                dino.moveLeft();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
                dino.moveRight();
            }

            // Actualizar dino
            dino.update(groundY);

            // Crear nuevos enemigos
            if (enemyClock.getElapsedTime().asSeconds() > nextEnemyTime) {
                int enemyType = std::rand() % 10;
                
                if (enemyType < 4) {
                    // Ave voladora - a diferentes alturas alcanzables con salto
                    int height = std::rand() % 3;
                    float birdY = groundY - 60 - (height * 25); // Alturas: -60, -85, -110 (más bajas)
                    enemies.push_back(Enemy(WINDOW_WIDTH, birdY, true));
                } else {
                    // Cactus terrestre - pegado al suelo
                    // El cactus se dibuja desde su posición hacia abajo, así que la posición Y
                    // debe ser groundY menos la altura del cactus para que la base toque el suelo
                    float cactusY = groundY - 50; // groundY menos altura del cactus (50px)
                    enemies.push_back(Enemy(WINDOW_WIDTH, cactusY, false));
                }
                
                nextEnemyTime = 1.5f + static_cast<float>(std::rand() % 100) / 100.0f;
                if (score > 500) nextEnemyTime *= 0.85f;
                
                enemyClock.restart();
            }

            // Actualizar proyectiles
            for (auto& proj : projectiles) {
                proj.update();
                
                // Verificar colisión con enemigos
                for (auto& enemy : enemies) {
                    if (proj.active && proj.getBounds().findIntersection(enemy.getBounds()).has_value()) {
                        enemy.takeDamage();
                        proj.active = false;
                        
                        if (enemy.isDead()) {
                            explosions.push_back(Explosion(
                                enemy.getBounds().position.x + enemy.getBounds().size.x / 2,
                                enemy.getBounds().position.y + enemy.getBounds().size.y / 2
                            ));
                            enemiesKilled++;
                            score += 50;
                        }
                    }
                }
            }

            // Eliminar proyectiles inactivos
            projectiles.erase(
                std::remove_if(projectiles.begin(), projectiles.end(),
                    [](const Projectile& p) { return !p.active; }),
                projectiles.end()
            );

            // Actualizar enemigos
            for (auto& enemy : enemies) {
                enemy.update();
                
                // Verificar colisión con el dino
                if (enemy.getBounds().findIntersection(dino.getBounds()).has_value()) {
                    playerHealth--;
                    explosions.push_back(Explosion(
                        enemy.getBounds().position.x,
                        enemy.getBounds().position.y
                    ));
                    enemy.health = 0; // Destruir enemigo tras colisión
                    
                    if (playerHealth <= 0) {
                        gameOver = true;
                    }
                }
            }

            // Eliminar enemigos muertos o fuera de pantalla
            enemies.erase(
                std::remove_if(enemies.begin(), enemies.end(),
                    [](const Enemy& e) { return e.isDead() || e.isOffScreen(); }),
                enemies.end()
            );

            // Actualizar explosiones
            for (auto& explosion : explosions) {
                explosion.update();
            }

            explosions.erase(
                std::remove_if(explosions.begin(), explosions.end(),
                    [](const Explosion& e) { return !e.active; }),
                explosions.end()
            );

            // Actualizar puntaje por tiempo
            if (scoreClock.getElapsedTime().asSeconds() > 0.1f) {
                score++;
                scoreClock.restart();
            }

            if (fontLoaded) {
                scoreText.setString("Score: " + std::to_string(score));
                killsText.setString("Kills: " + std::to_string(enemiesKilled));
                healthText.setString("Health: " + std::string(playerHealth, '*'));
            }
        }

        // Dibujar
        if (backgroundLoaded) {
            window.clear();
            window.draw(backgroundSprite);
            window.draw(backgroundSprite2);
        } else {
            window.clear(sf::Color(135, 206, 235)); // Cielo azul
        }

        // Dibujar suelo
        window.draw(ground);

        // Dibujar dino
        dino.draw(window);

        // Dibujar proyectiles
        for (auto& proj : projectiles) {
            proj.draw(window);
        }

        // Dibujar enemigos
        for (auto& enemy : enemies) {
            enemy.draw(window);
        }

        // Dibujar explosiones
        for (auto& explosion : explosions) {
            explosion.draw(window);
        }

        // Dibujar UI
        if (fontLoaded) {
            window.draw(scoreText);
            window.draw(killsText);
            window.draw(healthText);
        }

        // Mensaje de game over
        if (gameOver && fontLoaded) {
            sf::RectangleShape gameOverBg(sf::Vector2f(400, 150));
            gameOverBg.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2 - 75));
            gameOverBg.setFillColor(sf::Color(0, 0, 0, 200));
            window.draw(gameOverBg);
            
            sf::Text gameOverText(font);
            gameOverText.setString("GAME OVER");
            gameOverText.setCharacterSize(40);
            gameOverText.setFillColor(sf::Color::Red);
            gameOverText.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 110, WINDOW_HEIGHT / 2 - 50));
            window.draw(gameOverText);

            sf::Text finalScore(font);
            finalScore.setString("Final Score: " + std::to_string(score));
            finalScore.setCharacterSize(20);
            finalScore.setFillColor(sf::Color::White);
            finalScore.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2));
            window.draw(finalScore);

            sf::Text restartText(font);
            restartText.setString("Press any key to restart");
            restartText.setCharacterSize(18);
            restartText.setFillColor(sf::Color::White);
            restartText.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 110, WINDOW_HEIGHT / 2 + 40));
            window.draw(restartText);
        }

        // Mensaje inicial
        if (!gameStarted && fontLoaded) {
            sf::RectangleShape instrBg(sf::Vector2f(500, 200));
            instrBg.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 250, WINDOW_HEIGHT / 2 - 100));
            instrBg.setFillColor(sf::Color(0, 0, 0, 180));
            window.draw(instrBg);

            sf::Text title(font);
            title.setString("DINO REVENGE");
            title.setCharacterSize(35);
            title.setFillColor(sf::Color::Red);
            title.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 - 80));
            window.draw(title);

            sf::Text instr1(font);
            instr1.setString("ARROWS / WASD: Move and Jump");
            instr1.setCharacterSize(16);
            instr1.setFillColor(sf::Color::White);
            instr1.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 35));
            window.draw(instr1);

            sf::Text instr2(font);
            instr2.setString("X / CTRL: Shoot Forward");
            instr2.setCharacterSize(16);
            instr2.setFillColor(sf::Color::White);
            instr2.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 110, WINDOW_HEIGHT / 2 - 10));
            window.draw(instr2);

            sf::Text instr3(font);
            instr3.setString("Z: Shoot Backward");
            instr3.setCharacterSize(16);
            instr3.setFillColor(sf::Color::White);
            instr3.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 85, WINDOW_HEIGHT / 2 + 15));
            window.draw(instr3);

            sf::Text instr4(font);
            instr4.setString("DOWN / S: Duck");
            instr4.setCharacterSize(16);
            instr4.setFillColor(sf::Color::White);
            instr4.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 70, WINDOW_HEIGHT / 2 + 40));
            window.draw(instr4);

            sf::Text instr5(font);
            instr5.setString("Press any key to start!");
            instr5.setCharacterSize(20);
            instr5.setFillColor(sf::Color::Yellow);
            instr5.setPosition(sf::Vector2f(WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 + 70));
            window.draw(instr5);
        }

        window.display();
    }

    return 0;
}
