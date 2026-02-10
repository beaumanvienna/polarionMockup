/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include "project.h"
#include "workItem.h"
#include "document.h"
#include "user.h"
#include "comment.h"
#include "attachment.h"
#include "collection.h"

#include <crow.h>

#include <string>
#include <vector>

namespace PolarionMockup
{

    class JsonApiSerializer
    {
    public:
        // --- Single resource serialization ---

        static crow::json::wvalue SerializeProject(const Project& project);
        static crow::json::wvalue SerializeWorkItem(const WorkItem& workItem, const std::vector<std::string>& fields = {});
        static crow::json::wvalue SerializeDocument(const Document& document);
        static crow::json::wvalue SerializeUser(const User& user);
        static crow::json::wvalue SerializeComment(const Comment& comment);
        static crow::json::wvalue SerializeAttachment(const Attachment& attachment);
        static crow::json::wvalue SerializeCollection(const Collection& collection);
        static crow::json::wvalue SerializeLinkedWorkItem(const std::string& workItemId, const std::string& linkedId);

        // --- Collection response envelope ---

        static crow::json::wvalue WrapCollection(std::vector<crow::json::wvalue>&& data, int totalCount,
                                                 const std::string& selfLink = {}, int pageSize = 0, int pageNumber = 0,
                                                 int totalPages = 0);

        // --- Single resource response envelope ---

        static crow::json::wvalue WrapSingle(crow::json::wvalue&& data);

        // --- Error response ---

        static crow::json::wvalue Error(int status, const std::string& title, const std::string& detail = {});

        // --- Utility ---

        static std::vector<std::string> ParseFieldList(const char* fieldsParam);

    private:
        static bool HasField(const std::vector<std::string>& fields, const std::string& name);
    };

} // namespace PolarionMockup
