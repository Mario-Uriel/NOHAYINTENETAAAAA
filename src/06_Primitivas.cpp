#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML Window");

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear();

        // Dibujar primitivas básicas
        sf::CircleShape circle(50);
        circle.setFillColor(sf::Color::Red);
        circle.setPosition(sf::Vector2f(100, 100));
        window.draw(circle);

        sf::RectangleShape rectangle(sf::Vector2f(200, 100));
        rectangle.setFillColor(sf::Color::Green);
        rectangle.setPosition(sf::Vector2f(300, 200));
        window.draw(rectangle);

        sf::ConvexShape triangle;
        triangle.setPointCount(3);
        triangle.setPoint(0, sf::Vector2f(100, 300));
        triangle.setPoint(1, sf::Vector2f(200, 300));
        triangle.setPoint(2, sf::Vector2f(150, 400));
        triangle.setFillColor(sf::Color::Blue);
        window.draw(triangle);

        window.display();
    }

    return 0;
}
