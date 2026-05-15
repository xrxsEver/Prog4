#pragma once
#include <glm/glm.hpp>

namespace dae
{
	// Pure data container - no invariants, so using struct with public members (C.131)
	struct Transform final
	{
		glm::vec3 position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation{ 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
	};
}
