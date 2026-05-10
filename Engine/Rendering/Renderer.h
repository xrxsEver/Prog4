#pragma once
#include <memory>
#include "Singleton.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;

namespace dae
{
	class Texture2D;
	class Minigin;

	struct Color {
		unsigned char r, g, b, a;
	};

	struct Rect {
		float x, y, width, height;
	};

	/**
	 * Simple RAII wrapper for the SDL renderer
	 */
	class Renderer final : public Singleton<Renderer>
	{
		struct SDLRendererDeleter final
		{
			void operator()(SDL_Renderer *renderer) const noexcept;
		};

		using RendererPtr = std::unique_ptr<SDL_Renderer, SDLRendererDeleter>;

		RendererPtr m_renderer{};
		SDL_Window *m_window{};

		Color m_clearColor{};

		friend class Minigin;
		void RenderClear() const;
		void RenderPresent() const;

	public:
		void Init(SDL_Window *window);

		void ShutDown();
		void Destroy();

		void RenderTexture(const Texture2D &texture, float x, float y) const;
		void RenderTexture(const Texture2D &texture, float x, float y, float width, float height) const;
		void RenderTexture(const Texture2D &texture, const Rect &srcRect, float x, float y, float width, float height) const;

		SDL_Texture* CreateTextureFromSurface(SDL_Surface* surface) const;

		void InitImGui() const;
		void ImGuiNewFrame() const;
		void RenderImGui() const;
		void ShutDownImGui() const;

		const Color &GetBackgroundColor() const { return m_clearColor; }
		void SetBackgroundColor(const Color &color) { m_clearColor = color; }
	};
}
