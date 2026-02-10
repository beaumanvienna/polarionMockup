/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include "resource.h"
#include "types.h"

#include <string>
#include <vector>

namespace PolarionMockup
{

    class WorkItem : public Resource
    {
    public:
        WorkItem() = default;
        WorkItem(const std::string& projectId, const std::string& id);

        // --- Attributes ---

        const std::string& GetTitle() const;
        void SetTitle(const std::string& title);

        const std::string& GetWorkItemType() const;
        void SetWorkItemType(const std::string& type);

        const std::string& GetStatus() const;
        void SetStatus(const std::string& status);

        const RichText& GetDescription() const;
        void SetDescription(const std::string& type, const std::string& value);

        const std::string& GetSeverity() const;
        void SetSeverity(const std::string& severity);

        const std::string& GetPriority() const;
        void SetPriority(const std::string& priority);

        const std::string& GetResolution() const;
        void SetResolution(const std::string& resolution);

        const std::string& GetCreated() const;
        void SetCreated(const std::string& created);

        const std::string& GetUpdated() const;
        void SetUpdated(const std::string& updated);

        const std::string& GetOutlineNumber() const;
        void SetOutlineNumber(const std::string& outlineNumber);

        // --- Relationships ---

        const std::string& GetProjectId() const;

        const std::string& GetAuthorId() const;
        void SetAuthorId(const std::string& authorId);

        const std::vector<std::string>& GetAssigneeIds() const;
        void AddAssignee(const std::string& userId);

        const std::vector<std::string>& GetCategoryIds() const;
        void AddCategory(const std::string& categoryId);

        const std::string& GetModuleId() const;
        void SetModuleId(const std::string& moduleId);

        const std::vector<std::string>& GetLinkedWorkItemIds() const;
        void AddLinkedWorkItem(const std::string& linkedId);
        bool RemoveLinkedWorkItem(const std::string& linkedId);

    private:
        // Attributes
        std::string m_Title;
        std::string m_WorkItemType; // "requirement", "task", etc.
        std::string m_Status;
        RichText m_Description;
        std::string m_Severity;
        std::string m_Priority;
        std::string m_Resolution;
        std::string m_Created;
        std::string m_Updated;
        std::string m_OutlineNumber;

        // Relationships (stored as IDs)
        std::string m_ProjectId;
        std::string m_AuthorId;
        std::string m_ModuleId;
        std::vector<std::string> m_AssigneeIds;
        std::vector<std::string> m_CategoryIds;
        std::vector<std::string> m_LinkedWorkItemIds;
    };

} // namespace PolarionMockup
