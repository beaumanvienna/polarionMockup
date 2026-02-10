/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include "resource.h"

#include <string>

namespace PolarionMockup
{

    class User : public Resource
    {
    public:

        User() = default;
        User(const std::string& id, const std::string& name);

        const std::string& GetName() const;
        void SetName(const std::string& name);

    private:

        std::string m_Name;
    };

} // namespace PolarionMockup
