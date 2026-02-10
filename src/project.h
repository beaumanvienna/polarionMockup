/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include "resource.h"

#include <string>

namespace PolarionMockup
{

    class Project : public Resource
    {
    public:

        Project() = default;
        Project(const std::string& id, const std::string& name);

        const std::string& GetName() const;
        void SetName(const std::string& name);

        const std::string& GetDescription() const;
        void SetDescription(const std::string& description);

    private:

        std::string m_Name;
        std::string m_Description;
    };

} // namespace PolarionMockup
