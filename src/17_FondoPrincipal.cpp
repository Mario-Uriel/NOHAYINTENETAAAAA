#include <SFML/Graphics.hpp>

int main()
{
    // Crear una ventana
    sf::RenderWindow window(sf::VideoMode({1000, 600}), "SFML - Fondo Principal");

    // Cargar la imagen del fondo principal desde un archivo
    sf::Texture texture;
    if (!texture.loadFromFile("./assets/images/Fondo principal.png"))
    {
        // Manejar el error si no se puede cargar la imagen
        return -1;
    }

    // Crear un sprite y asignarle la textura
    sf::Sprite sprite(texture);
    
    // Escalar el sprite para que se ajuste a la ventana
    sf::Vector2u textureSize = texture.getSize();
    float scaleX = 1000.0f / textureSize.x;
    float scaleY = 600.0f / textureSize.y;
    sprite.setScale(sf::Vector2f(scaleX, scaleY));

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
