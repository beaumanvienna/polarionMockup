/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include <string>

namespace PolarionMockup
{

    class Resource
    {
    public:

        Resource() = default;
        Resource(const std::string& type, const std::string& id);
        virtual ~Resource() = default;

        const std::string& GetType() const;
        const std::string& GetId() const;
        const std::string& GetRevision() const;

        void SetRevision(const std::string& revision);

    protected:

        std::string m_Type;
        std::string m_Id;
        std::string m_Revision;
    };

} // namespace PolarionMockup
