#include <SFML/Graphics.hpp>

int main()
{
    // Crear una ventana
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML - Bala Ballesta");

    // Cargar la imagen de la bala de ballesta desde un archivo
    sf::Texture texture;
    if (!texture.loadFromFile("./assets/images/Bala ballesta .png"))
    {
        // Manejar el error si no se puede cargar la imagen
        return -1;
    }

    // Crear un sprite y asignarle la textura
    sf::Sprite sprite(texture);
    
    // Escalar el sprite para mejor visualización
    sprite.setScale(sf::Vector2f(0.3f, 0.3f));
    
    // Centrar el sprite en la ventana después de escalarlo
    sf::Vector2u textureSize = texture.getSize();
    sprite.setPosition(sf::Vector2f(
        (800.0f - textureSize.x * 0.3f) / 2.0f,
        (600.0f - textureSize.y * 0.3f) / 2.0f
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

        // Limpiar la ventana con un fondo oscuro para mejor contraste
        window.clear(sf::Color(50, 50, 50));

        // Dibujar el sprite en la ventana
        window.draw(sprite);

        // Mostrar la ventana
        window.display();
    }

    return 0;
}
