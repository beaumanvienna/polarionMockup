/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "document.h"

namespace PolarionMockup
{

    Document::Document(const std::string& projectId,
                       const std::string& spaceId,
                       const std::string& id,
                       const std::string& title)
        : Resource("documents", projectId + "/" + spaceId + "/" + id),
          m_Title(title),
          m_ProjectId(projectId),
          m_SpaceId(spaceId)
    {
    }

    const std::string& Document::GetTitle() const
    {
        return m_Title;
    }

    void Document::SetTitle(const std::string& title)
    {
        m_Title = title;
    }

    const std::string& Document::GetProjectId() const
    {
        return m_ProjectId;
    }

    const std::string& Document::GetSpaceId() const
    {
        return m_SpaceId;
    }

    const std::vector<std::string>& Document::GetWorkItemIds() const
    {
        return m_WorkItemIds;
    }

    void Document::AddWorkItem(const std::string& workItemId)
    {
        m_WorkItemIds.push_back(workItemId);
    }

} // namespace PolarionMockup
