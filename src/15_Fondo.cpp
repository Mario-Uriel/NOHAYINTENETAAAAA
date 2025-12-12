#include <SFML/Graphics.hpp>

int main()
{
    // Crear una ventana
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Fondo");

    // Cargar la imagen de fondo desde un archivo
    sf::Texture texture;
    if (!texture.loadFromFile("./assets/images/fondo.jpeg"))
    {
        // Manejar el error si no se puede cargar la imagen
        return -1;
    }

    // Crear un sprite y asignarle la textura
    sf::Sprite sprite(texture);
    
    // Escalar la imagen manteniendo la proporción (sin distorsión)
    sf::Vector2u textureSize = texture.getSize();
    sf::Vector2u windowSize = window.getSize();
    
    // Calcular la escala que mantenga la proporción y muestre la imagen completa
    float scaleX = static_cast<float>(windowSize.x) / textureSize.x;
    float scaleY = static_cast<float>(windowSize.y) / textureSize.y;
    float scale = (scaleX < scaleY) ? scaleX : scaleY; // Usar la escala menor para que quepa completa
    
    sprite.setScale(sf::Vector2f(scale, scale));
    
    // Centrar la imagen en la ventana
    float scaledWidth = textureSize.x * scale;
    float scaledHeight = textureSize.y * scale;
    sprite.setPosition(sf::Vector2f((windowSize.x - scaledWidth) / 2, (windowSize.y - scaledHeight) / 2));

    // Bucle principal
    while (window.isOpen())
    {
        // Procesar eventos
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                // Cerrar la ventana si se recibe el evento de cerrar
                window.close();
            }
        }

        // Limpiar la ventana
        window.clear();

        // Dibujar el sprite en la ventana
        window.draw(sprite);

        // Mostrar la ventana
        window.display();
    }

    return 0;
}
