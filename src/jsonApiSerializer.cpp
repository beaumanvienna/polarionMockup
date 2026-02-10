/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "jsonApiSerializer.h"

namespace PolarionMockup
{

    // ================================================================
    // Single resource serialization
    // ================================================================

    crow::json::wvalue JsonApiSerializer::SerializeProject(const Project& project)
    {
        crow::json::wvalue item;
        item["type"] = project.GetType();
        item["id"] = project.GetId();
        item["attributes"]["name"] = project.GetName();
        item["attributes"]["description"] = project.GetDescription();
        return item;
    }

    crow::json::wvalue JsonApiSerializer::SerializeWorkItem(const WorkItem& workItem, const std::vector<std::string>& fields)
    {
        crow::json::wvalue item;
        item["type"] = "workitems";
        item["id"] = workItem.GetId();

        if (HasField(fields, "title"))
            item["attributes"]["title"] = workItem.GetTitle();
        if (HasField(fields, "type"))
            item["attributes"]["type"] = workItem.GetWorkItemType();
        if (HasField(fields, "status"))
            item["attributes"]["status"] = workItem.GetStatus();
        if (HasField(fields, "severity"))
            item["attributes"]["severity"] = workItem.GetSeverity();
        if (HasField(fields, "priority"))
            item["attributes"]["priority"] = workItem.GetPriority();
        if (HasField(fields, "resolution"))
            item["attributes"]["resolution"] = workItem.GetResolution();
        if (HasField(fields, "created"))
            item["attributes"]["created"] = workItem.GetCreated();
        if (HasField(fields, "updated"))
            item["attributes"]["updated"] = workItem.GetUpdated();
        if (HasField(fields, "outlineNumber"))
            item["attributes"]["outlineNumber"] = workItem.GetOutlineNumber();

        if (HasField(fields, "description"))
        {
            item["attributes"]["description"]["type"] = workItem.GetDescription().m_Type;
            item["attributes"]["description"]["value"] = workItem.GetDescription().m_Value;
        }

        // Relationships (always included)
        item["relationships"]["project"]["data"]["type"] = "projects";
        item["relationships"]["project"]["data"]["id"] = workItem.GetProjectId();

        if (!workItem.GetAuthorId().empty())
        {
            item["relationships"]["author"]["data"]["type"] = "users";
            item["relationships"]["author"]["data"]["id"] = workItem.GetAuthorId();
        }

        if (!workItem.GetAssigneeIds().empty())
        {
            std::vector<crow::json::wvalue> assignees;
            for (const auto& uid : workItem.GetAssigneeIds())
            {
                crow::json::wvalue ref;
                ref["type"] = "users";
                ref["id"] = uid;
                assignees.push_back(std::move(ref));
            }
            item["relationships"]["assignee"]["data"] = std::move(assignees);
        }

        if (!workItem.GetCategoryIds().empty())
        {
            std::vector<crow::json::wvalue> cats;
            for (const auto& cid : workItem.GetCategoryIds())
            {
                crow::json::wvalue ref;
                ref["type"] = "categories";
                ref["id"] = cid;
                cats.push_back(std::move(ref));
            }
            item["relationships"]["categories"]["data"] = std::move(cats);
        }

        if (!workItem.GetLinkedWorkItemIds().empty())
        {
            std::vector<crow::json::wvalue> links;
            for (const auto& lid : workItem.GetLinkedWorkItemIds())
            {
                crow::json::wvalue ref;
                ref["type"] = "workitems";
                ref["id"] = lid;
                links.push_back(std::move(ref));
            }
            item["relationships"]["linkedWorkItems"]["data"] = std::move(links);
        }

        if (!workItem.GetModuleId().empty())
        {
            item["relationships"]["module"]["data"]["type"] = "documents";
            item["relationships"]["module"]["data"]["id"] = workItem.GetModuleId();
        }

        return item;
    }

    crow::json::wvalue JsonApiSerializer::SerializeDocument(const Document& document)
    {
        crow::json::wvalue item;
        item["type"] = "documents";
        item["id"] = document.GetId();
        item["attributes"]["title"] = document.GetTitle();

        item["relationships"]["project"]["data"]["type"] = "projects";
        item["relationships"]["project"]["data"]["id"] = document.GetProjectId();

        if (!document.GetWorkItemIds().empty())
        {
            std::vector<crow::json::wvalue> wiRefs;
            for (const auto& wiId : document.GetWorkItemIds())
            {
                crow::json::wvalue ref;
                ref["type"] = "workitems";
                ref["id"] = wiId;
                wiRefs.push_back(std::move(ref));
            }
            item["relationships"]["workItems"]["data"] = std::move(wiRefs);
        }

        return item;
    }

    crow::json::wvalue JsonApiSerializer::SerializeUser(const User& user)
    {
        crow::json::wvalue item;
        item["type"] = "users";
        item["id"] = user.GetId();
        item["attributes"]["name"] = user.GetName();
        return item;
    }

    crow::json::wvalue JsonApiSerializer::SerializeComment(const Comment& comment)
    {
        crow::json::wvalue item;
        item["type"] = "workitem_comments";
        item["id"] = comment.GetId();
        item["attributes"]["text"]["type"] = comment.GetText().m_Type;
        item["attributes"]["text"]["value"] = comment.GetText().m_Value;
        item["attributes"]["created"] = comment.GetCreated();

        if (!comment.GetAuthorId().empty())
        {
            item["relationships"]["author"]["data"]["type"] = "users";
            item["relationships"]["author"]["data"]["id"] = comment.GetAuthorId();
        }

        item["relationships"]["workItem"]["data"]["type"] = "workitems";
        item["relationships"]["workItem"]["data"]["id"] = comment.GetWorkItemId();

        return item;
    }

    crow::json::wvalue JsonApiSerializer::SerializeAttachment(const Attachment& attachment)
    {
        crow::json::wvalue item;
        item["type"] = "workitem_attachments";
        item["id"] = attachment.GetId();
        item["attributes"]["fileName"] = attachment.GetFileName();
        item["attributes"]["title"] = attachment.GetTitle();
        item["attributes"]["contentType"] = attachment.GetContentType();
        item["attributes"]["length"] = attachment.GetLength();
        item["attributes"]["created"] = attachment.GetCreated();

        if (!attachment.GetAuthorId().empty())
        {
            item["relationships"]["author"]["data"]["type"] = "users";
            item["relationships"]["author"]["data"]["id"] = attachment.GetAuthorId();
        }

        item["relationships"]["workItem"]["data"]["type"] = "workitems";
        item["relationships"]["workItem"]["data"]["id"] = attachment.GetWorkItemId();

        return item;
    }

    crow::json::wvalue JsonApiSerializer::SerializeCollection(const Collection& collection)
    {
        crow::json::wvalue item;
        item["type"] = "collections";
        item["id"] = collection.GetId();
        item["attributes"]["name"] = collection.GetName();
        item["attributes"]["description"] = collection.GetDescription();

        item["relationships"]["project"]["data"]["type"] = "projects";
        item["relationships"]["project"]["data"]["id"] = collection.GetProjectId();

        if (!collection.GetDocumentIds().empty())
        {
            std::vector<crow::json::wvalue> docRefs;
            for (const auto& docId : collection.GetDocumentIds())
            {
                crow::json::wvalue ref;
                ref["type"] = "documents";
                ref["id"] = docId;
                docRefs.push_back(std::move(ref));
            }
            item["relationships"]["documents"]["data"] = std::move(docRefs);
        }

        return item;
    }

    crow::json::wvalue JsonApiSerializer::SerializeLinkedWorkItem(const std::string& workItemId, const std::string& linkedId)
    {
        crow::json::wvalue item;
        item["type"] = "linkedworkitems";
        item["id"] = workItemId + "/" + linkedId;
        item["relationships"]["workItem"]["data"]["type"] = "workitems";
        item["relationships"]["workItem"]["data"]["id"] = linkedId;
        return item;
    }

    // ================================================================
    // Response envelopes
    // ================================================================

    crow::json::wvalue JsonApiSerializer::WrapCollection(std::vector<crow::json::wvalue>&& data, int totalCount,
                                                         const std::string& selfLink, int pageSize, int pageNumber,
                                                         int totalPages)
    {
        crow::json::wvalue result;
        result["data"] = std::move(data);
        result["meta"]["totalCount"] = totalCount;

        if (!selfLink.empty() && pageSize > 0)
        {
            result["links"]["self"] =
                selfLink + "?page[size]=" + std::to_string(pageSize) + "&page[number]=" + std::to_string(pageNumber);
            result["links"]["first"] = selfLink + "?page[size]=" + std::to_string(pageSize) + "&page[number]=1";
            result["links"]["last"] =
                selfLink + "?page[size]=" + std::to_string(pageSize) + "&page[number]=" + std::to_string(totalPages);

            if (pageNumber > 1)
            {
                result["links"]["prev"] =
                    selfLink + "?page[size]=" + std::to_string(pageSize) + "&page[number]=" + std::to_string(pageNumber - 1);
            }

            if (pageNumber < totalPages)
            {
                result["links"]["next"] =
                    selfLink + "?page[size]=" + std::to_string(pageSize) + "&page[number]=" + std::to_string(pageNumber + 1);
            }
        }

        return result;
    }

    crow::json::wvalue JsonApiSerializer::WrapSingle(crow::json::wvalue&& data)
    {
        crow::json::wvalue result;
        result["data"] = std::move(data);
        return result;
    }

    crow::json::wvalue JsonApiSerializer::Error(int status, const std::string& title, const std::string& detail)
    {
        crow::json::wvalue err;
        err["errors"][0]["status"] = std::to_string(status);
        err["errors"][0]["title"] = title;
        if (!detail.empty())
        {
            err["errors"][0]["detail"] = detail;
        }
        return err;
    }

    // ================================================================
    // Utility
    // ================================================================

    std::vector<std::string> JsonApiSerializer::ParseFieldList(const char* fieldsParam)
    {
        std::vector<std::string> result;
        if (!fieldsParam)
        {
            return result;
        }

        std::string fieldsStr = fieldsParam;
        size_t pos = 0;
        while (pos < fieldsStr.size())
        {
            size_t comma = fieldsStr.find(',', pos);
            if (comma == std::string::npos)
            {
                result.push_back(fieldsStr.substr(pos));
                break;
            }
            result.push_back(fieldsStr.substr(pos, comma - pos));
            pos = comma + 1;
        }
        return result;
    }

    bool JsonApiSerializer::HasField(const std::vector<std::string>& fields, const std::string& name)
    {
        if (fields.empty())
            return true;
        for (const auto& f : fields)
        {
            if (f == name)
                return true;
        }
        return false;
    }

} // namespace PolarionMockup
