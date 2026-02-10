/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "workItem.h"

namespace PolarionMockup
{

    WorkItem::WorkItem(const std::string& projectId, const std::string& id)
        : Resource("workitems", projectId + "/" + id), m_Status("open"), m_ProjectId(projectId)
    {
    }

    // --- Attributes ---

    const std::string& WorkItem::GetTitle() const { return m_Title; }

    void WorkItem::SetTitle(const std::string& title) { m_Title = title; }

    const std::string& WorkItem::GetWorkItemType() const { return m_WorkItemType; }

    void WorkItem::SetWorkItemType(const std::string& type) { m_WorkItemType = type; }

    const std::string& WorkItem::GetStatus() const { return m_Status; }

    void WorkItem::SetStatus(const std::string& status) { m_Status = status; }

    const RichText& WorkItem::GetDescription() const { return m_Description; }

    void WorkItem::SetDescription(const std::string& type, const std::string& value)
    {
        m_Description.m_Type = type;
        m_Description.m_Value = value;
    }

    const std::string& WorkItem::GetSeverity() const { return m_Severity; }

    void WorkItem::SetSeverity(const std::string& severity) { m_Severity = severity; }

    const std::string& WorkItem::GetPriority() const { return m_Priority; }

    void WorkItem::SetPriority(const std::string& priority) { m_Priority = priority; }

    const std::string& WorkItem::GetResolution() const { return m_Resolution; }

    void WorkItem::SetResolution(const std::string& resolution) { m_Resolution = resolution; }

    const std::string& WorkItem::GetCreated() const { return m_Created; }

    void WorkItem::SetCreated(const std::string& created) { m_Created = created; }

    const std::string& WorkItem::GetUpdated() const { return m_Updated; }

    void WorkItem::SetUpdated(const std::string& updated) { m_Updated = updated; }

    const std::string& WorkItem::GetOutlineNumber() const { return m_OutlineNumber; }

    void WorkItem::SetOutlineNumber(const std::string& outlineNumber) { m_OutlineNumber = outlineNumber; }

    // --- Relationships ---

    const std::string& WorkItem::GetProjectId() const { return m_ProjectId; }

    const std::string& WorkItem::GetAuthorId() const { return m_AuthorId; }

    void WorkItem::SetAuthorId(const std::string& authorId) { m_AuthorId = authorId; }

    const std::vector<std::string>& WorkItem::GetAssigneeIds() const { return m_AssigneeIds; }

    void WorkItem::AddAssignee(const std::string& userId) { m_AssigneeIds.push_back(userId); }

    const std::vector<std::string>& WorkItem::GetCategoryIds() const { return m_CategoryIds; }

    void WorkItem::AddCategory(const std::string& categoryId) { m_CategoryIds.push_back(categoryId); }

    const std::string& WorkItem::GetModuleId() const { return m_ModuleId; }

    void WorkItem::SetModuleId(const std::string& moduleId) { m_ModuleId = moduleId; }

    const std::vector<std::string>& WorkItem::GetLinkedWorkItemIds() const { return m_LinkedWorkItemIds; }

    void WorkItem::AddLinkedWorkItem(const std::string& linkedId) { m_LinkedWorkItemIds.push_back(linkedId); }

    bool WorkItem::RemoveLinkedWorkItem(const std::string& linkedId)
    {
        for (auto it = m_LinkedWorkItemIds.begin(); it != m_LinkedWorkItemIds.end(); ++it)
        {
            if (*it == linkedId)
            {
                m_LinkedWorkItemIds.erase(it);
                return true;
            }
        }
        return false;
    }

} // namespace PolarionMockup
