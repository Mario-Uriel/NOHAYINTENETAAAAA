## 📝 Descripción del Proyecto

**PockyMan: Asalto a la Pokeplaza** es un juego de acción tipo endless runner donde controlas a Pikachu o Ballesta en su misión de defender la Pokeplaza de enemigos invasores. Esquiva obstáculos, dispara proyectiles y acumula la mayor puntuación posible mientras la dificultad aumenta progresivamente.

### 🎯 Objetivo del Juego

Sobrevive el mayor tiempo posible, elimina enemigos y alcanza el récord más alto. Cada enemigo derrotado suma puntos, pero ten c

uidado: solo tienes 3 vidas. La velocidad del juego aumenta conforme avanzas, poniendo a prueba tus reflejos y habilidades.

### 🎮 Controles

**Movimiento:**
- **A / Flecha Izquierda**: Mover a la izquierda
- **D / Flecha Derecha**: Mover a la derecha
- **Space**: Saltar
- **S / Flecha Abajo**:Acelerar caída (en aire)

**Combate:**
- **R / Clic Izquierdo**: Disparar proyectiles

**Sistema:**
- **P / ESC**: Pausar/Reanudar juego
- **M**: Volver al menú principal (durante pausa)
- **ESC**: Salir del juego o menú

### ⚙️ Mecánicas

**Sistema de Dificultad:**
- **Fácil**: Velocidad reducida (0.7x), multiplicador de puntos 1.5x, cooldown de disparo rápido (0.15s)
- **Normal**: Velocidad estándar (1.0x), multiplicador de puntos 2.0x, cooldown normal (0.2s)
- **Difícil**: Velocidad aumentada (1.3x), multiplicador de puntos 2.5x, cooldown lento (0.3s)

**Progresión Dinámica:**
- La velocidad del juego aumenta gradualmente con tu puntuación (hasta 2x)
- Los enemigos aparecen con intervalos dinámicos según la dificultad

**Sistema de Enemigos:**
- **Gengar**: Enemigo volador estándar
- **Camioneta**: Obstáculo terrestre grande
- **Mewtwo**: Enemigo desafiante con patrón especial

**Mecánica de Disparo:**
- Cooldown entre disparos según dificultad
- Sonido continuo mientras disparas
- Los proyectiles destruyen enemigos al impactar

**Sistema de Vidas:**
- 3 vidas por partida
- Pierdes una vida al colisionar con enemigos
- Game Over al perder todas las vidas

### 🏆 Características

- **Sistema de Menús Completo**: Menú principal, selección de dificultad, selección de personaje, configuración y tabla de récords
- **Tabla de Récords Top 10**: Guarda los mejores puntajes con nombre del jugador y dificultad
- **Dos Personajes Jugables**: Pikachu y Ballesta con animaciones únicas
- **Sistema de Pausa**: Pausa el juego en cualquier momento sin perder progreso
- **Música Dinámica**: Música de menú y dos pistas de juego que alternan automáticamente
- **Efectos de Sonido**: Sonidos de disparo sincronizados con las acciones
- **Control de Volumen**: Ajusta música y efectos de sonido de forma independiente
- **Fondos Parallax**: Scroll infinito de fondos animados
- **Configuración Persistente**: Guarda volúmenes y récords automáticamente
- **Sistema de Dificultad**: 3 niveles con ajustes de velocidad, puntuación y cooldown

### 👥 Equipo

- **Líder**: Mario Uriel Aguayo Sandoval (@Mario-Uriel)
- **Integrante 2**: Noé Sebastián Palomera Trujillo (@S3b4s-117)

### 🛠️ Tecnologías

- **Framework**: SFML 3.x (Simple and Fast Multimedia Library)
- **Lenguaje**: C++
- **Librerías adicionales**: 
  - Box2D (física)
  - SFML Graphics (renderizado)
  - SFML Audio (sonido y música)
  - SFML Window (ventana y eventos)
- **Herramientas**: 
  - Make (compilación)
  - g++ (compilador)
  - Git (control de versiones)

### 🎨 Assets

**Sprites:**
- PIKACHU (2) (1).png - Sprite sheet de Pikachu (4 frames)
- Ballesta .png - Sprite sheet de Ballesta (4 frames)
- Gengar.png - Sprite de enemigo Gengar
- Camioneta FINAL.png - Sprite de enemigo Camioneta
- Mewtwo (1).png - Sprite de enemigo Mewtwo

**Fondos:**
- Menu principal.png - Fondo del menú principal
- Fondo principal.png - Fondo de selección de personaje
- fondo.png - Fondo del juego

**Música:**
- Selecciona-tu-personaje.ogg - Música del menú
- Jugar1.ogg - Primera pista de juego
- Jugar2.ogg - Segunda pista de juego

**Efectos de Sonido:**
- AK-47.ogg - Sonido de disparo Pikachu
- Ballesta sonido.ogg - Sonido de disparo Ballesta

**Fuentes:**
- Minecraft.ttf - Fuente principal del juego

### 📜 Créditos

- Proyecto desarrollado como parte del curso de Programación de Videojuegos
- Sprites de Pokémon inspirados en la franquicia original
- Música y efectos de sonido utilizados con fines educativos
- Agradecimientos especiales a GitHub Copilot por asistencia en el desarrollo

---