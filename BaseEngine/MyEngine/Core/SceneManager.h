#pragma once
#include "pch/pch.h"
#include "Singleton.h"


enum struct Scene { MainMenu, Pause, Level1, Dead };

class SceneManager: public Singleton<SceneManager>
{
public:
	SceneManager();
	~SceneManager();

	void LoadScene(Scene scene);

	Scene GrabCurrentScene() const { return CurrentScene; };

private:
	Scene CurrentScene;
	std::vector<Entity> SceneEntityList;

	void UnloadCurrentScene();
	void LoadMainMenu();
	void LoadPause();
	void LoadLevel1();
	void LoadDead();
};

