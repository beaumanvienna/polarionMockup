/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include "resource.h"

#include <string>
#include <vector>

namespace PolarionMockup
{

    class Collection : public Resource
    {
    public:

        Collection() = default;
        Collection(const std::string& projectId, const std::string& id, const std::string& name);

        const std::string& GetName() const;
        void SetName(const std::string& name);

        const std::string& GetDescription() const;
        void SetDescription(const std::string& description);

        const std::string& GetProjectId() const;

        const std::vector<std::string>& GetDocumentIds() const;
        void AddDocument(const std::string& documentId);

    private:

        std::string m_Name;
        std::string m_Description;
        std::string m_ProjectId;
        std::vector<std::string> m_DocumentIds;
    };

} // namespace PolarionMockup
