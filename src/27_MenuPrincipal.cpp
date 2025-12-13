#include <SFML/Graphics.hpp>

int main()
{
    // Crear una ventana
    sf::RenderWindow window(sf::VideoMode({1000, 600}), "SFML - Menu Principal");

    // Cargar la imagen del menú principal desde un archivo
    sf::Texture texture;
    if (!texture.loadFromFile("./assets/images/Menu principal.png"))
    {
        // Manejar el error si no se puede cargar la imagen
        return -1;
    }

    // Crear un sprite y asignarle la textura
    sf::Sprite sprite(texture);
    
    // Escalar el sprite para que cubra toda la ventana sin distorsión
    sf::Vector2u textureSize = texture.getSize();
    float scaleX = 1000.0f / textureSize.x;
    float scaleY = 600.0f / textureSize.y;
    float scale = std::max(scaleX, scaleY); // Usar el mayor para cubrir toda la ventana
    sprite.setScale(sf::Vector2f(scale, scale));
    
    // Centrar el sprite si es necesario
    float spriteWidth = textureSize.x * scale;
    float spriteHeight = textureSize.y * scale;
    sprite.setPosition(sf::Vector2f(
        (1000.0f - spriteWidth) / 2.0f,
        (600.0f - spriteHeight) / 2.0f
    ));

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
