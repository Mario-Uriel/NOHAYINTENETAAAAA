#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <iostream>
using namespace std;

int main()
{
    float fuerza = 1.0f;

    // Crear una ventana de SFML
    sf::RenderWindow ventana(sf::VideoMode({800, 600}), "Ejemplo de Fisica con Box2D y SFML");

    // Crear un mundo de Box2D
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, 10.0f};
    b2WorldId mundo = b2CreateWorld(&worldDef);

    // Crear un suelo estático
    b2BodyDef cuerpoSueloDef = b2DefaultBodyDef();
    cuerpoSueloDef.position = {400.0f, 500.0f}; // Posición del centro del cuerpo
    b2BodyId cuerpoSuelo = b2CreateBody(mundo, &cuerpoSueloDef);

    // Crear una forma rectangular
    int boxWidth = 600; // 600 pixeles de ancho
    int boxHeight = 10; // 10 pixeles de alto
    b2Polygon formaSuelo = b2MakeBox(boxWidth / 2.0f, boxHeight / 2.0f);

    // Agregar la forma al cuerpo
    b2ShapeDef fixtureSueloDef = b2DefaultShapeDef();
    fixtureSueloDef.friction = 1.0f;
    b2CreatePolygonShape(cuerpoSuelo, &fixtureSueloDef, &formaSuelo);

    // Crear un cuerpo dinámico
    b2BodyDef cuerpoBolaDef = b2DefaultBodyDef();
    cuerpoBolaDef.type = b2_dynamicBody;
    cuerpoBolaDef.position = {400.0f, 300.0f};
    b2BodyId cuerpoBola = b2CreateBody(mundo, &cuerpoBolaDef);

    // Crear una forma circular
    b2Circle formaBola;
    formaBola.center = {0.0f, 0.0f};
    formaBola.radius = 25.0f;

    // Agregar la forma al cuerpo
    b2ShapeDef fixtureBolaDef = b2DefaultShapeDef();
    fixtureBolaDef.density = 0.01f;
    fixtureBolaDef.friction = 0.7f;
    b2CreateCircleShape(cuerpoBola, &fixtureBolaDef, &formaBola);

    // Bucle principal del juego
    while (ventana.isOpen())
    {
        // Procesar eventos
        while (const auto evento = ventana.pollEvent())
        {
            if (evento->is<sf::Event::Closed>())
                ventana.close();
        }

        // Controlar la bola con el teclado
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            b2Body_ApplyLinearImpulse(cuerpoBola, {-fuerza, 0.0f}, b2Body_GetPosition(cuerpoBola), true);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            b2Body_ApplyLinearImpulse(cuerpoBola, {fuerza, 0.0f}, b2Body_GetPosition(cuerpoBola), true);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            b2Body_ApplyLinearImpulse(cuerpoBola, {0.0f, -fuerza}, b2Body_GetPosition(cuerpoBola), true);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            b2Body_ApplyLinearImpulse(cuerpoBola, {0.0f, fuerza}, b2Body_GetPosition(cuerpoBola), true);

        // Actualizar el mundo de Box2D
        // Ajustar el valor de 1.0 / 60.0 para cambiar la velocidad de la simulación física
        b2World_Step(mundo, 1.0f / 60.0f, 4);
        b2Vec2 posBola = b2Body_GetPosition(cuerpoBola);
        cout << "Posicion de la bola: " << posBola.x << ", " << posBola.y << endl;

        // Limpiar la ventana
        ventana.clear();

        // Dibujar el suelo
        b2Vec2 posSuelo = b2Body_GetPosition(cuerpoSuelo);
        sf::RectangleShape suelo(sf::Vector2f(boxWidth, boxHeight));
        suelo.setOrigin(sf::Vector2f(boxWidth / 2.0f, boxHeight / 2.0f)); // El origen x,y está en el centro de la forma
        suelo.setPosition(sf::Vector2f(posSuelo.x, posSuelo.y));
        ventana.draw(suelo);

        // Dibujar la bola
        sf::CircleShape bola(formaBola.radius);
        bola.setOrigin(sf::Vector2f(formaBola.radius, formaBola.radius));
        bola.setFillColor(sf::Color::Red);
        bola.setPosition(sf::Vector2f(posBola.x, posBola.y));
        ventana.draw(bola);

        // Mostrar la ventana
        ventana.display();
    }

    // Destruir el mundo
    b2DestroyWorld(mundo);

    return 0;
}

