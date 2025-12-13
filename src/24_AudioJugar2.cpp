#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Audio - Jugar 2");

    sf::Music music;
    if (!music.openFromFile("./assets/music/Jugar2.ogg"))
    {
        // Error al cargar el archivo de música
        return -1;
    }

    // Reproducir la música en bucle
    music.setLooping(true);
    music.play();

    // Texto para mostrar información
    sf::Font font;
    if (!font.openFromFile("./assets/fonts/Minecraft.ttf")) {
        return -1;
    }
    
    sf::Text infoText(font);
    infoText.setString("Reproduciendo: Jugar2.ogg\nPresiona ESC para salir");
    infoText.setCharacterSize(24);
    infoText.setFillColor(sf::Color::White);
    infoText.setPosition(sf::Vector2f(50, 250));

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }
            }
        }

        window.clear(sf::Color(30, 30, 30));
        window.draw(infoText);
        window.display();
    }

    return 0;
}
