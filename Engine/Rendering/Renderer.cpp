#include <stdexcept>
#include <cstring>
#include <iostream>
#include <array>
#include <cmath>
#include <SDL3/SDL.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include "Renderer.h"
#include "Texture2D.h"

void dae::Renderer::SDLRendererDeleter::operator()(SDL_Renderer *renderer) const noexcept
{
	if (renderer != nullptr)
	{
		SDL_DestroyRenderer(renderer);
	}
}

void dae::Renderer::Init(SDL_Window *window)
{
	m_window = window;
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	m_renderer.reset(SDL_CreateRenderer(window, nullptr));
	if (m_renderer == nullptr)
	{
		std::cout << "Failed to create the renderer: " << SDL_GetError() << "\n";
		throw std::runtime_error(std::string("SDL_CreateRenderer Error: ") + SDL_GetError());
	}
}

void dae::Renderer::ShutDown()
{
	m_renderer.reset();
	m_window = nullptr;
}

void dae::Renderer::RenderClear() const
{
	const auto &color = GetBackgroundColor();
	SDL_SetRenderDrawColor(m_renderer.get(), color.r, color.g, color.b, color.a);
	SDL_RenderClear(m_renderer.get());
}

void dae::Renderer::RenderPresent() const
{
	SDL_RenderPresent(m_renderer.get());
}

void dae::Renderer::Destroy()
{
	ShutDown();
}

void dae::Renderer::RenderTexture(const Texture2D &texture, const float x, const float y) const
{
	SDL_FRect dst{};
	dst.x = x;
	dst.y = y;
	SDL_GetTextureSize(texture.GetSDLTexture(), &dst.w, &dst.h);
	SDL_RenderTexture(m_renderer.get(), texture.GetSDLTexture(), nullptr, &dst);
}

void dae::Renderer::RenderTexture(const Texture2D &texture, const float x, const float y, const float width, const float height) const
{
	SDL_FRect dst{};
	dst.x = x;
	dst.y = y;
	dst.w = width;
	dst.h = height;
	SDL_RenderTexture(m_renderer.get(), texture.GetSDLTexture(), nullptr, &dst);
}

void dae::Renderer::RenderTexture(const Texture2D &texture, const Rect &srcRect, const float x, const float y, const float width, const float height) const
{
	SDL_FRect dst{};
	dst.x = x;
	dst.y = y;
	dst.w = width;
	dst.h = height;
	SDL_FRect src{srcRect.x, srcRect.y, srcRect.width, srcRect.height};
	SDL_RenderTexture(m_renderer.get(), texture.GetSDLTexture(), &src, &dst);
}

SDL_Texture* dae::Renderer::CreateTextureFromSurface(SDL_Surface* surface) const
{
	return SDL_CreateTextureFromSurface(m_renderer.get(), surface);
}

void dae::Renderer::RenderRect(const Rect &rect, const Color &color) const
{
	RenderRect(rect.x, rect.y, rect.width, rect.height, color);
}

void dae::Renderer::RenderRect(float x, float y, float width, float height, const Color &color) const
{
	SDL_SetRenderDrawColor(m_renderer.get(), color.r, color.g, color.b, color.a);
	SDL_FRect r{x, y, width, height};
	SDL_RenderRect(m_renderer.get(), &r);
}

void dae::Renderer::RenderFilledRect(float x, float y, float width, float height, const Color &color) const
{
	SDL_SetRenderDrawColor(m_renderer.get(), color.r, color.g, color.b, color.a);
	SDL_FRect r{x, y, width, height};
	SDL_RenderFillRect(m_renderer.get(), &r);
}

void dae::Renderer::RenderCircle(float cx, float cy, float radius, const Color &color) const
{
	// Outline only, traced as a closed loop of short line segments (no SDL circle primitive)
	SDL_SetRenderDrawColor(m_renderer.get(), color.r, color.g, color.b, color.a);

	constexpr int segments = 40;
	std::array<SDL_FPoint, segments + 1> points{};
	for (int i = 0; i <= segments; ++i)
	{
		const float angle = (static_cast<float>(i) / segments) * 2.0f * 3.14159265f;
		points[i] = SDL_FPoint{ cx + std::cos(angle) * radius, cy + std::sin(angle) * radius };
	}
	SDL_RenderLines(m_renderer.get(), points.data(), static_cast<int>(points.size()));
}

void dae::Renderer::InitImGui() const
{
	ImGui_ImplSDL3_InitForSDLRenderer(m_window, m_renderer.get());
	ImGui_ImplSDLRenderer3_Init(m_renderer.get());
}

void dae::Renderer::ImGuiNewFrame() const
{
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
}

void dae::Renderer::RenderImGui() const
{
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_renderer.get());
}

void dae::Renderer::ShutDownImGui() const
{
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
}
