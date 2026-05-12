#pragma once

#include <string>
#include <unordered_map>
#include "SnoBeeType.h"

namespace dae
{
    class TypeRegistry
    {
    public:
        TypeRegistry();

        // Get the registry instance (singleton pattern for global access at startup)
        static TypeRegistry& GetInstance();

        [[nodiscard]] const SnoBeeType* GetSnoBeeType(const std::string& breedName) const;

    private:
        std::unordered_map<std::string, SnoBeeType> m_snoBeeTypes;
    };
}
