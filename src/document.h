/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include "resource.h"

#include <string>
#include <vector>

namespace PolarionMockup
{

    class Document : public Resource
    {
    public:

        Document() = default;
        Document(const std::string& projectId,
                 const std::string& spaceId,
                 const std::string& id,
                 const std::string& title);

        const std::string& GetTitle() const;
        void SetTitle(const std::string& title);

        const std::string& GetProjectId() const;
        const std::string& GetSpaceId() const;

        const std::vector<std::string>& GetWorkItemIds() const;
        void AddWorkItem(const std::string& workItemId);

    private:

        std::string m_Title;
        std::string m_ProjectId;
        std::string m_SpaceId;
        std::vector<std::string> m_WorkItemIds;
    };

} // namespace PolarionMockup
