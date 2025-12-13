#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <sstream>

const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 600;

// Enumeraciones para el sistema de menú
enum class GameDifficulty {
    EASY,      // Todo más lento, menor multiplicador de score
    NORMAL,    // Velocidad base, multiplicador estándar
    HARD       // Todo más rápido + cooldown en disparos, mayor multiplicador
};

enum class MenuState {
    MAIN_MENU,           // Menú principal
    CHARACTER_SELECT,    // Selección de personaje
    DIFFICULTY_SELECT,   // Selección de dificultad
    SETTINGS,           // Configuraciones (volumen)
    HIGH_SCORES,        // Registro de récords
    PLAYING,            // Estado de juego
    GAME_OVER          // Pantalla de game over con input de nombre
};

// Estructura para guardar récords con nombre de jugador
struct HighScoreEntry {
    std::string playerName;
    int score;
    std::string difficulty; // "Facil", "Normal", "Dificil"
    
    HighScoreEntry() : playerName(""), score(0), difficulty("Normal") {}
    HighScoreEntry(const std::string& name, int s, const std::string& diff) 
        : playerName(name), score(s), difficulty(diff) {}
};

// Estructura de configuración global
struct GameConfig {
    float musicVolume = 50.0f;   // Volumen de música de fondo
    float sfxVolume = 50.0f;     // Volumen de efectos de sonido
    std::vector<HighScoreEntry> highScores; // Lista de todos los récords
};

// Funciones para guardar y cargar configuración
void saveConfig(const GameConfig& config) {
    std::ofstream file("game_config.dat");
    if (file.is_open()) {
        file << config.musicVolume << "\n";
        file << config.sfxVolume << "\n";
        file << config.highScores.size() << "\n";
        
        // Guardar todos los récords
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
        file >> config.musicVolume;
        file >> config.sfxVolume;
        
        size_t numScores;
        file >> numScores;
        file.ignore(); // Ignorar el salto de línea
        
        config.highScores.clear();
        for (size_t i = 0; i < numScores; ++i) {
            HighScoreEntry entry;
            std::getline(file, entry.playerName);
            file >> entry.score;
            file.ignore();
            std::getline(file, entry.difficulty);
            config.highScores.push_back(entry);
        }
        file.close();
    }
}

// Función para agregar un nuevo récord y ordenar la lista
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
    
    // Ordenar por puntuación descendente
    std::sort(config.highScores.begin(), config.highScores.end(),
              [](const HighScoreEntry& a, const HighScoreEntry& b) {
                  return a.score > b.score;
              });
    
    // Mantener solo los top 10
    if (config.highScores.size() > 10) {
        config.highScores.resize(10);
    }
    
    saveConfig(config);
}

// Función para obtener multiplicadores según dificultad
struct DifficultyModifiers {
    float speedMultiplier;
    float scoreMultiplier;
    float shootCooldown;
};

DifficultyModifiers getDifficultyModifiers(GameDifficulty difficulty) {
    DifficultyModifiers mods;
    
    switch (difficulty) {
        case GameDifficulty::EASY:
            mods.speedMultiplier = 0.7f;   // Todo 30% más lento
            mods.scoreMultiplier = 0.5f;   // 50% del score normal
            mods.shootCooldown = 0.25f;    // Disparo normal
            break;
            
        case GameDifficulty::NORMAL:
            mods.speedMultiplier = 1.0f;   // Velocidad base
            mods.scoreMultiplier = 1.0f;   // Score estándar
            mods.shootCooldown = 0.25f;    // Disparo normal
            break;
            
        case GameDifficulty::HARD:
            mods.speedMultiplier = 1.4f;   // Todo 40% más rápido
            mods.scoreMultiplier = 1.5f;   // 150% del score normal
            mods.shootCooldown = 0.5f;     // Cooldown doble entre disparos
            break;
    }
    
    return mods;
}

// MENÚ PRINCIPAL
MenuState showMainMenu(sf::RenderWindow& window, sf::Music& menuMusic, GameConfig& config) {
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return MenuState::MAIN_MENU;
    }
    
    // Cargar fondo del menú
    sf::Texture bgTexture;
    bool hasBackground = bgTexture.loadFromFile("assets/images/Menu principal.png");
    sf::Sprite bgSprite(bgTexture);
    if (hasBackground) {
        float scaleX = static_cast<float>(WINDOW_WIDTH) / bgTexture.getSize().x;
        float scaleY = static_cast<float>(WINDOW_HEIGHT) / bgTexture.getSize().y;
        bgSprite.setScale(sf::Vector2f(scaleX, scaleY));
    }
    
    // Título del juego
    sf::Text titleText(font);
    titleText.setString("PockyMan: Asalto a la Pokeplaza");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setOutlineColor(sf::Color::Black);
    titleText.setOutlineThickness(3);
    
    // Centrar título
    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setPosition(sf::Vector2f((WINDOW_WIDTH - titleBounds.size.x) / 2, 80));
    
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
        text.setFillColor(i == 0 ? sf::Color::Cyan : sf::Color::White);
        text.setPosition(sf::Vector2f(300, 250 + i * 70));
        menuTexts.push_back(text);
    }
    
    sf::Text instructionText(font);
    instructionText.setString("Usa FLECHAS o numeros para seleccionar, ENTER para confirmar");
    instructionText.setCharacterSize(18);
    instructionText.setFillColor(sf::Color(200, 200, 200));
    instructionText.setPosition(sf::Vector2f(150, 520));
    
    // Iniciar música del menú
    if (menuMusic.getStatus() != sf::Music::Status::Playing) {
        menuMusic.play();
    }
    menuMusic.setVolume(config.musicVolume);
    
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return MenuState::MAIN_MENU;
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                // Navegación con flechas
                if (keyPressed->code == sf::Keyboard::Key::Up) {
                    menuTexts[selectedOption].setFillColor(sf::Color::White);
                    selectedOption = (selectedOption - 1 + menuOptions.size()) % menuOptions.size();
                    menuTexts[selectedOption].setFillColor(sf::Color::Cyan);
                }
                else if (keyPressed->code == sf::Keyboard::Key::Down) {
                    menuTexts[selectedOption].setFillColor(sf::Color::White);
                    selectedOption = (selectedOption + 1) % menuOptions.size();
                    menuTexts[selectedOption].setFillColor(sf::Color::Cyan);
                }
                // Selección con números
                else if (keyPressed->code == sf::Keyboard::Key::Num1) {
                    selectedOption = 0;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Num2) {
                    selectedOption = 1;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Num3) {
                    selectedOption = 2;
                }
                // Confirmar selección
                else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                    switch (selectedOption) {
                        case 0:
                            return MenuState::CHARACTER_SELECT;
                        case 1:
                            return MenuState::SETTINGS;
                        case 2:
                            return MenuState::HIGH_SCORES;
                    }
                }
            }
        }
        
        window.clear(sf::Color(20, 20, 40));
        window.draw(bgSprite);
        
        // Overlay semi-transparente para mejorar legibilidad
        sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 100));
        window.draw(overlay);
        
        window.draw(titleText);
        for (auto& text : menuTexts) {
            window.draw(text);
        }
        window.draw(instructionText);
        window.display();
    }
    
    return MenuState::MAIN_MENU;
}

// SELECCIÓN DE DIFICULTAD
GameDifficulty showDifficultySelect(sf::RenderWindow& window) {
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return GameDifficulty::NORMAL;
    }
    
    sf::Text titleText(font);
    titleText.setString("SELECCIONA LA DIFICULTAD");
    titleText.setCharacterSize(40);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setPosition(sf::Vector2f(220, 80));
    
    std::vector<std::string> difficultyOptions = {
        "1. FACIL",
        "2. NORMAL",
        "3. DIFICIL"
    };
    
    std::vector<std::string> descriptions = {
        "Todo mas lento | Score x0.5",
        "Velocidad base | Score x1.0",
        "Todo mas rapido + Cooldown | Score x1.5"
    };
    
    std::vector<sf::Text> optionTexts;
    std::vector<sf::Text> descTexts;
    int selectedDifficulty = 1; // Normal por defecto
    
    for (size_t i = 0; i < difficultyOptions.size(); ++i) {
        sf::Text optText(font);
        optText.setString(difficultyOptions[i]);
        optText.setCharacterSize(35);
        optText.setFillColor(i == 1 ? sf::Color::Cyan : sf::Color::White);
        optText.setPosition(sf::Vector2f(350, 200 + i * 100));
        optionTexts.push_back(optText);
        
        sf::Text desc(font);
        desc.setString(descriptions[i]);
        desc.setCharacterSize(20);
        desc.setFillColor(sf::Color(180, 180, 180));
        desc.setPosition(sf::Vector2f(250, 240 + i * 100));
        descTexts.push_back(desc);
    }
    
    sf::Text instructionText(font);
    instructionText.setString("FLECHAS o numeros para elegir | ENTER para confirmar");
    instructionText.setCharacterSize(18);
    instructionText.setFillColor(sf::Color(200, 200, 200));
    instructionText.setPosition(sf::Vector2f(200, 530));
    
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return GameDifficulty::NORMAL;
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Up) {
                    optionTexts[selectedDifficulty].setFillColor(sf::Color::White);
                    selectedDifficulty = (selectedDifficulty - 1 + 3) % 3;
                    optionTexts[selectedDifficulty].setFillColor(sf::Color::Cyan);
                }
                else if (keyPressed->code == sf::Keyboard::Key::Down) {
                    optionTexts[selectedDifficulty].setFillColor(sf::Color::White);
                    selectedDifficulty = (selectedDifficulty + 1) % 3;
                    optionTexts[selectedDifficulty].setFillColor(sf::Color::Cyan);
                }
                else if (keyPressed->code == sf::Keyboard::Key::Num1) {
                    selectedDifficulty = 0;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Num2) {
                    selectedDifficulty = 1;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Num3) {
                    selectedDifficulty = 2;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Enter) {
                    return static_cast<GameDifficulty>(selectedDifficulty);
                }
            }
        }
        
        window.clear(sf::Color(20, 20, 40));
        window.draw(titleText);
        for (size_t i = 0; i < optionTexts.size(); ++i) {
            window.draw(optionTexts[i]);
            window.draw(descTexts[i]);
        }
        window.draw(instructionText);
        window.display();
    }
    
    return GameDifficulty::NORMAL;
}

// CONFIGURACIONES (Ajuste de volumen)
void showSettings(sf::RenderWindow& window, GameConfig& config) {
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return;
    }
    
    sf::Text titleText(font);
    titleText.setString("CONFIGURACIONES");
    titleText.setCharacterSize(40);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setPosition(sf::Vector2f(320, 100));
    
    sf::Text musicLabel(font);
    musicLabel.setString("Volumen de Musica:");
    musicLabel.setCharacterSize(28);
    musicLabel.setFillColor(sf::Color::White);
    musicLabel.setPosition(sf::Vector2f(200, 250));
    
    sf::Text sfxLabel(font);
    sfxLabel.setString("Volumen de Efectos:");
    sfxLabel.setCharacterSize(28);
    sfxLabel.setFillColor(sf::Color::White);
    sfxLabel.setPosition(sf::Vector2f(200, 350));
    
    sf::Text instructionText(font);
    instructionText.setString("FLECHAS IZQUIERDA/DERECHA para ajustar | ESC para guardar y salir");
    instructionText.setCharacterSize(16);
    instructionText.setFillColor(sf::Color(200, 200, 200));
    instructionText.setPosition(sf::Vector2f(150, 500));
    
    int selectedSetting = 0; // 0 = music, 1 = sfx
    
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
                }
                else if (keyPressed->code == sf::Keyboard::Key::Up) {
                    selectedSetting = 0;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Down) {
                    selectedSetting = 1;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Left) {
                    if (selectedSetting == 0) {
                        config.musicVolume = std::max(0.0f, config.musicVolume - 5.0f);
                    } else {
                        config.sfxVolume = std::max(0.0f, config.sfxVolume - 5.0f);
                    }
                }
                else if (keyPressed->code == sf::Keyboard::Key::Right) {
                    if (selectedSetting == 0) {
                        config.musicVolume = std::min(100.0f, config.musicVolume + 5.0f);
                    } else {
                        config.sfxVolume = std::min(100.0f, config.sfxVolume + 5.0f);
                    }
                }
            }
        }
        
        // Actualizar textos de volumen con indicador de selección
        std::string musicStr = "Volumen de Musica: " + std::to_string(static_cast<int>(config.musicVolume)) + "%";
        if (selectedSetting == 0) musicStr = "> " + musicStr + " <";
        musicLabel.setString(musicStr);
        musicLabel.setFillColor(selectedSetting == 0 ? sf::Color::Cyan : sf::Color::White);
        
        std::string sfxStr = "Volumen de Efectos: " + std::to_string(static_cast<int>(config.sfxVolume)) + "%";
        if (selectedSetting == 1) sfxStr = "> " + sfxStr + " <";
        sfxLabel.setString(sfxStr);
        sfxLabel.setFillColor(selectedSetting == 1 ? sf::Color::Cyan : sf::Color::White);
        
        window.clear(sf::Color(20, 20, 40));
        window.draw(titleText);
        window.draw(musicLabel);
        window.draw(sfxLabel);
        window.draw(instructionText);
        window.display();
    }
}

// REGISTRO DE RÉCORDS (Muestra todos los récords históricos)
void showHighScores(sf::RenderWindow& window, const GameConfig& config) {
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return;
    }
    
    sf::Text titleText(font);
    titleText.setString("REGISTRO DE RECORD - TOP 10");
    titleText.setCharacterSize(40);
    titleText.setFillColor(sf::Color::Yellow);
    titleText.setPosition(sf::Vector2f(200, 50));
    
    std::vector<sf::Text> scoreTexts;
    
    if (config.highScores.empty()) {
        sf::Text emptyText(font);
        emptyText.setString("No hay records registrados aun");
        emptyText.setCharacterSize(25);
        emptyText.setFillColor(sf::Color::White);
        emptyText.setPosition(sf::Vector2f(280, 280));
        scoreTexts.push_back(emptyText);
    } else {
        for (size_t i = 0; i < config.highScores.size() && i < 10; ++i) {
            const auto& entry = config.highScores[i];
            
            std::string scoreStr = std::to_string(i + 1) + ". " + 
                                  entry.playerName + " - " + 
                                  std::to_string(entry.score) + " pts (" + 
                                  entry.difficulty + ")";
            
            sf::Text text(font);
            text.setString(scoreStr);
            text.setCharacterSize(22);
            
            // Color según posición
            if (i == 0) {
                text.setFillColor(sf::Color(255, 215, 0)); // Oro
            } else if (i == 1) {
                text.setFillColor(sf::Color(192, 192, 192)); // Plata
            } else if (i == 2) {
                text.setFillColor(sf::Color(205, 127, 50)); // Bronce
            } else {
                text.setFillColor(sf::Color::White);
            }
            
            text.setPosition(sf::Vector2f(120, 140 + i * 35));
            scoreTexts.push_back(text);
        }
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
        window.draw(titleText);
        for (auto& text : scoreTexts) {
            window.draw(text);
        }
        window.draw(backText);
        window.display();
    }
}

// PANTALLA DE GAME OVER CON INPUT DE NOMBRE
std::string showGameOver(sf::RenderWindow& window, int finalScore, GameDifficulty difficulty) {
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Minecraft.ttf")) {
        return "Player";
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
    scoreText.setPosition(sf::Vector2f(280, 220));
    
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
    instructionText.setString("Escribe tu nombre y presiona ENTER (maximo 15 caracteres)");
    instructionText.setCharacterSize(18);
    instructionText.setFillColor(sf::Color(200, 200, 200));
    instructionText.setPosition(sf::Vector2f(180, 450));
    
    sf::Text skipText(font);
    skipText.setString("Presiona ESC para saltar");
    skipText.setCharacterSize(16);
    skipText.setFillColor(sf::Color(150, 150, 150));
    skipText.setPosition(sf::Vector2f(370, 500));
    
    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                return "Player";
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Enter && !playerName.empty()) {
                    return playerName;
                }
                else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    return "Anonimo";
                }
                else if (keyPressed->code == sf::Keyboard::Key::Backspace && !playerName.empty()) {
                    playerName.pop_back();
                }
            }
            
            if (const auto* textEntered = event->getIf<sf::Event::TextEntered>()) {
                // Solo aceptar caracteres alfanuméricos y espacios
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
        window.draw(titleText);
        window.draw(scoreText);
        window.draw(promptText);
        window.draw(nameInputText);
        window.draw(instructionText);
        window.draw(skipText);
        window.display();
    }
    
    return "Player";
}

// Función principal de demostración del sistema de menú
int main() {
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), 
                           "PockyMan: Asalto a la Pokeplaza - Sistema de Menu");
    window.setFramerateLimit(60);
    
    // Configuración global
    GameConfig gameConfig;
    loadConfig(gameConfig);
    
    // Música del menú
    sf::Music menuMusic;
    if (!menuMusic.openFromFile("assets/music/Selecciona-tu-personaje.ogg")) {
        std::cerr << "Error cargando musica del menu" << std::endl;
    }
    menuMusic.setLooping(true);
    menuMusic.setVolume(gameConfig.musicVolume);
    
    MenuState currentState = MenuState::MAIN_MENU;
    GameDifficulty selectedDifficulty = GameDifficulty::NORMAL;
    
    while (window.isOpen()) {
        switch (currentState) {
            case MenuState::MAIN_MENU:
                currentState = showMainMenu(window, menuMusic, gameConfig);
                break;
                
            case MenuState::CHARACTER_SELECT:
                // Aquí iría la selección de personaje (ya existe en 18_DinoRevengeSelect.cpp)
                // Por ahora saltamos directo a selección de dificultad
                currentState = MenuState::DIFFICULTY_SELECT;
                break;
                
            case MenuState::DIFFICULTY_SELECT:
                selectedDifficulty = showDifficultySelect(window);
                // Después de seleccionar dificultad, iniciar el juego
                // Por ahora, volver al menú para demostración
                currentState = MenuState::MAIN_MENU;
                
                // En la integración real, aquí se iniciaría el juego:
                // currentState = MenuState::PLAYING;
                break;
                
            case MenuState::SETTINGS:
                showSettings(window, gameConfig);
                currentState = MenuState::MAIN_MENU;
                break;
                
            case MenuState::HIGH_SCORES:
                showHighScores(window, gameConfig);
                currentState = MenuState::MAIN_MENU;
                break;
                
            case MenuState::PLAYING:
                // Aquí va el loop del juego (código de 18_DinoRevengeSelect.cpp)
                // Cuando termina el juego, ir a GAME_OVER
                currentState = MenuState::GAME_OVER;
                break;
                
            case MenuState::GAME_OVER: {
                // Simular un puntaje final para demostración
                int finalScore = 1500;
                std::string playerName = showGameOver(window, finalScore, selectedDifficulty);
                
                // Guardar el récord
                addHighScore(gameConfig, playerName, finalScore, selectedDifficulty);
                
                // Volver al menú principal
                currentState = MenuState::MAIN_MENU;
                break;
            }
        }
    }
    
    return 0;
}
