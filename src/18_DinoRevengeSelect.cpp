#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 600;
const int GROUND_HEIGHT = 50;
const float GRAVITY = 0.4f;
const float JUMP_STRENGTH = -15.0f;

// Enumeraciones para menús y dificultad
enum class GameDifficulty {
    EASY,
    NORMAL,
    HARD
};

enum class MenuState {
    MAIN_MENU,
    DIFFICULTY_SELECT,
    CHARACTER_SELECT,
    SETTINGS,
    HIGH_SCORES,
    PLAYING
};

// Estructura para guardar récords con nombre de jugador
struct HighScoreEntry {
    std::string playerName;
    int score;
    std::string difficulty;
    
    HighScoreEntry() : playerName(""), score(0), difficulty("Normal") {}
    HighScoreEntry(const std::string& name, int s, const std::string& diff) 
        : playerName(name), score(s), difficulty(diff) {}
};

// Estructura de configuración global
struct GameConfig {
    float musicVolume = 50.0f;
    float sfxVolume = 50.0f;
    std::vector<HighScoreEntry> highScores;
};

// Estructura para almacenar información de personajes
struct CharacterInfo {
    std::string name;
    std::string texturePath;
    sf::Texture texture;
    int numFrames;
};

// Funciones para guardar y cargar configuración
void saveConfig(const GameConfig& config) {
    std::ofstream file("game_config.dat");
    if (file.is_open()) {
        file << config.musicVolume << "\n";
        file << config.sfxVolume << "\n";
        file << config.highScores.size() << "\n";
        
        for (const auto& entry : config.highScores) {
            file << entry.playerName << "\n";
            file << entry.score << "\n";
            file << entry.difficulty << "\n";
        }
        file.close();
    }
}

void loadConfig(GameConfig& config) {
    std::ifstream file("game_config.dat");
    if (file.is_open()) {
        std::string line;
        
        // Leer volúmenes
        std::getline(file, line);
        config.musicVolume = std::stof(line);
        std::getline(file, line);
        config.sfxVolume = std::stof(line);
        
        // Leer número de scores
        std::getline(file, line);
        size_t numScores = std::stoi(line);
        
        config.highScores.clear();
        for (size_t i = 0; i < numScores; ++i) {
            HighScoreEntry entry;
            std::getline(file, entry.playerName);
            std::getline(file, line);
            entry.score = std::stoi(line);
            std::getline(file, entry.difficulty);
            config.highScores.push_back(entry);
        }
        file.close();
    }
}

// Función para agregar récord con nombre
void addHighScore(GameConfig& config, const std::string& playerName, int score, GameDifficulty difficulty) {
    std::string diffStr;
    switch (difficulty) {
        case GameDifficulty::EASY:
            diffStr = "Facil";
            break;
        case GameDifficulty::NORMAL:
            diffStr = "Normal";
            break;
        case GameDifficulty::HARD:
            diffStr = "Dificil";
            break;
    }
    
    config.highScores.push_back(HighScoreEntry(playerName, score, diffStr));
    
    std::sort(config.highScores.begin(), config.highScores.end(),
              [](const HighScoreEntry& a, const HighScoreEntry& b) {
                  return a.score > b.score;
              });
    
    if (config.highScores.size() > 10) {
        config.highScores.resize(10);
    }
    
    saveConfig(config);
}

class Projectile {
public:
    sf::CircleShape shape;
    sf::CircleShape glow;
    sf::Vector2f velocity;
    bool active;
    int direction;

    Projectile(float x, float y, int dir) {
        direction = dir;
        shape.setRadius(6);
        shape.setPosition(sf::Vector2f(x, y));
        shape.setFillColor(sf::Color(255, 150, 0));
        shape.setOutlineColor(sf::Color(255, 200, 0));
        shape.setOutlineThickness(2);
        
        glow.setRadius(10);
        glow.setPosition(sf::Vector2f(x - 4, y - 4));
        glow.setFillColor(sf::Color(255, 100, 0, 100));
        
        velocity = sf::Vector2f(15.0f * direction, 0.0f);
        active = true;
    }

    void update() {
        shape.move(velocity);
        glow.setPosition(sf::Vector2f(shape.getPosition().x - 4, shape.getPosition().y - 4));

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
    sf::Texture* walkTexture;
    sf::Sprite sprite;
    float velocityY;
    float x, y;
    bool isJumping;
    bool isDucking;
    int animationFrame;
    int facingDirection;
    sf::Clock animationClock;
    sf::Clock shootClock;
    int numFrames;
    float spriteScale;
    float shootCooldownTime;

    Dino(float startX, float startY, sf::Texture* texture, int frames, float cooldown = 0.25f) : sprite(*texture) {
        walkTexture = texture;
        x = startX;
        y = startY;
        facingDirection = 1;
        numFrames = frames;
        spriteScale = 0.6f;
        shootCooldownTime = cooldown;
        
        sprite.setTexture(*walkTexture);
        sprite.setScale(sf::Vector2f(spriteScale, spriteScale));
        
        sf::Vector2u texSize = walkTexture->getSize();
        int frameWidth = texSize.x / numFrames;
        
        // Recortar sprite: usar 60% desde más abajo (eliminar 15% arriba, 25% abajo)
        int visibleHeight = static_cast<int>(texSize.y * 0.6f);
        int offsetY = static_cast<int>(texSize.y * 0.15f);
        
        // Recortar también los lados si hay espacio extra
        int visibleWidth = static_cast<int>(frameWidth * 0.8f);
        int offsetX = static_cast<int>(frameWidth * 0.1f);
        
        sprite.setTextureRect(sf::IntRect(sf::Vector2i(offsetX, offsetY), sf::Vector2i(visibleWidth, visibleHeight)));
        
        // Origen en la base de la parte visible
        sprite.setOrigin(sf::Vector2f(visibleWidth / 2.0f, visibleHeight));

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
        x -= 8.0f;
        if (x < 50) x = 50;
        facingDirection = -1;
    }

    void moveRight() {
        x += 8.0f;
        if (x > WINDOW_WIDTH - 50) x = WINDOW_WIDTH - 50;
        facingDirection = 1;
    }

    void update(float groundY, bool fastFall = false) {
        if (isJumping) {
            if (fastFall) {
                velocityY += GRAVITY * 4.0f; // Caída más rápida al presionar abajo/S
            } else {
                velocityY += GRAVITY;
            }
            y += velocityY;

            if (y >= groundY) {
                y = groundY;
                velocityY = 0;
                isJumping = false;
            }
        }

        sprite.setTexture(*walkTexture);
        sf::Vector2u texSize = walkTexture->getSize();
        int frameWidth = texSize.x / numFrames;
        
        // Recortar sprite: usar 60% desde más abajo (eliminar 15% arriba, 25% abajo)
        int visibleHeight = static_cast<int>(texSize.y * 0.6f);
        int offsetY = static_cast<int>(texSize.y * 0.15f);
        
        // Recortar también los lados si hay espacio extra
        int visibleWidth = static_cast<int>(frameWidth * 0.8f);
        int offsetX = static_cast<int>(frameWidth * 0.1f);
        
        sprite.setOrigin(sf::Vector2f(visibleWidth / 2.0f, visibleHeight));
        
        if (animationClock.getElapsedTime().asSeconds() > 0.12f && !isJumping) {
            animationFrame = (animationFrame + 1) % numFrames;
            sprite.setTextureRect(sf::IntRect(sf::Vector2i(animationFrame * frameWidth + offsetX, offsetY), sf::Vector2i(visibleWidth, visibleHeight)));
            animationClock.restart();
        }
        
        // Aplicar escala según la dirección
        if (facingDirection == -1) {
            sprite.setScale(sf::Vector2f(-spriteScale, spriteScale));
        } else {
            sprite.setScale(sf::Vector2f(spriteScale, spriteScale));
        }
        
        // IMPORTANTE: Actualizar posición del sprite con las coordenadas x, y
        sprite.setPosition(sf::Vector2f(x, y));
    }

    bool canShoot() {
        return shootClock.getElapsedTime().asSeconds() > shootCooldownTime;
    }

    void resetShootClock() {
        shootClock.restart();
    }

    void draw(sf::RenderWindow& window) {
        window.draw(sprite);
    }

    sf::FloatRect getBounds() const {
        sf::FloatRect bounds = sprite.getGlobalBounds();
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
        float shootX = facingDirection == 1 ? x + (bounds.size.x * 0.4f) : x - (bounds.size.x * 0.4f);
        return sf::Vector2f(shootX, shootY);
    }
};

class Explosion {
public:
    std::vector<sf::CircleShape> particles;
    std::vector<sf::Vector2f> velocities;
    sf::Clock lifetime;
    bool active;

    Explosion(float x, float y) {
        active = true;
        int numParticles = 20;
        
        for (int i = 0; i < numParticles; ++i) {
            sf::CircleShape particle(3);
            particle.setFillColor(sf::Color(255, 100 + rand() % 156, 0));
            particle.setPosition(sf::Vector2f(x, y));
            particles.push_back(particle);
            
            float angle = (rand() % 360) * 3.14159f / 180.0f;
            float speed = 2.0f + (rand() % 3);
            velocities.push_back(sf::Vector2f(cos(angle) * speed, sin(angle) * speed));
        }
    }

    void update() {
        for (size_t i = 0; i < particles.size(); ++i) {
            particles[i].move(velocities[i]);
            velocities[i].y += 0.2f;
        }

        if (lifetime.getElapsedTime().asSeconds() > 0.5f) {
            active = false;
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

class Enemy {
public:
    sf::Sprite sprite;
    sf::Texture* texture;
    float x, y;
    float speed;
    bool active;
    int type;
    int numFrames;
    int currentFrame;
    sf::Clock animClock;
    float spriteScale;

    Enemy(float startX, float groundY, sf::Texture* tex, int frames, int enemyType) : sprite(*tex) {
        x = startX;
        type = enemyType; // 0=Gengar, 1=Camioneta, 2=Mewtwo
        active = true;
        speed = 3.0f + (rand() % 3) * 0.5f;
        texture = tex;
        numFrames = frames;
        currentFrame = 0;
        
        // Calcular escala y posición Y según el tipo de enemigo
        sf::Vector2u texSize = texture->getSize();
        float targetHeight;
        
        if (type == 0) { // Gengar - más grande y mucho más abajo
            targetHeight = 180.0f;
            y = 600.0f; // Posición Y mucho más abajo para confirmar cambio
        } else if (type == 1) { // Camioneta - más abajo
            targetHeight = 140.0f;
            y = 595.0f; // Posición Y mucho más abajo para confirmar cambio
        } else { // Mewtwo
            targetHeight = 150.0f;
            y = 590.0f; // Posición Y más abajo
        }
        
        spriteScale = targetHeight / texSize.y;
        
        sprite.setTexture(*texture);
        sprite.setScale(sf::Vector2f(spriteScale, spriteScale));
        
        int frameWidth = texSize.x / numFrames;
        
        sprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(frameWidth, texSize.y)));
        
        // Origen en la base del sprite para que todos toquen el piso
        sprite.setOrigin(sf::Vector2f(frameWidth / 2.0f, texSize.y));
        sprite.setPosition(sf::Vector2f(x, y));
    }

    void update(float speedMultiplier = 1.0f) {
        x -= speed * speedMultiplier;
        sprite.setPosition(sf::Vector2f(x, y));
        
        // Animar sprite
        if (animClock.getElapsedTime().asSeconds() > 0.12f) {
            currentFrame = (currentFrame + 1) % numFrames;
            sf::Vector2u texSize = texture->getSize();
            int frameWidth = texSize.x / numFrames;
            sprite.setTextureRect(sf::IntRect(sf::Vector2i(currentFrame * frameWidth, 0), sf::Vector2i(frameWidth, texSize.y)));
            animClock.restart();
        }

        if (x < -100) {
            active = false;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (active) {
            window.draw(sprite);
        }
    }

    sf::FloatRect getBounds() const {
        return sprite.getGlobalBounds();
    }
};

// Función para mostrar el menú principal
MenuState showMainMenu(sf::RenderWindow& window, sf::Music& menuMusic, GameConfig& config) {
    // Cargar fondo del menú
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("assets/images/Menu principal.png")) {
        return MenuState::PLAYING; // Si falla, ir directo al juego
    }
    sf::Sprite backgroundSprite(backgroundTexture);
    
    // Escalar fondo
    sf::Vector2u bgSize = backgroundTexture.getSize();
    float scaleX = static_cast<float>(WINDOW_WIDTH) / bgSize.x;
    float scaleY = static_cast<float>(WINDOW_HEIGHT) / bgSize.y;
    float scale = std::max(scaleX, scaleY);
    backgroundSprite.setScale(sf::Vector2f(scale, scale));
    backgroundSprite.setPosition(sf::Vector2f(
        (WINDOW_WIDTH - bgSize.x * scale) / 2.0f,
        (WINDOW_HEIGHT - bgSize.y * scale) / 2.0f
    ));
    
    // Cargar fuente
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return MenuState::PLAYING;
    }
    
    // Título
    sf::Text titleText(font);
    titleText.setString("PockyMan: Asalto a la Pokeplaza");
    titleText.setCharacterSize(45);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3);
    titleText.setPosition(sf::Vector2f(150, 80));
    
    // Opciones del menú
    std::vector<std::string> menuOptions = {
        "1. Empezar Juego",
        "2. Configuraciones",
        "3. Registro de Record"
    };
    
    std::vector<sf::Text> menuTexts;
    int selectedOption = 0;
    
    for (size_t i = 0; i < menuOptions.size(); ++i) {
        sf::Text text(font);
        text.setString(menuOptions[i]);
        text.setCharacterSize(35);
        text.setFillColor(sf::Color::White);
        text.setPosition(sf::Vector2f(300, 250 + i * 80));
        menuTexts.push_back(text);
    }
    
    // Overlay semitransparente
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return MenuState::MAIN_MENU;
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Up) {
                    selectedOption = (selectedOption - 1 + menuOptions.size()) % menuOptions.size();
                } else if (keyPressed->code == sf::Keyboard::Key::Down) {
                    selectedOption = (selectedOption + 1) % menuOptions.size();
                } else if (keyPressed->code == sf::Keyboard::Key::Enter || 
                           keyPressed->code == sf::Keyboard::Key::Num1 ||
                           keyPressed->code == sf::Keyboard::Key::Num2 ||
                           keyPressed->code == sf::Keyboard::Key::Num3) {
                    
                    if (keyPressed->code == sf::Keyboard::Key::Num1) selectedOption = 0;
                    else if (keyPressed->code == sf::Keyboard::Key::Num2) selectedOption = 1;
                    else if (keyPressed->code == sf::Keyboard::Key::Num3) selectedOption = 2;
                    
                    if (selectedOption == 0) return MenuState::DIFFICULTY_SELECT;
                    else if (selectedOption == 1) return MenuState::SETTINGS;
                    else if (selectedOption == 2) return MenuState::HIGH_SCORES;
                }
            }
        }
        
        // Actualizar colores de selección
        for (size_t i = 0; i < menuTexts.size(); ++i) {
            if (i == selectedOption) {
                menuTexts[i].setFillColor(sf::Color::Yellow);
                menuTexts[i].setOutlineColor(sf::Color::Black);
                menuTexts[i].setOutlineThickness(2);
            } else {
                menuTexts[i].setFillColor(sf::Color::White);
                menuTexts[i].setOutlineThickness(0);
            }
        }
        
        window.clear();
        window.draw(backgroundSprite);
        window.draw(overlay);
        window.draw(titleText);
        for (auto& text : menuTexts) {
            window.draw(text);
        }
        window.display();
    }
    
    return MenuState::MAIN_MENU;
}

// Función para seleccionar dificultad
GameDifficulty showDifficultySelect(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return GameDifficulty::NORMAL;
    }
    
    sf::Text titleText(font);
    titleText.setString("SELECCIONA LA DIFICULTAD");
    titleText.setCharacterSize(40);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setPosition(sf::Vector2f(220, 100));
    
    std::vector<std::string> difficulties = {
        "1. FACIL - Todo mas lento, menos puntos",
        "2. NORMAL - Dificultad balanceada",
        "3. DIFICIL - Todo mas rapido, cooldown en disparos"
    };
    
    std::vector<sf::Text> diffTexts;
    int selectedDiff = 1; // Normal por defecto
    
    for (size_t i = 0; i < difficulties.size(); ++i) {
        sf::Text text(font);
        text.setString(difficulties[i]);
        text.setCharacterSize(28);
        text.setFillColor(sf::Color::White);
        text.setPosition(sf::Vector2f(150, 250 + i * 80));
        diffTexts.push_back(text);
    }
    
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return GameDifficulty::NORMAL;
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Up) {
                    selectedDiff = (selectedDiff - 1 + 3) % 3;
                } else if (keyPressed->code == sf::Keyboard::Key::Down) {
                    selectedDiff = (selectedDiff + 1) % 3;
                } else if (keyPressed->code == sf::Keyboard::Key::Enter ||
                           keyPressed->code == sf::Keyboard::Key::Num1 ||
                           keyPressed->code == sf::Keyboard::Key::Num2 ||
                           keyPressed->code == sf::Keyboard::Key::Num3) {
                    
                    if (keyPressed->code == sf::Keyboard::Key::Num1) return GameDifficulty::EASY;
                    else if (keyPressed->code == sf::Keyboard::Key::Num2) return GameDifficulty::NORMAL;
                    else if (keyPressed->code == sf::Keyboard::Key::Num3) return GameDifficulty::HARD;
                    else return static_cast<GameDifficulty>(selectedDiff);
                }
            }
        }
        
        for (size_t i = 0; i < diffTexts.size(); ++i) {
            if (i == selectedDiff) {
                diffTexts[i].setFillColor(sf::Color::Yellow);
            } else {
                diffTexts[i].setFillColor(sf::Color::White);
            }
        }
        
        window.clear(sf::Color(20, 20, 40));
        window.draw(titleText);
        for (auto& text : diffTexts) {
            window.draw(text);
        }
        window.display();
    }
    
    return GameDifficulty::NORMAL;
}

// Función para mostrar configuraciones
void showSettings(sf::RenderWindow& window, GameConfig& config) {
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return;
    }
    
    sf::Text titleText(font);
    titleText.setString("CONFIGURACIONES");
    titleText.setCharacterSize(40);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setPosition(sf::Vector2f(320, 80));
    
    sf::Text musicText(font);
    musicText.setCharacterSize(30);
    musicText.setFillColor(sf::Color::White);
    musicText.setPosition(sf::Vector2f(200, 200));
    
    sf::Text sfxText(font);
    sfxText.setCharacterSize(30);
    sfxText.setFillColor(sf::Color::White);
    sfxText.setPosition(sf::Vector2f(200, 280));
    
    sf::Text instructionText(font);
    instructionText.setString("Usa Flechas para ajustar, ESC para salir");
    instructionText.setCharacterSize(20);
    instructionText.setFillColor(sf::Color(200, 200, 200));
    instructionText.setPosition(sf::Vector2f(250, 450));
    
    int selectedSetting = 0; // 0 = música, 1 = sfx
    
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return;
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    saveConfig(config);
                    return;
                } else if (keyPressed->code == sf::Keyboard::Key::Up) {
                    selectedSetting = 0;
                } else if (keyPressed->code == sf::Keyboard::Key::Down) {
                    selectedSetting = 1;
                } else if (keyPressed->code == sf::Keyboard::Key::Left) {
                    if (selectedSetting == 0) {
                        config.musicVolume = std::max(0.0f, config.musicVolume - 5.0f);
                    } else {
                        config.sfxVolume = std::max(0.0f, config.sfxVolume - 5.0f);
                    }
                } else if (keyPressed->code == sf::Keyboard::Key::Right) {
                    if (selectedSetting == 0) {
                        config.musicVolume = std::min(100.0f, config.musicVolume + 5.0f);
                    } else {
                        config.sfxVolume = std::min(100.0f, config.sfxVolume + 5.0f);
                    }
                }
            }
        }
        
        musicText.setString("Volumen Musica: " + std::to_string(static_cast<int>(config.musicVolume)) + "%");
        musicText.setFillColor(selectedSetting == 0 ? sf::Color::Yellow : sf::Color::White);
        
        sfxText.setString("Volumen Efectos: " + std::to_string(static_cast<int>(config.sfxVolume)) + "%");
        sfxText.setFillColor(selectedSetting == 1 ? sf::Color::Yellow : sf::Color::White);
        
        window.clear(sf::Color(20, 20, 40));
        window.draw(titleText);
        window.draw(musicText);
        window.draw(sfxText);
        window.draw(instructionText);
        window.display();
    }
}

// Función para mostrar récords
void showHighScores(sf::RenderWindow& window, const GameConfig& config) {
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return;
    }
    
    // Cargar fondo
    sf::Texture bgTexture;
    bool hasBackground = bgTexture.loadFromFile("assets/images/Menu principal.png");
    sf::Sprite bgSprite(bgTexture);
    if (hasBackground) {
        float scaleX = static_cast<float>(WINDOW_WIDTH) / bgTexture.getSize().x;
        float scaleY = static_cast<float>(WINDOW_HEIGHT) / bgTexture.getSize().y;
        bgSprite.setScale(sf::Vector2f(scaleX, scaleY));
    }
    
    sf::Text titleText(font);
    titleText.setString("REGISTRO DE RECORD - TOP 10");
    titleText.setCharacterSize(40);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setPosition(sf::Vector2f(200, 50));
    
    std::vector<sf::Text> scoreTexts;
    
    // Mostrar siempre 10 lugares
    for (size_t i = 0; i < 10; ++i) {
        std::string scoreStr;
        
        if (i < config.highScores.size()) {
            const auto& entry = config.highScores[i];
            scoreStr = std::to_string(i + 1) + ". " + 
                      entry.playerName + " - " + 
                      std::to_string(entry.score) + " pts (" + 
                      entry.difficulty + ")";
        } else {
            scoreStr = std::to_string(i + 1) + ". ---";
        }
        
        sf::Text text(font);
        text.setString(scoreStr);
        text.setCharacterSize(22);
        
        if (i == 0) {
            text.setFillColor(sf::Color(255, 215, 0));
        } else if (i == 1) {
            text.setFillColor(sf::Color(192, 192, 192));
        } else if (i == 2) {
            text.setFillColor(sf::Color(205, 127, 50));
        } else {
            text.setFillColor(sf::Color::White);
        }
        
        text.setPosition(sf::Vector2f(120, 140 + i * 35));
        scoreTexts.push_back(text);
    }
    
    sf::Text backText(font);
    backText.setString("Presiona ESC para volver");
    backText.setCharacterSize(20);
    backText.setFillColor(sf::Color(200, 200, 200));
    backText.setPosition(sf::Vector2f(320, 540));
    
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return;
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    return;
                }
            }
        }
        
        window.clear(sf::Color(20, 20, 40));
        window.draw(bgSprite);
        
        sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 120));
        window.draw(overlay);
        
        window.draw(titleText);
        for (auto& text : scoreTexts) {
            window.draw(text);
        }
        window.draw(backText);
        window.display();
    }
}

// Estructura para retornar opciones de game over
struct GameOverResult {
    std::string playerName;
    int choice; // 0=Reintentar, 1=Ver Records, 2=Menu, -1=ESC sin guardar
};

// Declaración anticipada
GameOverResult showPostGameMenu(sf::RenderWindow& window, const std::string& playerName, int finalScore);

// Pantalla de Game Over con input de nombre
GameOverResult showGameOver(sf::RenderWindow& window, int finalScore, GameDifficulty difficulty) {
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return {"Player", -1};
    }
    
    sf::Texture bgTexture;
    bool hasBackground = bgTexture.loadFromFile("assets/images/Menu principal.png");
    sf::Sprite bgSprite(bgTexture);
    if (hasBackground) {
        float scaleX = static_cast<float>(WINDOW_WIDTH) / bgTexture.getSize().x;
        float scaleY = static_cast<float>(WINDOW_HEIGHT) / bgTexture.getSize().y;
        bgSprite.setScale(sf::Vector2f(scaleX, scaleY));
    }
    
    sf::Text titleText(font);
    titleText.setString("GAME OVER");
    titleText.setCharacterSize(60);
    titleText.setFillColor(sf::Color::Red);
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3);
    titleText.setPosition(sf::Vector2f(320, 100));
    
    sf::Text scoreText(font);
    scoreText.setString("Puntuacion Final: " + std::to_string(finalScore));
    scoreText.setCharacterSize(35);
    scoreText.setFillColor(sf::Color::Yellow);
    scoreText.setPosition(sf::Vector2f(250, 220));
    
    sf::Text promptText(font);
    promptText.setString("Ingresa tu nombre:");
    promptText.setCharacterSize(28);
    promptText.setFillColor(sf::Color::White);
    promptText.setPosition(sf::Vector2f(330, 300));
    
    std::string playerName = "";
    sf::Text nameInputText(font);
    nameInputText.setCharacterSize(30);
    nameInputText.setFillColor(sf::Color::Cyan);
    nameInputText.setPosition(sf::Vector2f(350, 350));
    
    sf::Text instructionText(font);
    instructionText.setString("Escribe tu nombre y presiona ENTER (max 15 caracteres)");
    instructionText.setCharacterSize(18);
    instructionText.setFillColor(sf::Color(200, 200, 200));
    instructionText.setPosition(sf::Vector2f(180, 450));
    
    sf::Text skipText(font);
    skipText.setString("R: Reintentar | ESC: Menu");
    skipText.setCharacterSize(16);
    skipText.setFillColor(sf::Color(150, 150, 150));
    skipText.setPosition(sf::Vector2f(360, 500));
    
    bool nameEntered = false;
    
    while (window.isOpen() && !nameEntered) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return {"Player", -1};
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Enter && !playerName.empty()) {
                    nameEntered = true;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    return {"", -1};
                }
                else if (keyPressed->code == sf::Keyboard::Key::Backspace && !playerName.empty()) {
                    playerName.pop_back();
                }
            }
            
            if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                if (textEntered->unicode < 128 && playerName.length() < 15) {
                    char typed = static_cast<char>(textEntered->unicode);
                    if ((typed >= 'a' && typed <= 'z') || 
                        (typed >= 'A' && typed <= 'Z') || 
                        (typed >= '0' && typed <= '9') || 
                        typed == ' ') {
                        playerName += typed;
                    }
                }
            }
        }
        
        nameInputText.setString(playerName + "_");
        
        window.clear(sf::Color(20, 20, 40));
        window.draw(bgSprite);
        
        sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);
        
        window.draw(titleText);
        window.draw(scoreText);
        window.draw(promptText);
        window.draw(nameInputText);
        window.draw(instructionText);
        window.draw(skipText);
        window.display();
    }
    
    // Ahora mostrar opciones después de guardar
    return showPostGameMenu(window, playerName.empty() ? "Anonimo" : playerName, finalScore);
}

// Menú después de registrar el nombre
GameOverResult showPostGameMenu(sf::RenderWindow& window, const std::string& playerName, int finalScore) {
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return {playerName, 2};
    }
    
    sf::Texture bgTexture;
    bool hasBackground = bgTexture.loadFromFile("assets/images/Menu principal.png");
    sf::Sprite bgSprite(bgTexture);
    if (hasBackground) {
        float scaleX = static_cast<float>(WINDOW_WIDTH) / bgTexture.getSize().x;
        float scaleY = static_cast<float>(WINDOW_HEIGHT) / bgTexture.getSize().y;
        bgSprite.setScale(sf::Vector2f(scaleX, scaleY));
    }
    
    sf::Text titleText(font);
    titleText.setString("RECORD GUARDADO!");
    titleText.setCharacterSize(50);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3);
    titleText.setPosition(sf::Vector2f(250, 100));
    
    sf::Text nameText(font);
    nameText.setString("Jugador: " + playerName);
    nameText.setCharacterSize(30);
    nameText.setFillColor(sf::Color::White);
    nameText.setPosition(sf::Vector2f(350, 200));
    
    sf::Text scoreText(font);
    scoreText.setString("Puntuacion: " + std::to_string(finalScore));
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color::Cyan);
    scoreText.setPosition(sf::Vector2f(330, 250));
    
    std::vector<std::string> options = {
        "1. Reintentar",
        "2. Ver Records",
        "3. Menu Principal"
    };
    
    std::vector<sf::Text> optionTexts;
    int selectedOption = 0;
    
    for (size_t i = 0; i < options.size(); ++i) {
        sf::Text text(font);
        text.setString(options[i]);
        text.setCharacterSize(28);
        text.setFillColor(i == 0 ? sf::Color::Cyan : sf::Color::White);
        text.setPosition(sf::Vector2f(350, 350 + i * 60));
        optionTexts.push_back(text);
    }
    
    sf::Text instructionText(font);
    instructionText.setString("Usa FLECHAS o numeros, ENTER para confirmar");
    instructionText.setCharacterSize(18);
    instructionText.setFillColor(sf::Color(200, 200, 200));
    instructionText.setPosition(sf::Vector2f(250, 530));
    
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return {playerName, 2};
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Up) {
                    selectedOption = (selectedOption - 1 + 3) % 3;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Down) {
                    selectedOption = (selectedOption + 1) % 3;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Num1) {
                    return {playerName, 0};
                }
                else if (keyPressed->code == sf::Keyboard::Key::Num2) {
                    return {playerName, 1};
                }
                else if (keyPressed->code == sf::Keyboard::Key::Num3) {
                    return {playerName, 2};
                }
                else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                    return {playerName, selectedOption};
                }
            }
        }
        
        // Actualizar colores de selección
        for (size_t i = 0; i < optionTexts.size(); ++i) {
            if (i == selectedOption) {
                optionTexts[i].setFillColor(sf::Color::Cyan);
                optionTexts[i].setOutlineColor(sf::Color::Black);
                optionTexts[i].setOutlineThickness(2);
            } else {
                optionTexts[i].setFillColor(sf::Color::White);
                optionTexts[i].setOutlineThickness(0);
            }
        }
        
        window.clear(sf::Color(20, 20, 40));
        window.draw(bgSprite);
        
        sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);
        
        window.draw(titleText);
        window.draw(nameText);
        window.draw(scoreText);
        for (auto& text : optionTexts) {
            window.draw(text);
        }
        window.draw(instructionText);
        window.display();
    }
    
    return {playerName, 2};
}

// Función para mostrar el menú de selección de personaje
int showCharacterSelection(sf::RenderWindow& window) {
    // La música del menú principal sigue sonando durante la selección de personaje
    
    // Cargar fondo principal
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("assets/images/Fondo principal.png")) {
        return 0; // Error cargando fondo
    }
    sf::Sprite backgroundSprite(backgroundTexture);
    
    // Escalar el fondo para que cubra toda la ventana sin distorsión
    sf::Vector2u bgSize = backgroundTexture.getSize();
    float scaleX = static_cast<float>(WINDOW_WIDTH) / bgSize.x;
    float scaleY = static_cast<float>(WINDOW_HEIGHT) / bgSize.y;
    float scale = std::max(scaleX, scaleY); // Usar el mayor para cubrir toda la ventana
    backgroundSprite.setScale(sf::Vector2f(scale, scale));
    
    // Centrar el fondo si es necesario
    float bgWidth = bgSize.x * scale;
    float bgHeight = bgSize.y * scale;
    backgroundSprite.setPosition(sf::Vector2f(
        (WINDOW_WIDTH - bgWidth) / 2.0f,
        (WINDOW_HEIGHT - bgHeight) / 2.0f
    ));
    
    // Cargar personajes
    CharacterInfo pika, ballesta;
    
    pika.name = "PIKA";
    pika.texturePath = "assets/images/PIKACHU (2) (1).png";
    pika.numFrames = 4;
    
    ballesta.name = "Umbreon";
    ballesta.texturePath = "assets/images/Ballesta .png";
    ballesta.numFrames = 4;
    
    if (!pika.texture.loadFromFile(pika.texturePath) || 
        !ballesta.texture.loadFromFile(ballesta.texturePath)) {
        return 0; // Error cargando texturas
    }
    
    // Configurar sprites de vista previa
    sf::Sprite pikaSprite(pika.texture);
    sf::Sprite ballestaSprite(ballesta.texture);
    
    // Configurar escala y posición para Pika (izquierda)
    sf::Vector2u pikaSize = pika.texture.getSize();
    int pikaFrameWidth = pikaSize.x / pika.numFrames;
    pikaSprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(pikaFrameWidth, pikaSize.y)));
    float pikaScale = 250.0f / pikaSize.y;
    pikaSprite.setScale(sf::Vector2f(pikaScale, pikaScale));
    pikaSprite.setPosition(sf::Vector2f(200, 250));
    
    // Configurar escala y posición para Ballesta (derecha)
    sf::Vector2u ballestaSize = ballesta.texture.getSize();
    int ballestaFrameWidth = ballestaSize.x / ballesta.numFrames;
    ballestaSprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(ballestaFrameWidth, ballestaSize.y)));
    float ballestaScale = 250.0f / ballestaSize.y;
    ballestaSprite.setScale(sf::Vector2f(ballestaScale, ballestaScale));
    ballestaSprite.setPosition(sf::Vector2f(600, 250));
    
    // Cargar fuente
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return 0;
    }
    
    // Textos
    sf::Text titleText(font);
    titleText.setString("SELECCIONA TU PERSONAJE");
    titleText.setCharacterSize(40);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(sf::Vector2f(250, 50));
    
    sf::Text pikaText(font);
    pikaText.setString("PIKA");
    pikaText.setCharacterSize(30);
    pikaText.setFillColor(sf::Color::White);
    pikaText.setPosition(sf::Vector2f(210, 450));
    
    sf::Text ballestaText(font);
    ballestaText.setString("UMBREON");
    ballestaText.setCharacterSize(30);
    ballestaText.setFillColor(sf::Color::White);
    ballestaText.setPosition(sf::Vector2f(590, 450));
    
    sf::Text instructionText(font);
    instructionText.setString("Presiona 1 para PIKA o 2 para UMBREON");
    instructionText.setCharacterSize(20);
    instructionText.setFillColor(sf::Color(200, 200, 200));
    instructionText.setPosition(sf::Vector2f(280, 520));
    
    // Indicadores de selección (marcos)
    sf::RectangleShape pikaFrame(sf::Vector2f(pikaFrameWidth * pikaScale + 20, pikaSize.y * pikaScale + 20));
    pikaFrame.setFillColor(sf::Color::Transparent);
    pikaFrame.setOutlineColor(sf::Color::Yellow);
    pikaFrame.setOutlineThickness(5);
    pikaFrame.setPosition(sf::Vector2f(190, 240));
    
    sf::RectangleShape ballestaFrame(sf::Vector2f(ballestaFrameWidth * ballestaScale + 20, ballestaSize.y * ballestaScale + 20));
    ballestaFrame.setFillColor(sf::Color::Transparent);
    ballestaFrame.setOutlineColor(sf::Color::Yellow);
    ballestaFrame.setOutlineThickness(5);
    ballestaFrame.setPosition(sf::Vector2f(590, 240));
    
    // Overlay semitransparente para mejorar la legibilidad
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 120)); // Negro con 47% de opacidad
    
    int selectedCharacter = -1;
    int hoveredCharacter = 0; // 0 = pika, 1 = ballesta
    
    sf::Clock animClock;
    int animFrame = 0;
    
    while (window.isOpen() && selectedCharacter == -1) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return -1;
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Num1 || keyPressed->code == sf::Keyboard::Key::Numpad1) {
                    selectedCharacter = 0; // Pika
                } else if (keyPressed->code == sf::Keyboard::Key::Num2 || keyPressed->code == sf::Keyboard::Key::Numpad2) {
                    selectedCharacter = 1; // Ballesta
                } else if (keyPressed->code == sf::Keyboard::Key::Left) {
                    hoveredCharacter = 0;
                } else if (keyPressed->code == sf::Keyboard::Key::Right) {
                    hoveredCharacter = 1;
                } else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                    selectedCharacter = hoveredCharacter;
                }
            }
        }
        
        // Animación de sprites
        if (animClock.getElapsedTime().asSeconds() > 0.15f) {
            animFrame = (animFrame + 1) % 4;
            pikaSprite.setTextureRect(sf::IntRect(sf::Vector2i(animFrame * pikaFrameWidth, 0), sf::Vector2i(pikaFrameWidth, pikaSize.y)));
            ballestaSprite.setTextureRect(sf::IntRect(sf::Vector2i(animFrame * ballestaFrameWidth, 0), sf::Vector2i(ballestaFrameWidth, ballestaSize.y)));
            animClock.restart();
        }
        
        // Dibujar
        window.clear();
        
        // Dibujar fondo principal
        window.draw(backgroundSprite);
        
        // Dibujar overlay semitransparente
        window.draw(overlay);
        
        // Dibujar marcos de selección
        if (hoveredCharacter == 0) {
            window.draw(pikaFrame);
        } else {
            window.draw(ballestaFrame);
        }
        
        // Dibujar sprites
        window.draw(pikaSprite);
        window.draw(ballestaSprite);
        
        // Dibujar textos
        window.draw(titleText);
        window.draw(pikaText);
        window.draw(ballestaText);
        window.draw(instructionText);
        
        window.display();
    }
    
    return selectedCharacter;
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    // Cargar configuración
    GameConfig gameConfig;
    loadConfig(gameConfig);

    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "PockyMan: Asalto a la Pokeplaza");
    window.setFramerateLimit(60);

    // Cargar música del menú principal
    sf::Music menuMusic;
    if (menuMusic.openFromFile("assets/music/Selecciona-tu-personaje.ogg")) {
        menuMusic.setLooping(true);
        menuMusic.setVolume(gameConfig.musicVolume);
        menuMusic.play();
    }

    // Estado del juego
    MenuState currentState = MenuState::MAIN_MENU;
    GameDifficulty difficulty = GameDifficulty::NORMAL;
    int selectedCharacter = -1;
    bool shouldExit = false;

    // Bucle principal del menú
CONTINUE_MENU_LOOP:
    while (window.isOpen()) {
        switch (currentState) {
            case MenuState::MAIN_MENU: {
                currentState = showMainMenu(window, menuMusic, gameConfig);
                if (!window.isOpen()) {
                    saveConfig(gameConfig);
                    return 0;
                }
                break;
            }
            
            case MenuState::DIFFICULTY_SELECT: {
                difficulty = showDifficultySelect(window);
                if (!window.isOpen()) {
                    saveConfig(gameConfig);
                    return 0;
                } else {
                    currentState = MenuState::CHARACTER_SELECT;
                }
                break;
            }
            
            case MenuState::CHARACTER_SELECT: {
                selectedCharacter = showCharacterSelection(window);
                if (selectedCharacter == -1 || !window.isOpen()) {
                    saveConfig(gameConfig);
                    return 0;
                } else {
                    currentState = MenuState::PLAYING;
                }
                break;
            }
            
            case MenuState::SETTINGS: {
                showSettings(window, gameConfig);
                if (!window.isOpen()) {
                    saveConfig(gameConfig);
                    return 0;
                } else {
                    menuMusic.setVolume(gameConfig.musicVolume);
                    currentState = MenuState::MAIN_MENU;
                }
                break;
            }
            
            case MenuState::HIGH_SCORES: {
                showHighScores(window, gameConfig);
                if (!window.isOpen()) {
                    saveConfig(gameConfig);
                    return 0;
                } else {
                    currentState = MenuState::MAIN_MENU;
                }
                break;
            }
            
            case MenuState::PLAYING: {
                menuMusic.stop();
                goto START_GAME;
            }
        }
    }
    
    saveConfig(gameConfig);
    return 0;

START_GAME:
    // === INICIO DEL JUEGO ===
    
    if (selectedCharacter == -1) {
        return 0; // Ventana cerrada durante la selección
    }
    
    // Cargar textura del personaje seleccionado
    sf::Texture characterTexture;
    int numFrames = 4;
    
    if (selectedCharacter == 0) {
        if (!characterTexture.loadFromFile("assets/images/PIKACHU (2) (1).png")) {
            return -1;
        }
    } else {
        if (!characterTexture.loadFromFile("assets/images/Ballesta .png")) {
            return -1;
        }
    }
    
    // Cargar texturas de enemigos
    sf::Texture gengarTexture, camionetaTexture, mewtwoTexture;
    if (!gengarTexture.loadFromFile("assets/images/Gengar.png")) {
        return -1;
    }
    if (!camionetaTexture.loadFromFile("assets/images/Camioneta FINAL.png")) {
        return -1;
    }
    if (!mewtwoTexture.loadFromFile("assets/images/Mewtwo (1).png")) {
        return -1;
    }
    
    // Array de texturas de enemigos para selección aleatoria
    sf::Texture* enemyTextures[] = {&gengarTexture, &camionetaTexture, &mewtwoTexture};
    int enemyFrames[] = {4, 3, 4}; // Frames por cada enemigo

    // Aplicar modificadores de dificultad
    float difficultySpeedMultiplier = 1.0f;
    float difficultyScoreMultiplier = 1.0f;
    float shootCooldown = 0.25f; // Tiempo entre disparos
    
    switch (difficulty) {
        case GameDifficulty::EASY:
            difficultySpeedMultiplier = 0.7f; // 70% de velocidad
            difficultyScoreMultiplier = 0.5f; // 50% de puntos
            break;
        case GameDifficulty::NORMAL:
            difficultySpeedMultiplier = 1.0f; // 100% de velocidad
            difficultyScoreMultiplier = 1.0f; // 100% de puntos
            break;
        case GameDifficulty::HARD:
            difficultySpeedMultiplier = 1.4f; // 140% de velocidad
            difficultyScoreMultiplier = 1.5f; // 150% de puntos
            shootCooldown = 0.5f; // Doble cooldown en disparos
            break;
    }

    // Calcular posición del suelo - personajes tocan el borde del suelo
    float groundY = WINDOW_HEIGHT - GROUND_HEIGHT;
    
    // Ajustar posición del personaje más abajo
    float playerGroundY = groundY + 70;

    // Crear personaje con la textura seleccionada
    Dino dino(100, playerGroundY, &characterTexture, numFrames, shootCooldown);

    // Cargar fondo
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("assets/images/fondo.png")) {
        return -1;
    }
    sf::Sprite background1(backgroundTexture);
    sf::Sprite background2(backgroundTexture);
    
    // Escalar el fondo para que abarque toda la altura de la ventana
    sf::Vector2u bgSize = backgroundTexture.getSize();
    float scaleY = static_cast<float>(WINDOW_HEIGHT) / bgSize.y;
    float scaleX = scaleY; // Mantener proporción
    background1.setScale(sf::Vector2f(scaleX, scaleY));
    background2.setScale(sf::Vector2f(scaleX, scaleY));
    
    // Posicionar el segundo fondo para scroll continuo
    background2.setPosition(sf::Vector2f(bgSize.x * scaleX, 0));
    float backgroundSpeed = 2.0f;
    float gameSpeedMultiplier = 1.0f * difficultySpeedMultiplier;

    // Cargar músicas del juego
    sf::Music gameMusic1, gameMusic2;
    if (!gameMusic1.openFromFile("assets/music/Jugar1.ogg")) {
        return -1;
    }
    if (!gameMusic2.openFromFile("assets/music/Jugar2.ogg")) {
        return -1;
    }
    
    // Reproducir la primera música del juego
    gameMusic1.setVolume(gameConfig.musicVolume);
    gameMusic2.setVolume(gameConfig.musicVolume);
    gameMusic1.play();
    int currentMusic = 1; // 1 = Jugar1, 2 = Jugar2
    
    // Cargar sonidos de disparo según el personaje
    sf::Music shootSound;
    if (selectedCharacter == 0) {
        // Pikachu usa AK-47
        if (!shootSound.openFromFile("assets/music/AK-47.ogg")) {
            return -1;
        }
    } else {
        // Umbreon usa Ballesta
        if (!shootSound.openFromFile("assets/music/Ballesta sonido.ogg")) {
            return -1;
        }
    }
    shootSound.setLooping(true); // Sonido en bucle mientras se dispara
    shootSound.setVolume(gameConfig.sfxVolume);
    bool isShooting = false;

    // Suelo
    sf::RectangleShape ground(sf::Vector2f(WINDOW_WIDTH, GROUND_HEIGHT));
    ground.setPosition(sf::Vector2f(0, WINDOW_HEIGHT - GROUND_HEIGHT));
    ground.setFillColor(sf::Color(139, 90, 43));

    std::vector<Enemy> enemies;
    std::vector<Projectile> projectiles;
    std::vector<Explosion> explosions;

    sf::Clock enemySpawnClock;
    float spawnInterval = 2.0f;
    int lastEnemyType = -1; // -1=ninguno, 0=Gengar, 1=Camioneta, 2=Mewtwo
    int consecutiveTrucks = 0; // Contador de camionetas consecutivas

    int score = 0;
    int lives = 3;
    int highScore = gameConfig.highScores.empty() ? 0 : gameConfig.highScores[0].score;

    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return -1;
    }

    sf::Text scoreText(font);
    scoreText.setString("Score: 0");
    scoreText.setCharacterSize(24);
    scoreText.setPosition(sf::Vector2f(10, 10));
    scoreText.setFillColor(sf::Color::White);

    sf::Text livesText(font);
    livesText.setString("Lives: 3");
    livesText.setCharacterSize(24);
    livesText.setPosition(sf::Vector2f(10, 40));
    livesText.setFillColor(sf::Color::Red);
    
    sf::Text highScoreText(font);
    highScoreText.setString("High Score: " + std::to_string(highScore));
    highScoreText.setCharacterSize(24);
    highScoreText.setPosition(sf::Vector2f(10, 70));
    highScoreText.setFillColor(sf::Color::Yellow);
    
    // Texto de debug para mostrar posición X del personaje
    sf::Text debugText(font);
    debugText.setCharacterSize(20);
    debugText.setPosition(sf::Vector2f(10, 100));
    debugText.setFillColor(sf::Color::Cyan);

    bool gameOver = false;
    bool isPaused = false;
    
    sf::Text gameOverText(font);
    gameOverText.setString("GAME OVER - Presiona R para registrar tu record");
    gameOverText.setCharacterSize(26);
    gameOverText.setPosition(sf::Vector2f(150, WINDOW_HEIGHT / 2));
    gameOverText.setFillColor(sf::Color::Red);
    
    sf::Text pauseText(font);
    pauseText.setString("PAUSA");
    pauseText.setCharacterSize(60);
    pauseText.setFillColor(sf::Color::Yellow);
    pauseText.setOutlineColor(sf::Color::Black);
    pauseText.setOutlineThickness(3);
    pauseText.setPosition(sf::Vector2f(380, 200));
    
    sf::Text pauseOptionsText(font);
    pauseOptionsText.setString("P o ESC: Continuar | M: Menu Principal");
    pauseOptionsText.setCharacterSize(22);
    pauseOptionsText.setFillColor(sf::Color::White);
    pauseOptionsText.setPosition(sf::Vector2f(220, 320));

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                // Pausa con P o ESC (solo si no está en game over)
                if ((keyPressed->code == sf::Keyboard::Key::P || keyPressed->code == sf::Keyboard::Key::Escape) && !gameOver) {
                    isPaused = !isPaused;
                    if (isPaused) {
                        // Pausar música
                        if (currentMusic == 1) gameMusic1.pause();
                        else gameMusic2.pause();
                        if (isShooting) shootSound.pause();
                    } else {
                        // Reanudar música
                        if (currentMusic == 1) gameMusic1.play();
                        else gameMusic2.play();
                        if (isShooting) shootSound.play();
                    }
                }
                
                // Volver al menú desde pausa
                if (keyPressed->code == sf::Keyboard::Key::M && isPaused && !gameOver) {
                    gameMusic1.stop();
                    gameMusic2.stop();
                    shootSound.stop();
                    isShooting = false;
                    goto EXIT_GAME_LOOP;
                }
                
                if (keyPressed->code == sf::Keyboard::Key::Space && !gameOver && !isPaused) {
                    dino.jump();
                }
                if (keyPressed->code == sf::Keyboard::Key::R && gameOver) {
                    // Pedir nombre del jugador y mostrar opciones
                    GameOverResult result = showGameOver(window, score, difficulty);
                    
                    // Guardar récord si ingresó nombre
                    if (!result.playerName.empty() && result.choice != -1) {
                        addHighScore(gameConfig, result.playerName, score, difficulty);
                    }
                    
                    // Detener músicas y sonidos del juego
                    gameMusic1.stop();
                    gameMusic2.stop();
                    shootSound.stop();
                    isShooting = false;
                    
                    // Manejar la opción elegida
                    if (result.choice == 0) {
                        // REINTENTAR - Reiniciar juego con mismo personaje y dificultad
                        dino = Dino(100, playerGroundY, &characterTexture, numFrames, shootCooldown);
                        enemies.clear();
                        projectiles.clear();
                        explosions.clear();
                        score = 0;
                        lives = 3;
                        gameOver = false;
                        enemySpawnClock.restart();
                        lastEnemyType = -1;
                        consecutiveTrucks = 0;
                        spawnInterval = 2.0f;
                        
                        // Reiniciar música del juego
                        gameMusic1.stop();
                        gameMusic2.stop();
                        gameMusic1.play();
                        currentMusic = 1;
                    }
                    else if (result.choice == 1) {
                        // VER RECORDS - Mostrar tabla de récords y volver al menú
                        showHighScores(window, gameConfig);
                        // Salir del loop del juego para volver al menú
                        goto EXIT_GAME_LOOP;
                    }
                    else {
                        // MENU PRINCIPAL o ESC - Volver al menú
                        goto EXIT_GAME_LOOP;
                    }
                }
                if (keyPressed->code == sf::Keyboard::Key::Escape && gameOver) {
                    // Detener todas las músicas y sonidos del juego
                    gameMusic1.stop();
                    gameMusic2.stop();
                    shootSound.stop();
                    isShooting = false;
                    
                    // Volver al menú principal
                    goto EXIT_GAME_LOOP;
                }
            }
        }
        
        // Alternar entre las músicas del juego cuando una termina (solo si no está en pausa)
        if (!isPaused) {
            if (currentMusic == 1 && gameMusic1.getStatus() != sf::Music::Status::Playing) {
                gameMusic2.play();
                currentMusic = 2;
            } else if (currentMusic == 2 && gameMusic2.getStatus() != sf::Music::Status::Playing) {
                gameMusic1.play();
                currentMusic = 1;
            }
        }

        if (!gameOver && !isPaused) {
            // Controles de movimiento horizontal (A/D o Flechas Izquierda/Derecha)
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
                dino.moveLeft();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
                dino.moveRight();
            }
            
            // Control de agacharse y caída rápida
            bool isDuckingPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);
            dino.duck(isDuckingPressed);
            
            // Control de disparo con tecla R o clic izquierdo del ratón
            bool shootKeyPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R) || 
                                   sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            
            if (shootKeyPressed) {
                // Reproducir sonido de disparo en bucle mientras se dispara
                if (!isShooting) {
                    shootSound.play();
                    isShooting = true;
                }
                
                // Disparar proyectiles
                if (dino.canShoot()) {
                    sf::Vector2f shootPos = dino.getShootPosition();
                    projectiles.push_back(Projectile(shootPos.x, shootPos.y, dino.facingDirection));
                    dino.resetShootClock();
                }
            } else {
                // Detener sonido cuando se suelta la tecla
                if (isShooting) {
                    shootSound.stop();
                    isShooting = false;
                }
            }

            // Actualizar personaje con su posición de suelo ajustada y caída rápida si presiona abajo/S
            dino.update(playerGroundY, isDuckingPressed);
            
            // Aumentar velocidad del juego con el tiempo (cada 20 puntos)
            // La velocidad aumenta gradualmente pero respeta el multiplicador base de dificultad
            float progressMultiplier = 1.0f + (score / 100.0f);
            if (progressMultiplier > 2.0f) progressMultiplier = 2.0f; // Límite máximo de 2x
            gameSpeedMultiplier = difficultySpeedMultiplier * progressMultiplier;

            // Mover fondo con velocidad aumentada
            background1.move(sf::Vector2f(-backgroundSpeed * gameSpeedMultiplier, 0));
            background2.move(sf::Vector2f(-backgroundSpeed * gameSpeedMultiplier, 0));
            
            // Usar el ancho escalado del fondo para el scroll
            float scaledBgWidth = backgroundTexture.getSize().x * background1.getScale().x;
            if (background1.getPosition().x <= -scaledBgWidth) {
                background1.setPosition(sf::Vector2f(background2.getPosition().x + scaledBgWidth, 0));
            }
            if (background2.getPosition().x <= -scaledBgWidth) {
                background2.setPosition(sf::Vector2f(background1.getPosition().x + scaledBgWidth, 0));
            }

            // Spawn enemigos - seleccionar aleatoriamente entre Gengar, Camioneta y Mewtwo
            float currentSpawnInterval = spawnInterval;
            
            // Si el último enemigo fue una camioneta, usar intervalo más largo
            if (lastEnemyType == 1) {
                currentSpawnInterval = std::max(3.5f, spawnInterval * 1.8f); // Mínimo 3.5 segundos después de una camioneta
            }
            
            if (enemySpawnClock.getElapsedTime().asSeconds() > (currentSpawnInterval / gameSpeedMultiplier)) {
                int randomEnemy = rand() % 3; // 0=Gengar, 1=Camioneta, 2=Mewtwo
                
                // Si el último enemigo fue una camioneta, evitar generar otra camioneta
                // (75% de probabilidad de evitarla, 25% de permitirla)
                if (lastEnemyType == 1 && randomEnemy == 1 && (rand() % 100) < 75) {
                    randomEnemy = (rand() % 2 == 0) ? 0 : 2; // Gengar o Mewtwo en su lugar
                }
                
                // Limitar camionetas consecutivas a máximo 1
                if (randomEnemy == 1 && consecutiveTrucks >= 1) {
                    randomEnemy = (rand() % 2 == 0) ? 0 : 2; // Forzar Gengar o Mewtwo
                }
                
                enemies.push_back(Enemy(WINDOW_WIDTH, groundY, enemyTextures[randomEnemy], enemyFrames[randomEnemy], randomEnemy));
                enemySpawnClock.restart();
                
                // Actualizar contador de camionetas consecutivas
                if (randomEnemy == 1) {
                    consecutiveTrucks++;
                } else {
                    consecutiveTrucks = 0;
                }
                
                lastEnemyType = randomEnemy;
                
                if (score > 0 && score % 10 == 0 && spawnInterval > 1.2f) {
                    spawnInterval -= 0.05f; // Reducción más gradual, mínimo 1.2s
                }
            }

            // Actualizar enemigos con velocidad aumentada
            for (auto& enemy : enemies) {
                enemy.update(gameSpeedMultiplier);
            }

            // Actualizar proyectiles
            for (auto& projectile : projectiles) {
                projectile.update();
            }

            // Actualizar explosiones
            for (auto& explosion : explosions) {
                explosion.update();
            }

            // Colisiones proyectiles-enemigos
            for (auto& projectile : projectiles) {
                for (auto& enemy : enemies) {
                    if (projectile.active && enemy.active && 
                        projectile.getBounds().findIntersection(enemy.getBounds()).has_value()) {
                        // La camioneta (tipo 1) es inmune a las balas - las balas la traspasan
                        if (enemy.type == 1) {
                            continue; // La bala no se destruye y la camioneta no recibe daño
                        }
                        projectile.active = false;
                        enemy.active = false;
                        score += static_cast<int>(10 * difficultyScoreMultiplier);
                        explosions.push_back(Explosion(enemy.x, enemy.y - 25));
                    }
                }
            }

            // Colisiones dino-enemigos
            for (auto& enemy : enemies) {
                if (enemy.active && dino.getBounds().findIntersection(enemy.getBounds()).has_value()) {
                    enemy.active = false;
                    lives--;
                    if (lives <= 0) {
                        gameOver = true;
                        // Detener sonido de disparo al morir
                        shootSound.stop();
                        isShooting = false;
                    }
                }
            }

            // Limpiar objetos inactivos
            enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                [](const Enemy& e) { return !e.active; }), enemies.end());
            projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
                [](const Projectile& p) { return !p.active; }), projectiles.end());
            explosions.erase(std::remove_if(explosions.begin(), explosions.end(),
                [](const Explosion& e) { return !e.active; }), explosions.end());

            // Actualizar textos
            scoreText.setString("Score: " + std::to_string(score));
            livesText.setString("Lives: " + std::to_string(lives));
            debugText.setString("X: " + std::to_string(static_cast<int>(dino.x)) + " | Usa A/D o Flechas");
            
            // Actualizar high score si se supera
            if (score > highScore) {
                highScore = score;
                highScoreText.setString("High Score: " + std::to_string(highScore));
            }
        }

        // Dibujar
        window.clear(sf::Color(135, 206, 235));
        
        window.draw(background1);
        window.draw(background2);
        window.draw(ground);
        
        dino.draw(window);
        
        for (auto& enemy : enemies) {
            enemy.draw(window);
        }
        
        for (auto& projectile : projectiles) {
            projectile.draw(window);
        }
        
        for (auto& explosion : explosions) {
            explosion.draw(window);
        }
        
        window.draw(scoreText);
        window.draw(livesText);
        window.draw(highScoreText);
        window.draw(debugText);
        
        if (gameOver) {
            window.draw(gameOverText);
        }
        
        if (isPaused) {
            // Overlay semi-transparente
            sf::RectangleShape pauseOverlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
            pauseOverlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(pauseOverlay);
            
            window.draw(pauseText);
            window.draw(pauseOptionsText);
        }
        
        window.display();
    }

EXIT_GAME_LOOP:
    // Reiniciar la música del menú al volver
    if (window.isOpen()) {
        menuMusic.play();
        currentState = MenuState::MAIN_MENU;
        // Continuar en el bucle principal del menú
        goto CONTINUE_MENU_LOOP;
    } else {
        saveConfig(gameConfig);
        return 0;
    }
}
