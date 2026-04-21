#pragma once
#include <glm/glm.hpp>

namespace dae
{
	// Pure data container - no invariants, so using struct with public members (C.131)
	struct Transform final
	{
		glm::vec3 position{};
	};
}
