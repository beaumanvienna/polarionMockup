/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "comment.h"

namespace PolarionMockup
{

    Comment::Comment(const std::string& workItemCompositeId, const std::string& id)
        : Resource("workitem_comments", id), m_WorkItemId(workItemCompositeId)
    {
    }

    const RichText& Comment::GetText() const { return m_Text; }

    void Comment::SetText(const std::string& type, const std::string& value)
    {
        m_Text.m_Type = type;
        m_Text.m_Value = value;
    }

    const std::string& Comment::GetAuthorId() const { return m_AuthorId; }
    void Comment::SetAuthorId(const std::string& authorId) { m_AuthorId = authorId; }

    const std::string& Comment::GetCreated() const { return m_Created; }
    void Comment::SetCreated(const std::string& created) { m_Created = created; }

    const std::string& Comment::GetWorkItemId() const { return m_WorkItemId; }

} // namespace PolarionMockup
