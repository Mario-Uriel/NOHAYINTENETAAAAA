#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Reproductor de musica");

    sf::Music music;
    if (!music.openFromFile("./assets/music/musica.ogg"))
    {
        // Error al cargar el archivo de música
        return -1;
    }

    // Reproducir la música
    music.play();

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
        // Dibujar elementos adicionales en la ventana si es necesario
        window.display();

        // Esperar hasta que la música termine
        if (music.getStatus() != sf::SoundSource::Status::Playing)
        {
            window.close();
        }
    }

    return 0;
}
