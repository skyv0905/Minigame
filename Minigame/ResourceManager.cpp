#include "ResourceManager.h"
#include <iostream>

void ResourceManager::UnloadAll()
{
	for (auto& kv : textures)
	{
		std::cout << "Unloading Texture: " << kv.first << '\n';
		UnloadTexture(kv.second);
	}
	for (auto& kv : musics)
	{
		std::cout << "Unloading Music: " << kv.first << '\n';
		UnloadMusicStream(kv.second);
	}
	for (auto& kv : sounds)
	{
		std::cout << "Unloading Sound: " << kv.first << '\n';
		UnloadSound(kv.second);
	}
	for (auto& kv : fonts)
	{
		std::cout << "Unloading Font: " << kv.first << '\n';
		UnloadFont(kv.second);
	}

	textures.clear();
	musics.clear();
	sounds.clear();
	fonts.clear();
}

const Texture2D* ResourceManager::GetTexture(const std::string& key)
{
	auto it = textures.find(key);
	if (it != textures.end())
	{
		return &it->second;
	}

	std::string path = GetResourcePath(key);
	if (!FileExists(path.c_str()))
	{
		std::cerr << "Texture file not found: " << path << '\n';
		return nullptr;
	}

	std::cout << "Loading Texture: " << path << '\n';
	Texture texture = LoadTexture(path.c_str());
	if (!IsTextureValid(texture))
	{
		std::cerr << "Failed to load texture: " << path << '\n';
		return nullptr;
	}

	auto [newIt, inserted] = textures.emplace(key, texture);

	return &newIt->second;
}

Music* ResourceManager::GetMusic(const std::string& key)
{
	auto it = musics.find(key);
	if (it != musics.end())
	{
		return &it->second;
	}

	std::string path = GetResourcePath(key);
	if (!FileExists(path.c_str()))
	{
		std::cerr << "Music file not found: " << path << '\n';
		return nullptr;
	}

	std::cout << "Loading Music: " << path << '\n';
	Music music = LoadMusicStream(path.c_str());
	if (!IsMusicValid(music))
	{
		std::cerr << "Failed to load music: " << path << '\n';
		return nullptr;
	}
	auto [newIt, inserted] = musics.emplace(key, music);

	return &newIt->second;
}

Sound* ResourceManager::GetSound(const std::string& key)
{
	auto it = sounds.find(key);
	if (it != sounds.end())
	{
		return &it->second;
	}

	std::string path = GetResourcePath(key);
	if (!FileExists(path.c_str()))
	{
		std::cerr << "Sound file not found: " << path << '\n';
		return nullptr;
	}

	std::cout << "Loading Sound: " << path << '\n';
	Sound sound = LoadSound(path.c_str());
	if (!IsSoundValid(sound))
	{
		std::cerr << "Failed to load sound: " << path << '\n';
		return nullptr;
	}
	auto [newIt, inserted] = sounds.emplace(key, sound);

	return &newIt->second;
}

const Font* ResourceManager::GetFont(const std::string& key, int fontSize)
{
	//std::string cacheKey = key + "_" + std::to_string(fontSize);
	std::string cacheKey = key;
	auto it = fonts.find(cacheKey);
	if (it != fonts.end())
	{
		return &it->second;
	}

	std::string path = GetResourcePath(key);
	if (!FileExists(path.c_str()))
	{
		std::cerr << "Font file not found: " << path << '\n';
		return nullptr;
	}

	std::cout << "Loading Font: " << path << '\n';
	auto& codepoints = GetKoreanCodepoints();
	//Font font = LoadFontEx(path.c_str(), fontSize, codepoints.data(), static_cast<int>(codepoints.size()));
	Font font = LoadFontEx(path.c_str(), 32, codepoints.data(), static_cast<int>(codepoints.size()));
	if (!IsFontValid(font))
	{
		std::cerr << "Failed to load font: " << path << '\n';
		return nullptr;
	}
	auto [newIt, inserted] = fonts.emplace(cacheKey, font);

	return &newIt->second;
}

std::string ResourceManager::GetResourcePath(const std::string& key)
{
	return std::string(GetApplicationDirectory()) + "Data/Resources/" + key;
}

std::vector<int>& ResourceManager::GetKoreanCodepoints()
{
	static std::vector<int> codepoints = []()
		{
			std::vector<int> result;

			for (int i = 0x20; i <= 0x7E; ++i)
			{
				result.push_back(i);
			}

			for (int i = 0x3131; i <= 0x318E; ++i)
			{
				result.push_back(i);
			}

			for (int i = 0xAC00; i <= 0xD7A3; ++i)
			{
				result.push_back(i);
			}

			return result;
		}();

	return codepoints;
}
