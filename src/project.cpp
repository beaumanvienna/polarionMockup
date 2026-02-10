/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "project.h"

namespace PolarionMockup
{

    Project::Project(const std::string& id, const std::string& name)
        : Resource("projects", id), m_Name(name)
    {
    }

    const std::string& Project::GetName() const
    {
        return m_Name;
    }

    void Project::SetName(const std::string& name)
    {
        m_Name = name;
    }

    const std::string& Project::GetDescription() const
    {
        return m_Description;
    }

    void Project::SetDescription(const std::string& description)
    {
        m_Description = description;
    }

} // namespace PolarionMockup
