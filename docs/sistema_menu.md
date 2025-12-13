# Sistema de Menú para PockyMan: Asalto a la Poképlaza

## Descripción General

El archivo `28_MenuSystem.cpp` contiene la implementación completa del sistema de menú para el juego, incluyendo todas las funcionalidades solicitadas.

## Estructura del Menú

### 1. Menú Principal
**Título:** PockyMan: Asalto a la Poképlaza

**Opciones:**
- **1. Empezar Juego** - Inicia el flujo: Selección de Personaje → Selección de Dificultad → Juego
- **2. Configuraciones** - Ajustes de volumen (música y efectos de sonido)
- **3. Registro de Récord** - Muestra el top 10 de puntajes con nombres de jugadores

**Controles:**
- Flechas arriba/abajo o teclas numéricas (1, 2, 3) para navegar
- ENTER para confirmar selección

### 2. Selección de Dificultad

**Niveles disponibles:**

#### Fácil
- **Velocidad:** 0.7x (todo 30% más lento)
- **Multiplicador de Score:** 0.5x
- **Cooldown de disparo:** 0.25s (normal)

#### Normal
- **Velocidad:** 1.0x (velocidad base)
- **Multiplicador de Score:** 1.0x
- **Cooldown de disparo:** 0.25s

#### Difícil
- **Velocidad:** 1.4x (todo 40% más rápido)
- **Multiplicador de Score:** 1.5x
- **Cooldown de disparo:** 0.5s (el doble, mayor dificultad)

**Controles:**
- Flechas arriba/abajo o teclas numéricas (1, 2, 3) para seleccionar
- ENTER para confirmar

### 3. Configuraciones

Permite ajustar dos parámetros independientes:

- **Volumen de Música:** 0-100% (música de fondo)
- **Volumen de Efectos:** 0-100% (sonidos de disparos, explosiones, etc.)

**Controles:**
- Flechas arriba/abajo para cambiar entre configuraciones
- Flechas izquierda/derecha para ajustar valores (incrementos de 5%)
- ESC para guardar y salir

### 4. Registro de Récords

Muestra el **Top 10** de puntajes históricos con:
- Posición en el ranking
- Nombre del jugador
- Puntuación obtenida
- Dificultad jugada

**Colores especiales:**
- 🥇 1er lugar: Dorado
- 🥈 2do lugar: Plateado
- 🥉 3er lugar: Bronce
- Resto: Blanco

**Controles:**
- ESC para volver al menú principal

### 5. Pantalla de Game Over

Al terminar una partida, se muestra:
- Puntuación final
- Campo de input para ingresar nombre (máximo 15 caracteres)
- Acepta letras, números y espacios

**Controles:**
- Escribir nombre del jugador
- ENTER para guardar récord
- ESC para saltar y guardar como "Anonimo"

## Persistencia de Datos

### Archivo: `game_config.dat`

Guarda:
1. Volumen de música (float)
2. Volumen de efectos (float)
3. Número de récords guardados
4. Lista de récords (nombre, puntuación, dificultad)

Los datos se cargan automáticamente al iniciar el juego y se guardan:
- Al salir de configuraciones
- Al guardar un nuevo récord después de Game Over

## Funciones Principales

### Configuración
```cpp
struct GameConfig {
    float musicVolume;           // Volumen de música (0-100)
    float sfxVolume;            // Volumen de efectos (0-100)
    vector<HighScoreEntry> highScores; // Lista de récords
};

void saveConfig(const GameConfig& config);
void loadConfig(GameConfig& config);
```

### Sistema de Récords
```cpp
struct HighScoreEntry {
    string playerName;  // Nombre del jugador
    int score;         // Puntuación
    string difficulty; // "Facil", "Normal", "Dificil"
};

void addHighScore(GameConfig& config, const string& playerName, 
                  int score, GameDifficulty difficulty);
```

### Modificadores de Dificultad
```cpp
struct DifficultyModifiers {
    float speedMultiplier;    // Multiplicador de velocidad
    float scoreMultiplier;    // Multiplicador de puntuación
    float shootCooldown;      // Tiempo entre disparos
};

DifficultyModifiers getDifficultyModifiers(GameDifficulty difficulty);
```

### Pantallas del Menú
```cpp
MenuState showMainMenu(sf::RenderWindow& window, sf::Music& menuMusic, 
                       GameConfig& config);

GameDifficulty showDifficultySelect(sf::RenderWindow& window);

void showSettings(sf::RenderWindow& window, GameConfig& config);

void showHighScores(sf::RenderWindow& window, const GameConfig& config);

string showGameOver(sf::RenderWindow& window, int finalScore, 
                    GameDifficulty difficulty);
```

## Integración con 18_DinoRevengeSelect.cpp

Para integrar este sistema en el juego principal:

### Paso 1: Copiar Estructuras y Funciones
Copiar del archivo `28_MenuSystem.cpp`:
- Enums: `GameDifficulty`, `MenuState`
- Structs: `GameConfig`, `HighScoreEntry`, `DifficultyModifiers`
- Funciones: `saveConfig`, `loadConfig`, `addHighScore`, `getDifficultyModifiers`
- Funciones de menú: `showMainMenu`, `showDifficultySelect`, `showSettings`, `showHighScores`, `showGameOver`

### Paso 2: Modificar la Función Main
```cpp
int main() {
    // Configuración inicial
    GameConfig gameConfig;
    loadConfig(gameConfig);
    
    // Música del menú
    sf::Music menuMusic;
    menuMusic.openFromFile("assets/music/Selecciona-tu-personaje.ogg");
    menuMusic.setLooping(true);
    menuMusic.setVolume(gameConfig.musicVolume);
    
    MenuState currentState = MenuState::MAIN_MENU;
    GameDifficulty selectedDifficulty = GameDifficulty::NORMAL;
    
    // Loop principal del menú
    while (window.isOpen()) {
        switch (currentState) {
            case MenuState::MAIN_MENU:
                currentState = showMainMenu(window, menuMusic, gameConfig);
                break;
                
            case MenuState::DIFFICULTY_SELECT:
                selectedDifficulty = showDifficultySelect(window);
                currentState = MenuState::CHARACTER_SELECT;
                break;
                
            case MenuState::CHARACTER_SELECT:
                // Código existente de selección de personaje
                currentState = MenuState::PLAYING;
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
                // Loop del juego existente
                // Al terminar, ir a GAME_OVER
                break;
                
            case MenuState::GAME_OVER:
                string playerName = showGameOver(window, finalScore, selectedDifficulty);
                addHighScore(gameConfig, playerName, finalScore, selectedDifficulty);
                currentState = MenuState::MAIN_MENU;
                break;
        }
    }
}
```

### Paso 3: Aplicar Modificadores de Dificultad
```cpp
// Obtener modificadores al iniciar el juego
DifficultyModifiers mods = getDifficultyModifiers(selectedDifficulty);

// Aplicar en el juego
gameSpeedMultiplier = 1.0f * mods.speedMultiplier;
dino = Dino(100, playerGroundY, &characterTexture, numFrames, mods.shootCooldown);

// Al sumar puntos
score += static_cast<int>(10 * mods.scoreMultiplier);
```

### Paso 4: Actualizar Volúmenes
```cpp
// Aplicar volúmenes desde la configuración
gameMusic1.setVolume(gameConfig.musicVolume);
gameMusic2.setVolume(gameConfig.musicVolume);
shootSound.setVolume(gameConfig.sfxVolume);
```

## Recursos Necesarios

### Fuentes
- `assets/fonts/Minecraft.ttf`

### Imágenes
- `assets/images/Menu principal.png` (fondo del menú)

### Música
- `assets/music/Selecciona-tu-personaje.ogg` (música del menú)

## Notas Importantes

1. **Persistencia:** Los datos se guardan automáticamente, no se pierden al cerrar el juego
2. **Top 10:** Solo se mantienen los 10 mejores puntajes
3. **Validación:** Los nombres de jugador solo aceptan letras, números y espacios (máx. 15 caracteres)
4. **Ordenamiento:** Los récords se ordenan automáticamente por puntuación descendente
5. **Volúmenes:** Se ajustan en incrementos de 5% (rango 0-100%)

## Compilación

```bash
# Compilar solo el sistema de menú (demostración)
make run28_MenuSystem

# Compilar el juego completo con el menú integrado
make run18_DinoRevengeSelect
```

## Flujo de Navegación

```
Menú Principal
    ├─→ Empezar Juego
    │       ├─→ Selección de Dificultad
    │       │       └─→ Selección de Personaje
    │       │               └─→ Juego
    │       │                       └─→ Game Over (input nombre)
    │       │                               └─→ Guardar récord
    │       │                                       └─→ Menú Principal
    │
    ├─→ Configuraciones
    │       └─→ Ajustar volúmenes
    │               └─→ Menú Principal
    │
    └─→ Registro de Récord
            └─→ Ver Top 10
                    └─→ Menú Principal
```

## Características Implementadas

✅ Menú principal con título del juego  
✅ Flujo: Selección de Personaje → Selección de Dificultad  
✅ 3 niveles de dificultad con modificadores proporcionales  
✅ Configuración de volumen independiente (música y SFX)  
✅ Sistema de récords con nombres de jugadores  
✅ Top 10 histórico de puntuaciones  
✅ Persistencia de datos en archivo  
✅ Pantalla de Game Over con input de nombre  
✅ Colores especiales para los 3 primeros lugares  
✅ Navegación intuitiva con teclado  

---

**Archivo creado:** 13 de diciembre de 2025  
**Versión:** 1.0  
**Proyecto:** PockyMan: Asalto a la Poképlaza
