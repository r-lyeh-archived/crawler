#include <SFML\Graphics.hpp>
#include <iostream>
#include <Box2D\Box2D.h>
#include "sfbl.h"

float timeStep = 1.f / 60.f;
int velocityIteration = 6;
int posIteration = 2;
using namespace std;
b2Body* createCircleBody(b2World* w, b2Vec2 pos){
    b2BodyDef def;
    def.position = pos;
    def.type = b2_dynamicBody;
    b2Body* body = w->CreateBody(&def);

    b2CircleShape shape;
    shape.m_radius = 10.f;
    shape.m_p.Set(10.f, 10.f);
    b2FixtureDef fixDef;
    fixDef.shape = &shape;
    fixDef.restitution = 1.1f;
    fixDef.density = 0.9f;
    body->CreateFixture(&fixDef);

    return &body[0];
}

int main()
{
    srand(time(NULL));
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Project");
    b2World world(b2Vec2(0.f, 9.86f));
    sfbl::WorldLight worldLight(&world, &window);
    worldLight.setDarkness(250);// make scene more darker
        
    sf::CircleShape circle(10.f);
    circle.setFillColor(sf::Color::Yellow);
    b2ChainShape edge;
    b2Vec2 vecs[4];
    vecs[0].Set(0,0);
    vecs[1].Set(0,720);
    vecs[2].Set(1280,720);
    vecs[3].Set(1280,0);
    edge.CreateChain(vecs, 4);
    b2Body* fall = world.CreateBody(new b2BodyDef());
    fall->CreateFixture(&edge, 1.0);

    float radius = 400.f;
    sfbl::Light light(radius, sf::Vector2f(0, 0), sf::Color::White);
    worldLight.addAsPointLight(&light, 1000);
    
    sf::Texture bg;
    bg.loadFromFile("bg.jpg");

    sf::Clock clock;
    int fps = 0;
    while (window.isOpen())
    {
        world.Step(timeStep, velocityIteration, posIteration);

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                window.close();
            if (event.type == sf::Event::MouseButtonReleased){
                switch (event.mouseButton.button)
                {
                case sf::Mouse::Left:
                    worldLight.addAsPointLight(720, radius, sf::Vector2f(float(event.mouseButton.x), float(event.mouseButton.y)),
                        sf::Color(rand() % 256, 64, rand() % 256));
                    break;
                case sf::Mouse::Middle:
                    worldLight.addAsConeLight(240, 35, rand()%360, radius, sf::Vector2f(float(event.mouseButton.x), float(event.mouseButton.y)),
                        sf::Color(rand() % 256, 64, rand() % 256));
                    break;
                case sf::Mouse::Right:
                    worldLight.addAsSpotLight(720, radius, sf::Vector2f(float(event.mouseButton.x), float(event.mouseButton.y)),
                        sf::Color(rand() % 256, 64, rand() % 256));
                    break;
                default:
                    // Don't do anything
                    break;
                }
            }
            else if (event.type == sf::Event::MouseWheelMoved){
                radius += event.mouseWheel.delta*10;
            }
            else if (event.type == sf::Event::MouseMoved){
                light.position.x = (float)event.mouseMove.x;
                light.position.y = (float)event.mouseMove.y;
                light.radius = radius;
                worldLight.updatePointLight(&light);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)){
                    createCircleBody(&world, b2Vec2((float)event.mouseMove.x, (float)event.mouseMove.y));
                }
            }
            else if(event.type == sf::Event::KeyPressed){
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
                    worldLight.moveViewport(10,0);
                }else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
                    worldLight.moveViewport(-10,0);
                }
            }
        }

        window.clear();
        window.draw(sf::Sprite(bg));
        for (b2Body* b = world.GetBodyList(); b; b = b->GetNext())
        {
            circle.setPosition(b->GetPosition().x, b->GetPosition().y);
            circle.setRotation(b->GetAngle()*180.f / b2_pi);
            window.draw(circle);
        }
        worldLight.updateNrender();
        if(clock.getElapsedTime().asMilliseconds() >= 1000){
            cout<<"FPS = "<<fps<<endl;
            clock.restart();
            fps = 0;
        }else{
            fps++;
        }
        window.display();
    }
    return 0;
}
