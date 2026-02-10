/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include <string>
#include <vector>

namespace PolarionMockup
{

    struct RichText
    {
        std::string m_Type;  // "text/html" or "text/plain"
        std::string m_Value;
    };

    struct Hyperlink
    {
        std::string m_Role;
        std::string m_Title;
        std::string m_Uri;
    };

} // namespace PolarionMockup
