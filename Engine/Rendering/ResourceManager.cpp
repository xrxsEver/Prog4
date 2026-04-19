#include <stdexcept>
#include <SDL3_ttf/SDL_ttf.h>
#include "ResourceManager.h"
#include "Texture2D.h"
#include "Font.h"

namespace fs = std::filesystem;

dae::ResourceManager::~ResourceManager()
{
	ShutDown();
}

void dae::ResourceManager::Init(const std::filesystem::path &dataPath)
{
	m_dataPath = dataPath;

	if (m_isTtfInitialized)
	{
		return;
	}

	if (!TTF_Init())
	{
		throw std::runtime_error(std::string("Failed to load support for fonts: ") + SDL_GetError());
	}

	m_isTtfInitialized = true;
}

void dae::ResourceManager::ShutDown()
{
	m_loadedTextures.clear();
	m_loadedFonts.clear();

	if (!m_isTtfInitialized)
	{
		return;
	}

	TTF_Quit();
	m_isTtfInitialized = false;
}

std::shared_ptr<dae::Texture2D> dae::ResourceManager::LoadTexture(const std::string_view file)
{
	const auto fullPath = m_dataPath / fs::path{file};
	const auto filename = fs::path(fullPath).filename().string();

	if (const auto it = m_loadedTextures.find(filename); it != m_loadedTextures.end())
	{
		return it->second;
	}

	auto texture = std::make_shared<Texture2D>(fullPath.string());
	m_loadedTextures.emplace(filename, texture);
	return texture;
}

std::shared_ptr<dae::Font> dae::ResourceManager::LoadFont(const std::string_view file, const uint8_t size)
{
	const auto fullPath = m_dataPath / fs::path{file};
	const auto filename = fs::path(fullPath).filename().string();
	const auto key = std::pair<std::string, uint8_t>(filename, size);

	if (const auto it = m_loadedFonts.find(key); it != m_loadedFonts.end())
	{
		return it->second;
	}

	auto font = std::make_shared<Font>(fullPath.string(), size);
	m_loadedFonts.emplace(key, font);
	return font;
}

void dae::ResourceManager::UnloadUnusedResources()
{
	for (auto it = m_loadedTextures.begin(); it != m_loadedTextures.end();)
	{
		if (it->second.use_count() == 1)
			it = m_loadedTextures.erase(it);
		else
			++it;
	}

	for (auto it = m_loadedFonts.begin(); it != m_loadedFonts.end();)
	{
		if (it->second.use_count() == 1)
			it = m_loadedFonts.erase(it);
		else
			++it;
	}
}
