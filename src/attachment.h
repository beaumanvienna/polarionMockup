/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include "resource.h"

#include <string>

namespace PolarionMockup
{

    class Attachment : public Resource
    {
    public:

        Attachment() = default;
        Attachment(const std::string& workItemCompositeId, const std::string& id);

        const std::string& GetFileName() const;
        void SetFileName(const std::string& fileName);

        const std::string& GetTitle() const;
        void SetTitle(const std::string& title);

        const std::string& GetContentType() const;
        void SetContentType(const std::string& contentType);

        size_t GetLength() const;
        void SetLength(size_t length);

        const std::string& GetAuthorId() const;
        void SetAuthorId(const std::string& authorId);

        const std::string& GetCreated() const;
        void SetCreated(const std::string& created);

        const std::string& GetWorkItemId() const;

    private:

        std::string m_FileName;
        std::string m_Title;
        std::string m_ContentType;
        size_t      m_Length{0};
        std::string m_AuthorId;
        std::string m_Created;
        std::string m_WorkItemId;
    };

} // namespace PolarionMockup
