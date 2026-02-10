/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "auth.h"

namespace PolarionMockup
{

    static constexpr const char* HARDCODED_TOKEN = "1234!@#$";
    static constexpr const char* BEARER_PREFIX = "Bearer ";

    Auth::Auth() : m_ValidToken(HARDCODED_TOKEN) {}

    bool Auth::ValidateToken(const std::string& token) const { return token == m_ValidToken; }

    std::string Auth::ExtractBearerToken(const std::string& authHeader)
    {
        if (authHeader.size() > 7 && authHeader.substr(0, 7) == BEARER_PREFIX)
        {
            return authHeader.substr(7);
        }
        return {};
    }

} // namespace PolarionMockup
