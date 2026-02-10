/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "attachment.h"

namespace PolarionMockup
{

    Attachment::Attachment(const std::string& workItemCompositeId, const std::string& id)
        : Resource("workitem_attachments", id), m_WorkItemId(workItemCompositeId)
    {
    }

    const std::string& Attachment::GetFileName() const { return m_FileName; }
    void Attachment::SetFileName(const std::string& fileName) { m_FileName = fileName; }

    const std::string& Attachment::GetTitle() const { return m_Title; }
    void Attachment::SetTitle(const std::string& title) { m_Title = title; }

    const std::string& Attachment::GetContentType() const { return m_ContentType; }
    void Attachment::SetContentType(const std::string& contentType) { m_ContentType = contentType; }

    size_t Attachment::GetLength() const { return m_Length; }
    void Attachment::SetLength(size_t length) { m_Length = length; }

    const std::string& Attachment::GetAuthorId() const { return m_AuthorId; }
    void Attachment::SetAuthorId(const std::string& authorId) { m_AuthorId = authorId; }

    const std::string& Attachment::GetCreated() const { return m_Created; }
    void Attachment::SetCreated(const std::string& created) { m_Created = created; }

    const std::string& Attachment::GetWorkItemId() const { return m_WorkItemId; }

} // namespace PolarionMockup
