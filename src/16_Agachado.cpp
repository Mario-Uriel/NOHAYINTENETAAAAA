#include <SFML/Graphics.hpp>

int main()
{
    // Crear una ventana
    sf::RenderWindow window(sf::VideoMode({800, 600}), "agachado");

    // Cargar la imagen desde un archivo
    sf::Texture texture;
    if (!texture.loadFromFile("./assets/images/agachado.png"))
    {
        // Manejar el error si no se puede cargar la imagen
        return -1;
    }

    // Crear un sprite y asignarle la textura
    sf::Sprite sprite(texture);

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
