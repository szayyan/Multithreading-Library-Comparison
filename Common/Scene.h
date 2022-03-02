#pragma once

class Scene
{
public:
    virtual void Init(float ratio) = 0;
    virtual void Run() = 0;
    virtual void Update() = 0;
    virtual ~Scene() {};
};

class IScene
{
    /* Abstract class for all viewable scenes */
public:
    virtual void InitOpenGL(float ratio) = 0;
    virtual void UpdateFrame() = 0;
    virtual ~IScene() {};
};