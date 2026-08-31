#pragma once
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <vector>

class ResourceManager
{
public:
	void UnloadAll();
	const Texture2D* GetTexture(const std::string& key);
	Music* GetMusic(const std::string& key);
	Sound* GetSound(const std::string& key);
	const Font* GetFont(const std::string& key, int fontSize);

	static std::string GetResourcePath(const std::string& key);

private:
	std::unordered_map<std::string, Texture2D> textures;
	std::unordered_map<std::string, Music> musics;
	std::unordered_map<std::string, Sound> sounds;
	std::unordered_map<std::string, Font> fonts;

	std::vector<int>& GetKoreanCodepoints();
};

