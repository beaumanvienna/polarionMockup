/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "resource.h"

namespace PolarionMockup
{

    Resource::Resource(const std::string& type, const std::string& id)
        : m_Type(type), m_Id(id), m_Revision("1")
    {
    }

    const std::string& Resource::GetType() const
    {
        return m_Type;
    }

    const std::string& Resource::GetId() const
    {
        return m_Id;
    }

    const std::string& Resource::GetRevision() const
    {
        return m_Revision;
    }

    void Resource::SetRevision(const std::string& revision)
    {
        m_Revision = revision;
    }

} // namespace PolarionMockup
