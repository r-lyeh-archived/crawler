#include "sfbl.h"


sfbl::WorldLight::WorldLight(b2World* world, sf::RenderWindow* window)
{
    this->_world = world;
    this->_window = window;
    lightShader.loadFromFile("Light.frag", sf::Shader::Fragment);
    initSize = window->getSize();
    _render.create(initSize.x, initSize.y);
    _render.setSmooth(true);
    lightShader.setParameter("height", initSize.y);
}

sfbl::WorldLight::~WorldLight()
{
    for (size_t i = 0; i < lightMesh.size(); i++)
    {
        lightMesh[i].clear();
    }
    lightMesh.clear();
    _lights.clear();
}

void sfbl::WorldLight::removeLight(int index){
    fixedMesh.erase(fixedMesh.begin() + index);
    lightMesh.erase(lightMesh.begin() + index);
    _lights.erase(_lights.begin() + index);
}

void sfbl::WorldLight::removeLight(Light* light){
    for (size_t i = 0; i < _lights.size(); i++)
    {
        if (_lights[i] == &light[0]){
            fixedMesh.erase(fixedMesh.begin() + i);
            lightMesh.erase(lightMesh.begin() + i);
            _lights.erase(_lights.begin() + i);
            break;
        }
    }
}

void sfbl::WorldLight::addAsPointLight(Light*light, sf::Uint16 rays){
    this->_lights.push_back(&light[0]);
    sf::VertexArray array;
    sf::Color endColor = light->color;
    endColor.a = 0;
    if (rays < MIN_RAYS){
        rays = MIN_RAYS;
    }
    float perRay = b2_pi * 2 / float(rays);
    array.setPrimitiveType(sf::TrianglesFan);
    sf::Vector2f pos = light->position - _viewPort;
    array.append(sf::Vertex(pos, light->color));
    for (size_t i = 0; i <= rays; i++)
    {
        float x = cos(perRay*i)*light->radius + pos.x;
        float y = sin(perRay*i)*light->radius + pos.y;
        array.append(sf::Vertex(sf::Vector2f(x, y), endColor));
    }
    lightMesh.push_back(array);
    fixedMesh.push_back(array);
    isPointLight.push_back(true);
}

void sfbl::WorldLight::addAsPointLight(sf::Uint16 rays, float radius, sf::Vector2f pos, sf::Color color){
    this->_lights.push_back(new Light(radius, pos, color));
    pos -= _viewPort;
    sf::VertexArray array;
    sf::Color endColor = color;
    endColor.a = 0;
    if (rays < MIN_RAYS){
        rays = MIN_RAYS;
    }
    float perRay = b2_pi*2 / float(rays);
    array.setPrimitiveType(sf::TrianglesFan);
    array.append(sf::Vertex(pos, color));
    for (size_t i = 0; i <= rays; i++)
    {
        float x = cos(perRay*i)*radius+pos.x;
        float y = sin(perRay*i)*radius+pos.y;
        array.append(sf::Vertex(sf::Vector2f(x, y), endColor));
    }
    lightMesh.push_back(array);
    fixedMesh.push_back(array);
    isPointLight.push_back(true);
}

void sfbl::WorldLight::addAsSpotLight(Light*light, sf::Uint16 rays){
    this->addAsPointLight(&light[0], rays);
    isPointLight.back() = false;
}

void sfbl::WorldLight::addAsSpotLight(sf::Uint16 rays, float radius, sf::Vector2f pos, sf::Color color){
    this->addAsPointLight(rays, radius, pos, color);
    isPointLight.back() = false;
}

void sfbl::WorldLight::addAsConeLight(Light*light, sf::Uint16 rays, float cutOffAngle, float rotation){
    if (cutOffAngle > 180)
        cutOffAngle = 180;
    this->_lights.push_back(&light[0]);
    sf::VertexArray array;
    sf::Color endColor = light->color;
    endColor.a = 0;
    if (rays < MIN_RAYS){
        rays = MIN_RAYS;
    }
    cutOffAngle *= b2_pi / 180.f;// Convert from degree to radians
    rotation *= b2_pi / 180.f;// Convert from degree to radians
    float perRay = cutOffAngle * 2 / float(rays);
    array.setPrimitiveType(sf::TrianglesFan);
    sf::Vector2f pos = light->position - _viewPort;
    array.append(sf::Vertex(pos, light->color));
    for (size_t i = 0; i <= rays; i++)
    {
        float x = cos((perRay*i) - cutOffAngle + rotation)*light->radius + pos.x;
        float y = sin((perRay*i) - cutOffAngle + rotation)*light->radius + pos.y;
        array.append(sf::Vertex(sf::Vector2f(x, y), endColor));
    }
    lightMesh.push_back(array);
    fixedMesh.push_back(array);
    isPointLight.push_back(false);
}

void sfbl::WorldLight::addAsConeLight(sf::Uint16 rays, float cutOffAngle, float rotation, float radius, sf::Vector2f pos, sf::Color color){
    if (cutOffAngle > 180)
        cutOffAngle = 180;
    this->_lights.push_back(new Light(radius, pos, color));
    pos -= _viewPort;
    sf::VertexArray array;
    sf::Color endColor = color;
    endColor.a = 0;
    if (rays < MIN_RAYS){
        rays = MIN_RAYS;
    }
    cutOffAngle *= b2_pi / 180.f;// Convert from degree to radians
    rotation *= b2_pi / 180.f;// Convert from degree to radians
    float perRay = cutOffAngle * 2 / float(rays);
    array.setPrimitiveType(sf::TrianglesFan);
    array.append(sf::Vertex(pos, color));
    for (size_t i = 0; i <= rays; i++)
    {
        float x = cos((perRay*i) - cutOffAngle + rotation)*radius + pos.x;
        float y = sin((perRay*i) - cutOffAngle + rotation)*radius + pos.y;
        array.append(sf::Vertex(sf::Vector2f(x, y), endColor));
    }
    lightMesh.push_back(array);
    fixedMesh.push_back(array);
    isPointLight.push_back(false);
}

void sfbl::WorldLight::updatePointLight(Light* light){
    sf::Vector2f pos = light->position - _viewPort;
    for (size_t i = 0; i < _lights.size(); i++)
    {
        if (_lights[i] == &light[0]){
            fixedMesh[i][0].color = light->color;
            fixedMesh[i][0].position = pos;
            sf::Color endColor = light->color;
            endColor.a = 0;
            float perRay = 2*b2_pi / (fixedMesh[i].getVertexCount() - 2);
            for (size_t o = 1; o < fixedMesh[i].getVertexCount(); o++)
            {
                fixedMesh[i][o].color = endColor;
                fixedMesh[i][o].position.x = cos(perRay*(o - 1))*light->radius + pos.x;
                fixedMesh[i][o].position.y = sin(perRay*(o - 1))*light->radius + pos.y;
            }
            break;
        }
    }
}

void sfbl::WorldLight::updateSpotLight(Light* light){
    this->updatePointLight(&light[0]);
}

void sfbl::WorldLight::updateConeLight(Light* light, float cutOffAngle, float rotation){
    sf::Vector2f pos = light->position - _viewPort;
    for (size_t i = 0; i < _lights.size(); i++)
    {
        if (_lights[i] == &light[0]){
            fixedMesh[i][0].color = light->color;
            fixedMesh[i][0].position = pos;
            sf::Color endColor = light->color;
            endColor.a = 0;
            cutOffAngle *= b2_pi/180.f;
            float perRay = 2*cutOffAngle / (fixedMesh[i].getVertexCount() - 2);
            for (size_t o = 1; o < fixedMesh[i].getVertexCount(); o++)
            {
                fixedMesh[i][o].color = endColor;
                fixedMesh[i][o].position.x = cos(perRay*(o - 1) - cutOffAngle + rotation)*light->radius + pos.x;
                fixedMesh[i][o].position.y = sin(perRay*(o - 1) - cutOffAngle + rotation)*light->radius + pos.y;
            }
            break;
        }
    }
}

void sfbl::WorldLight::updateNrender(){
   this->update();
   this->render();
}

void sfbl::WorldLight::update(){
    b2RayCastInput in;
    in.maxFraction = 1.f;
    for (size_t m = 0; m < fixedMesh.size(); m++)
    {
        for (size_t i = 0; i < fixedMesh[m].getVertexCount(); i++)
        {
            lightMesh[m][i].position = fixedMesh[m][i].position;
            lightMesh[m][i].color = fixedMesh[m][i].color;
        }
    }
    for (size_t m = 0; m < fixedMesh.size(); m++)
    {
        size_t count = fixedMesh[m].getVertexCount();
        in.p1.Set(fixedMesh[m][0].position.x, fixedMesh[m][0].position.y);
        for (size_t i = 1; i < count; i++)
        {
            in.p2.Set(fixedMesh[m][i].position.x, fixedMesh[m][i].position.y);
            RaysCastCallback callBack(in.p2);
            _world->RayCast(&callBack, in.p2, in.p1);
            if(callBack.m_hit){
                lightMesh[m][i].position.x = callBack.m_point.x;
                lightMesh[m][i].position.y = callBack.m_point.y;
                lightMesh[m][i].color.a = 255 * callBack.m_fraction;
            }
        }
    }
    lightShader.setParameter("viewport", _viewPort);
}

void sfbl::WorldLight::render(){
    _render.clear(darkness);
    sf::RenderStates states;
    states.shader = &lightShader;
    states.blendMode = sf::BlendAdd;
    for (size_t i = 0; i < _lights.size(); i++)
    {
        if(isPointLight[i]){
            lightShader.setParameter("lightPos", _lights[i]->position);
            lightShader.setParameter("lightColor",_lights[i]->color);
            lightShader.setParameter("radius", _lights[i]->radius);
            _render.draw(lightMesh[i], states);
        }
        else{
            _render.draw(lightMesh[i], sf::BlendAdd); // we can get spotlight effect just by disabling shader
        }
    }
    _render.display();
    _window->draw(sf::Sprite(_render.getTexture()), sf::BlendMultiply);
    _window->draw(sf::Sprite(_render.getTexture()), sf::BlendAdd);
}

void sfbl::WorldLight::moveViewport(int x, int y){
    for (size_t m = 0; m < fixedMesh.size(); m++)
    {
        for (size_t i = 0; i < fixedMesh[m].getVertexCount(); i++)
        {
            fixedMesh[m][i].position -= sf::Vector2f(x, y);
        }
    }
    _viewPort += sf::Vector2f(x, y);
}

void sfbl::WorldLight::setViewportPos(int x, int y){
    sf::Vector2f move = (_viewPort - sf::Vector2f(x, y));
    for (size_t m = 0; m < fixedMesh.size(); m++)
    {
        for (size_t i = 0; i < fixedMesh[m].getVertexCount(); i++)
        {
            fixedMesh[m][i].position += move;
        }
    }
    _viewPort.x = x;
    _viewPort.y = y;
}

void sfbl::WorldLight::setViewportPos(sf::Vector2i p){
    this->setViewportPos(p.x, p.y);
}

const sf::Vector2f& sfbl::WorldLight::getViewportPos(){
    return _viewPort;
}

void sfbl::WorldLight::setDarkness(sf::Uint8 darkness){
    this->darkness.r = 255-darkness;
    this->darkness.g = 255-darkness;
    this->darkness.b = 255-darkness;
}

sf::Uint8 sfbl::WorldLight::getDarkness(){
    return darkness.r;
}
