#include <SFML/Graphics.hpp>

int main()
{
    // Crear una ventana
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Mewtwo Sprite Animado");

    // Cargar la imagen Mewtwo desde un archivo
    sf::Texture texture;
    if (!texture.loadFromFile("assets/images/Mewtwo (1).png"))
    {
        // Manejo de error si no se puede cargar la imagen
        return -1;
    }

    // Crear un sprite y asignarle la textura
    sf::Sprite sprite(texture);
    
    // Obtener el tamaño de la textura
    sf::Vector2u textureSize = texture.getSize();
    
    // Calcular escala para que quepa en pantalla (máximo 300px de altura)
    float scale = 300.0f / textureSize.y;
    if (scale > 1.0f) scale = 1.0f; // Limitar escala máxima
    sprite.setScale(sf::Vector2f(scale, scale)); // Sin voltear para que mire a la izquierda
    
    // Centrar en la ventana
    float scaledWidth = textureSize.x * scale;
    float scaledHeight = textureSize.y * scale;
    sprite.setPosition(sf::Vector2f((800 - scaledWidth) / 2, (600 - scaledHeight) / 2));

    sf::Clock clock;
    float frameTime = 0.12f; // Tiempo entre cada frame (animación más lenta)
    int currentFrame = 0;
    int numFrames = 4; // Número total de frames en la animación
    float posX = 800; // Empezar desde la derecha
    float speed = -100.0f; // Velocidad negativa para moverse a la izquierda

    while (window.isOpen())
    {
        // Procesar eventos
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        float deltaTime = clock.restart().asSeconds();

        // Mover el sprite hacia la izquierda
        posX += speed * deltaTime;
        
        // Si sale de la pantalla por la izquierda, volver a aparecer por la derecha
        if (posX < -scaledWidth)
        {
            posX = 800;
        }
        
        sprite.setPosition(sf::Vector2f(posX, (600 - scaledHeight) / 2));

        // Actualizar el frame de la animación
        static float animTimer = 0.0f;
        animTimer += deltaTime;
        if (animTimer >= frameTime)
        {
            currentFrame = (currentFrame + 1) % numFrames;
            // Animar usando frames horizontales de la imagen
            int frameWidth = textureSize.x / numFrames;
            sprite.setTextureRect(sf::IntRect(sf::Vector2i(currentFrame * frameWidth, 0), sf::Vector2i(frameWidth, textureSize.y)));
            animTimer = 0.0f;
        }

        window.clear(sf::Color(200, 200, 200));
        window.draw(sprite);
        window.display();
    }

    return 0;
}
