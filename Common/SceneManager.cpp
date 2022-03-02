#include <SFML/Graphics.hpp>
#include "SceneManager.h"
#include <iostream>


SceneManager::SceneManager(std::unique_ptr<IScene>& _scene):
scene(_scene)
{}

void SceneManager::Run() 
{

    // SFML Window Creation
    // --------------------------------------------------------------------------------- 
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antialiasingLevel = 4;
    settings.majorVersion = 3;
    settings.minorVersion = 3;
    settings.attributeFlags = sf::ContextSettings::Attribute::Core;
    sf::RenderWindow window(sf::VideoMode::getFullscreenModes()[0], "MPvsX", sf::Style::Default, settings);
    window.setFramerateLimit(30);
    window.setActive(true);

    float ratio = static_cast<float>(window.getSize().x) / window.getSize().y;
    scene->InitOpenGL( ratio );

    /* main program loop */
    bool running = true;
    while (running)
    {
        // handle window close
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                running = false;
            }
        }
        // update GL data and draw context
        scene->UpdateFrame();
        window.display();
    }
}

SceneManager::~SceneManager() 
{
}
