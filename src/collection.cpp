/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "collection.h"

namespace PolarionMockup
{

    Collection::Collection(const std::string& projectId, const std::string& id, const std::string& name)
        : Resource("collections", projectId + "/" + id), m_Name(name), m_ProjectId(projectId)
    {
    }

    const std::string& Collection::GetName() const { return m_Name; }
    void Collection::SetName(const std::string& name) { m_Name = name; }

    const std::string& Collection::GetDescription() const { return m_Description; }
    void Collection::SetDescription(const std::string& description) { m_Description = description; }

    const std::string& Collection::GetProjectId() const { return m_ProjectId; }

    const std::vector<std::string>& Collection::GetDocumentIds() const { return m_DocumentIds; }
    void Collection::AddDocument(const std::string& documentId) { m_DocumentIds.push_back(documentId); }

} // namespace PolarionMockup
