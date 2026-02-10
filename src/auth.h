/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include <string>

namespace PolarionMockup
{

    class Auth
    {
    public:
        Auth();

        bool ValidateToken(const std::string& token) const;

        // Extract token from "Bearer <token>" header value
        static std::string ExtractBearerToken(const std::string& authHeader);

    private:
        std::string m_ValidToken;
    };

} // namespace PolarionMockup
