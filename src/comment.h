/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include "resource.h"
#include "types.h"

#include <string>

namespace PolarionMockup
{

    class Comment : public Resource
    {
    public:

        Comment() = default;
        Comment(const std::string& workItemCompositeId, const std::string& id);

        const RichText& GetText() const;
        void SetText(const std::string& type, const std::string& value);

        const std::string& GetAuthorId() const;
        void SetAuthorId(const std::string& authorId);

        const std::string& GetCreated() const;
        void SetCreated(const std::string& created);

        const std::string& GetWorkItemId() const;

    private:

        RichText    m_Text;
        std::string m_AuthorId;
        std::string m_Created;
        std::string m_WorkItemId;
    };

} // namespace PolarionMockup
