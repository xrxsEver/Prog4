#include "TypeRegistry.h"

namespace dae
{
    TypeRegistry::TypeRegistry()
    {
        // Hard-code the default Sno-Bee breeds (sprite block at the default row 9 / col 8)
        m_snoBeeTypes["Basic"] = SnoBeeType{200.0f, 100, 9, 8, AITier::Basic, false};
        m_snoBeeTypes["Fast"] = SnoBeeType{600.0f, 200, 9, 8, AITier::Intermediate, true};
        m_snoBeeTypes["Smart"] = SnoBeeType{450.0f, 300, 9, 8, AITier::Advanced, true};
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
