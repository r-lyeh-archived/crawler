#ifndef SFBL_H
#define SFBL_H

#include <Box2D\Box2D.h>
#include <iostream>
#include <SFML\Graphics.hpp>
#include <SFML\OpenGL.hpp>
#include <functional>
#define MIN_RAYS 3
#define MAX_RAYS 0xFFFF // 65535

class RaysCastCallback : public b2RayCastCallback
{
public:
    RaysCastCallback(b2Vec2 startPoint) : m_hit(false), startPoint(startPoint){
    }

    float32 ReportFixture(b2Fixture*, const b2Vec2& point, const b2Vec2&, float32 fraction) {
        float dist = b2Distance(startPoint, point);
        if(dist > this->dist){// get the farthest intersection point
            this->dist = dist;
            m_point = point;
            m_fraction = fraction;
            m_hit = true;
        }
        return 1;// keep going to get all intersection points
    }
    bool m_hit;
    b2Vec2 m_point;
    float32 m_fraction;
private:
    b2Vec2 startPoint;
    float dist = 0.0f;
};

namespace sfbl{
    struct Light{
        Light(float radius, sf::Vector2f position, sf::Color color){
            this->radius = radius;
            this->position = position;
            this->color = color;
        }
        float radius;
        sf::Vector2f position;
        sf::Color color;
    };

    class WorldLight
    {
    public:
        ///add movable PointLight
        ///\param pointer to the specific light
        ///\param number of rays
        void addAsPointLight(Light*light, sf::Uint16 rays);

        /// add static PointLight
        ///\param number of rays
        ///\param radius of light
        ///\param position of light
        ///\param color of light
        void addAsPointLight(sf::Uint16 rays, float radius, sf::Vector2f position, sf::Color color);

        /// add movable SpotLight
        ///\param pointer to the specific light
        ///\param number of rays
        void addAsSpotLight(Light*light, sf::Uint16 rays);

        /// add static SpotLight
        ///\param number of rays
        ///\param radius of light
        ///\param position of light
        ///\param color of light
        void addAsSpotLight(sf::Uint16 rays, float radius, sf::Vector2f position, sf::Color color);

        ///add movable ConeLight
        ///\param pointer to the specific light
        ///\param number of rays
        ///\param cutOffAngle in degres
        ///\param rotation in degres
        void addAsConeLight(Light*light, sf::Uint16 rays, float cutOffAngle, float rotation);

        ///add static ConeLight
        ///\param number of rays
        ///\param cutOffAngle in degres
        ///\param rotation in degres
        ///\param radius of light
        ///\param position of light
        ///\param color of light
        void addAsConeLight(sf::Uint16 rays, float cutOffAngle, float rotation, float radius, sf::Vector2f position, sf::Color color);

        /// apply the params that you've changed manually
        void updatePointLight(Light*light);

        /// apply the params that you've changed manually
        void updateSpotLight(Light*light);

        /// apply the params that you've changed manually
        void updateConeLight(Light*light, float cutOffAngle, float rotation);

        /// update and render lights
        void updateNrender();

        /// update lights
        void update();

        /// render lights
        void render();

        /// move viewport of lights by x , y
        void moveViewport(int x, int y);

        /// set viewport of lights into specific position
        void setViewportPos(int x, int y);

        /// set viewport of lights into specific position
        void setViewportPos(sf::Vector2i pos);

        /// get viewport of lights
        const sf::Vector2f& getViewportPos();

        /// set the darkness of scene
        /// \param the darkness of scene 0 -> 255
        void setDarkness(sf::Uint8 darkness);

        /// get darkness
        sf::Uint8 getDarkness();

        /// remove light
        /// \param the index of light
        /// \note "index" its existence is not verified.
        void removeLight(int index);

        /// remove dynamic light
        /// \param pointer to light
        void removeLight(Light*light);

        WorldLight(b2World* world, sf::RenderWindow* window);

        ~WorldLight();

    private:
        sf::Vector2f _viewPort;
        sf::Vector2u initSize;

        b2World* _world;
        sf::RenderWindow* _window;
        sf::RenderTexture _render;

        std::vector<Light*>_lights;
        std::vector<bool>isPointLight;
        std::vector<sf::VertexArray>lightMesh,fixedMesh;
        sf::Shader lightShader;
        sf::Color darkness = sf::Color::Black;
    };

}// sfbl namespace

#endif // SFBL_H
