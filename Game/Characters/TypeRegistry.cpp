#include "TypeRegistry.h"

namespace dae
{
    TypeRegistry::TypeRegistry()
    {
        // Hard-code the default Sno-Bee breeds
        m_snoBeeTypes["Basic"] = SnoBeeType{400.0f, 100, 0, AITier::Basic, false};
        m_snoBeeTypes["Fast"] = SnoBeeType{600.0f, 200, 1, AITier::Intermediate, true};
        m_snoBeeTypes["Smart"] = SnoBeeType{450.0f, 300, 2, AITier::Advanced, true};
    }

    TypeRegistry& TypeRegistry::GetInstance()
    {
        static TypeRegistry instance;
        return instance;
    }

    [[nodiscard]] const SnoBeeType* TypeRegistry::GetSnoBeeType(const std::string& breedName) const
    {
        auto it = m_snoBeeTypes.find(breedName);
        if (it != m_snoBeeTypes.end())
        {
            return &it->second;
        }
        return nullptr;
    }
}
