#include <SFML/Graphics.hpp>

int main()
{
    // Crear una ventana
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML Texto");

    // Cargar la fuente de texto TTF
    sf::Font font;
    if (!font.openFromFile("./assets/fonts/Minecraft.ttf"))
    {
        // Manejar el error si no se puede cargar la fuente
        return -1;
    }

    // Cargar una fuente de texto
    sf::Font font2;
    if (!font2.openFromFile("./assets/fonts/Ring.ttf"))
    {
        // Manejar el error si no se puede cargar la fuente
        return -1;
    }

    // Crear un objeto de texto
    sf::Text text(font);
    text.setString("Ejemplo texto Minecraft!");
    text.setCharacterSize(29);
    text.setFillColor(sf::Color::White);

    // Crear un objeto de texto LOTR
    sf::Text text2(font2);
    text2.setString("Ejemplo texto LOTR");
    text2.setCharacterSize(40);
    text2.setPosition(sf::Vector2f(100, 100));

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        window.clear();
        window.draw(text);
        window.draw(text2);
        window.display();
    }

    return 0;
}
