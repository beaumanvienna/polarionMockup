/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "user.h"

namespace PolarionMockup
{

    User::User(const std::string& id, const std::string& name)
        : Resource("users", id), m_Name(name)
    {
    }

    const std::string& User::GetName() const
    {
        return m_Name;
    }

    void User::SetName(const std::string& name)
    {
        m_Name = name;
    }

} // namespace PolarionMockup
