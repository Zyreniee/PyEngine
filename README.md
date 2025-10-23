PyEngine

PyEngine — A lightweight, C++ game engine powered by Vulkan.

Hi! I'm 15 years old and currently studying at a vocational high school focused on software development. I created PyEngine to learn game engine development and Vulkan programming, and to provide a simple yet efficient engine for small projects and learning purposes.

PyEngine is designed to provide game developers with a simple yet efficient engine for handling core game logic and rendering. Built on Vulkan, it offers high-performance graphics while maintaining a modular and easy-to-understand structure. Perfect for learning, prototyping, or small-to-medium scale game projects.

Features

High-performance Vulkan rendering

Core game loop and scene management

Input handling (keyboard & mouse)

Basic physics and collision system

Modular and extensible architecture

Installation
git clone https://github.com/username/PyEngine.git
cd PyEngine
mkdir build && cd build
cmake ..
make

Usage
#include "PyEngine/Engine.h"
#include "PyEngine/Scene.h"

int main() {
    PyEngine::Engine engine;
    PyEngine::Scene scene("MyScene");

    // Add game objects
    scene.addObject(player);

    engine.run(scene);
    return 0;
}

Contributing

Report issues: Issues

Submit code contributions: Pull Requests

Help improve the engine and share your ideas!

License

MIT License
