#pragma once
#include <filesystem>
#include <string>
#include <string_view>
#include <memory>
#include <map>

namespace dae
{
	class Texture2D;
	class Font;
	class ResourceManager final
	{
	public:
		ResourceManager() = default;
		~ResourceManager();
		void Init(const std::filesystem::path &data);
		void ShutDown();
		std::shared_ptr<Texture2D> LoadTexture(std::string_view file);
		std::shared_ptr<Font> LoadFont(std::string_view file, uint8_t size);

	private:
		std::filesystem::path m_dataPath;
		bool m_isTtfInitialized{};

		void UnloadUnusedResources();

		std::map<std::string, std::shared_ptr<Texture2D>> m_loadedTextures;
		std::map<std::pair<std::string, uint8_t>, std::shared_ptr<Font>> m_loadedFonts;
	};
}
