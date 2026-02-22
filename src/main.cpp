/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "auth.h"
#include "dataStore.h"
#include "jsonApiSerializer.h"

#include <crow.h>
#include <spdlog/spdlog.h>

#include <cstring>
#include <iostream>

using Ser = PolarionMockup::JsonApiSerializer;

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0)
        {
            std::cout << "PolarionMockup v" << MOCKUP_VERSION << "\n"
                      << "Lightweight REST API server emulating Siemens Polarion ALM.\n\n"
                      << "Usage: mockup [options]\n\n"
                      << "Options:\n"
                      << "  -h, --help       Show this help message and exit\n"
                      << "  -v, --version    Show version and exit\n\n"
                      << "The server listens on http://0.0.0.0:18080 and requires\n"
                      << "Bearer token authentication (Authorization: Bearer <token>).\n";
            return 0;
        }
        if (std::strcmp(argv[i], "--version") == 0 || std::strcmp(argv[i], "-v") == 0)
        {
            std::cout << "PolarionMockup v" << MOCKUP_VERSION << "\n";
            return 0;
        }
    }

    spdlog::set_level(spdlog::level::info);
    spdlog::info("PolarionMockup v{} starting", MOCKUP_VERSION);

    PolarionMockup::DataStore dataStore;
    spdlog::info("DataStore seeded: {} projects, {} work items, {} users", dataStore.GetAllProjects().size(),
                 dataStore.GetAllWorkItems().size(), dataStore.GetAllUsers().size());

    PolarionMockup::Auth auth;

    crow::SimpleApp app;
    app.loglevel(crow::LogLevel::Warning);

    // --- Auth middleware (applied manually per route) ---
    auto Authenticate = [&auth](const crow::request& req) -> bool
    {
        auto it = req.headers.find("Authorization");
        if (it == req.headers.end())
        {
            spdlog::warn("Auth failed: missing Authorization header [{}]", req.url);
            return false;
        }
        std::string token = PolarionMockup::Auth::ExtractBearerToken(it->second);
        if (!auth.ValidateToken(token))
        {
            spdlog::warn("Auth failed: invalid token [{}]", req.url);
            return false;
        }
        return true;
    };

    // =============================================================
    // Health check (no auth required)
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/health")
    (
        []()
        {
            spdlog::info("GET /health");
            crow::json::wvalue r;
            r["status"] = "ok";
            return crow::response(200, r);
        });

    // =============================================================
    // GET /projects
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects")
    (
        [&Authenticate, &dataStore](const crow::request& req)
        {
            if (!Authenticate(req))
                return crow::response(401, "Unauthorized");

            spdlog::info("GET /projects");
            auto projects = dataStore.GetAllProjects();
            spdlog::info("  returning {} projects", projects.size());

            std::vector<crow::json::wvalue> arr;
            for (const auto* p : projects)
                arr.push_back(Ser::SerializeProject(*p));

            auto result = Ser::WrapCollection(std::move(arr), static_cast<int>(projects.size()));
            return crow::response(200, result);
        });

    // =============================================================
    // GET /projects/<id>
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>")
    (
        [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId)
        {
            if (!Authenticate(req))
                return crow::response(401, "Unauthorized");

            spdlog::info("GET /projects/{}", projectId);
            const auto* proj = dataStore.GetProject(projectId);
            if (!proj)
            {
                spdlog::warn("  project '{}' not found", projectId);
                return crow::response(404, Ser::Error(404, "Project not found"));
            }
            auto result = Ser::WrapSingle(Ser::SerializeProject(*proj));
            return crow::response(200, result);
        });

    // =============================================================
    // GET /projects/<id>/workitems  (list + query + pagination)
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems")
        .methods(crow::HTTPMethod::GET)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("GET /projects/{}/workitems", projectId);
                if (!dataStore.GetProject(projectId))
                {
                    spdlog::warn("  project '{}' not found", projectId);
                    return crow::response(404, Ser::Error(404, "Project not found"));
                }

                // Query
                std::string query;
                auto qp = req.url_params.get("query");
                if (qp)
                {
                    query = qp;
                    spdlog::info("  query=\"{}\"", query);
                }

                // Pagination
                int pageSize = 100, pageNumber = 1;
                auto ps = req.url_params.get("page[size]");
                if (ps)
                    pageSize = std::max(1, std::atoi(ps));
                auto pn = req.url_params.get("page[number]");
                if (pn)
                    pageNumber = std::max(1, std::atoi(pn));

                // Sparse fieldsets
                auto fields = Ser::ParseFieldList(req.url_params.get("fields[workitems]"));

                // Execute
                auto allItems = dataStore.QueryWorkItems(projectId, query);
                int totalCount = static_cast<int>(allItems.size());
                spdlog::info("  matched {} work items, page {}, pageSize {}", totalCount, pageNumber, pageSize);

                int startIdx = (pageNumber - 1) * pageSize;
                int endIdx = std::min(startIdx + pageSize, totalCount);

                std::vector<crow::json::wvalue> arr;
                for (int i = startIdx; i < endIdx; ++i)
                    arr.push_back(Ser::SerializeWorkItem(*allItems[i], fields));

                int totalPages = std::max(1, (totalCount + pageSize - 1) / pageSize);
                std::string basePath = "/polarion/rest/v1/projects/" + projectId + "/workitems";
                auto result = Ser::WrapCollection(std::move(arr), totalCount, basePath, pageSize, pageNumber, totalPages);
                return crow::response(200, result);
            });

    // =============================================================
    // POST /projects/<id>/workitems  (create single or bulk)
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems")
        .methods(crow::HTTPMethod::POST)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("POST /projects/{}/workitems", projectId);
                if (!dataStore.GetProject(projectId))
                    return crow::response(404, Ser::Error(404, "Project not found"));

                auto body = crow::json::load(req.body);
                if (!body || !body.has("data"))
                    return crow::response(400, Ser::Error(400, "Invalid request body", "Missing 'data' key"));

                // Support single object or array in data
                std::vector<crow::json::rvalue> items;
                if (body["data"].t() == crow::json::type::List)
                {
                    for (size_t i = 0; i < body["data"].size(); ++i)
                        items.push_back(body["data"][i]);
                }
                else
                {
                    items.push_back(body["data"]);
                }

                std::vector<crow::json::wvalue> created;
                for (const auto& d : items)
                {
                    if (!d.has("attributes") || !d["attributes"].has("id"))
                    {
                        spdlog::warn("  skipping item: missing attributes.id");
                        continue;
                    }

                    std::string wiId = d["attributes"]["id"].s();
                    PolarionMockup::WorkItem wi(projectId, wiId);

                    auto& attrs = d["attributes"];
                    if (attrs.has("title"))
                        wi.SetTitle(attrs["title"].s());
                    if (attrs.has("type"))
                        wi.SetWorkItemType(attrs["type"].s());
                    if (attrs.has("status"))
                        wi.SetStatus(attrs["status"].s());
                    if (attrs.has("severity"))
                        wi.SetSeverity(attrs["severity"].s());
                    if (attrs.has("priority"))
                        wi.SetPriority(attrs["priority"].s());
                    if (attrs.has("resolution"))
                        wi.SetResolution(attrs["resolution"].s());
                    if (attrs.has("created"))
                        wi.SetCreated(attrs["created"].s());
                    if (attrs.has("updated"))
                        wi.SetUpdated(attrs["updated"].s());
                    if (attrs.has("outlineNumber"))
                        wi.SetOutlineNumber(attrs["outlineNumber"].s());
                    if (attrs.has("description") && attrs["description"].has("type") && attrs["description"].has("value"))
                        wi.SetDescription(attrs["description"]["type"].s(), attrs["description"]["value"].s());

                    auto* result = dataStore.CreateWorkItem(projectId, std::move(wi));
                    if (result)
                    {
                        spdlog::info("  created work item '{}'", result->GetId());
                        created.push_back(Ser::SerializeWorkItem(*result));
                    }
                    else
                    {
                        spdlog::warn("  work item '{}' already exists", projectId + "/" + wiId);
                    }
                }

                if (created.size() == 1)
                {
                    auto result = Ser::WrapSingle(std::move(created[0]));
                    return crow::response(201, result);
                }
                auto result = Ser::WrapCollection(std::move(created), static_cast<int>(created.size()));
                return crow::response(201, result);
            });

    // =============================================================
    // GET /projects/<id>/workitems/<wiId>  (single get)
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>")
        .methods(crow::HTTPMethod::GET)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("GET /projects/{}/workitems/{}", projectId, workItemId);
                if (!dataStore.GetProject(projectId))
                    return crow::response(404, Ser::Error(404, "Project not found"));

                std::string compositeId = projectId + "/" + workItemId;
                const auto* wi = dataStore.GetWorkItem(compositeId);
                if (!wi)
                {
                    spdlog::warn("  work item '{}' not found", compositeId);
                    return crow::response(404, Ser::Error(404, "Work item not found"));
                }

                auto result = Ser::WrapSingle(Ser::SerializeWorkItem(*wi));
                return crow::response(200, result);
            });

    // =============================================================
    // PATCH /projects/<id>/workitems/<wiId>  (update single)
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>")
        .methods(crow::HTTPMethod::PATCH)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("PATCH /projects/{}/workitems/{}", projectId, workItemId);
                if (!dataStore.GetProject(projectId))
                    return crow::response(404, Ser::Error(404, "Project not found"));

                std::string compositeId = projectId + "/" + workItemId;
                auto* wi = dataStore.UpdateWorkItem(compositeId);
                if (!wi)
                {
                    spdlog::warn("  work item '{}' not found", compositeId);
                    return crow::response(404, Ser::Error(404, "Work item not found"));
                }

                auto body = crow::json::load(req.body);
                if (!body || !body.has("data") || !body["data"].has("attributes"))
                    return crow::response(400, Ser::Error(400, "Invalid request body"));

                auto& attrs = body["data"]["attributes"];
                if (attrs.has("title"))
                    wi->SetTitle(attrs["title"].s());
                if (attrs.has("type"))
                    wi->SetWorkItemType(attrs["type"].s());
                if (attrs.has("status"))
                    wi->SetStatus(attrs["status"].s());
                if (attrs.has("severity"))
                    wi->SetSeverity(attrs["severity"].s());
                if (attrs.has("priority"))
                    wi->SetPriority(attrs["priority"].s());
                if (attrs.has("resolution"))
                    wi->SetResolution(attrs["resolution"].s());
                if (attrs.has("outlineNumber"))
                    wi->SetOutlineNumber(attrs["outlineNumber"].s());
                if (attrs.has("description") && attrs["description"].has("type") && attrs["description"].has("value"))
                    wi->SetDescription(attrs["description"]["type"].s(), attrs["description"]["value"].s());

                spdlog::info("  updated work item '{}'", compositeId);
                auto result = Ser::WrapSingle(Ser::SerializeWorkItem(*wi));
                return crow::response(200, result);
            });

    // =============================================================
    // DELETE /projects/<id>/workitems/<wiId>  (delete single)
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>")
        .methods(crow::HTTPMethod::DELETE)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("DELETE /projects/{}/workitems/{}", projectId, workItemId);
                std::string compositeId = projectId + "/" + workItemId;
                if (dataStore.DeleteWorkItem(compositeId))
                {
                    spdlog::info("  deleted work item '{}'", compositeId);
                    return crow::response(204);
                }
                spdlog::warn("  work item '{}' not found", compositeId);
                return crow::response(404, Ser::Error(404, "Work item not found"));
            });

    // =============================================================
    // GET /projects/<id>/spaces/<spaceId>/documents
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/spaces/<string>/documents")
    (
        [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId, const std::string& spaceId)
        {
            if (!Authenticate(req))
                return crow::response(401, "Unauthorized");

            spdlog::info("GET /projects/{}/spaces/{}/documents", projectId, spaceId);
            if (!dataStore.GetProject(projectId))
                return crow::response(404, Ser::Error(404, "Project not found"));

            auto allDocs = dataStore.GetDocumentsByProject(projectId);
            std::vector<crow::json::wvalue> arr;
            for (const auto* doc : allDocs)
            {
                if (doc->GetSpaceId() == spaceId)
                    arr.push_back(Ser::SerializeDocument(*doc));
            }
            spdlog::info("  returning {} documents in space '{}'", arr.size(), spaceId);
            auto result = Ser::WrapCollection(std::move(arr), static_cast<int>(arr.size()));
            return crow::response(200, result);
        });

    // =============================================================
    // PATCH /projects/<id>/workitems  (bulk update)
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems")
        .methods(crow::HTTPMethod::PATCH)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("PATCH /projects/{}/workitems (bulk)", projectId);
                if (!dataStore.GetProject(projectId))
                    return crow::response(404, Ser::Error(404, "Project not found"));

                auto body = crow::json::load(req.body);
                if (!body || !body.has("data") || body["data"].t() != crow::json::type::List)
                    return crow::response(400, Ser::Error(400, "Invalid request body", "Expected data array"));

                std::vector<crow::json::wvalue> updated;
                for (size_t i = 0; i < body["data"].size(); ++i)
                {
                    auto& d = body["data"][i];
                    if (!d.has("id") || !d.has("attributes"))
                        continue;

                    std::string compositeId = projectId + "/" + std::string(d["id"].s());
                    auto* wi = dataStore.UpdateWorkItem(compositeId);
                    if (!wi)
                    {
                        spdlog::warn("  bulk update: '{}' not found", compositeId);
                        continue;
                    }

                    auto& attrs = d["attributes"];
                    if (attrs.has("title"))
                        wi->SetTitle(attrs["title"].s());
                    if (attrs.has("type"))
                        wi->SetWorkItemType(attrs["type"].s());
                    if (attrs.has("status"))
                        wi->SetStatus(attrs["status"].s());
                    if (attrs.has("severity"))
                        wi->SetSeverity(attrs["severity"].s());
                    if (attrs.has("priority"))
                        wi->SetPriority(attrs["priority"].s());
                    if (attrs.has("resolution"))
                        wi->SetResolution(attrs["resolution"].s());
                    if (attrs.has("outlineNumber"))
                        wi->SetOutlineNumber(attrs["outlineNumber"].s());
                    if (attrs.has("description") && attrs["description"].has("type") && attrs["description"].has("value"))
                        wi->SetDescription(attrs["description"]["type"].s(), attrs["description"]["value"].s());

                    spdlog::info("  updated '{}'", compositeId);
                    updated.push_back(Ser::SerializeWorkItem(*wi));
                }

                auto result = Ser::WrapCollection(std::move(updated), static_cast<int>(updated.size()));
                return crow::response(200, result);
            });

    // =============================================================
    // DELETE /projects/<id>/workitems  (bulk delete)
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems")
        .methods(crow::HTTPMethod::DELETE)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("DELETE /projects/{}/workitems (bulk)", projectId);

                auto body = crow::json::load(req.body);
                if (!body || !body.has("data") || body["data"].t() != crow::json::type::List)
                    return crow::response(400, Ser::Error(400, "Invalid request body", "Expected data array"));

                int deleted = 0;
                for (size_t i = 0; i < body["data"].size(); ++i)
                {
                    auto& d = body["data"][i];
                    if (!d.has("id"))
                        continue;

                    std::string compositeId = projectId + "/" + std::string(d["id"].s());
                    if (dataStore.DeleteWorkItem(compositeId))
                    {
                        spdlog::info("  deleted '{}'", compositeId);
                        ++deleted;
                    }
                    else
                    {
                        spdlog::warn("  '{}' not found", compositeId);
                    }
                }

                return crow::response(204);
            });

    // =============================================================
    // GET /projects/<id>/workitems/<wiId>/linkedworkitems
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>/linkedworkitems")
        .methods(crow::HTTPMethod::GET)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("GET /projects/{}/workitems/{}/linkedworkitems", projectId, workItemId);
                std::string compositeId = projectId + "/" + workItemId;
                const auto* wi = dataStore.GetWorkItem(compositeId);
                if (!wi)
                    return crow::response(404, Ser::Error(404, "Work item not found"));

                std::vector<crow::json::wvalue> arr;
                for (const auto& lid : wi->GetLinkedWorkItemIds())
                    arr.push_back(Ser::SerializeLinkedWorkItem(compositeId, lid));

                auto result = Ser::WrapCollection(std::move(arr), static_cast<int>(arr.size()));
                return crow::response(200, result);
            });

    // =============================================================
    // POST /projects/<id>/workitems/<wiId>/linkedworkitems
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>/linkedworkitems")
        .methods(crow::HTTPMethod::POST)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("POST /projects/{}/workitems/{}/linkedworkitems", projectId, workItemId);
                std::string compositeId = projectId + "/" + workItemId;
                auto* wi = dataStore.UpdateWorkItem(compositeId);
                if (!wi)
                    return crow::response(404, Ser::Error(404, "Work item not found"));

                auto body = crow::json::load(req.body);
                if (!body || !body.has("data"))
                    return crow::response(400, Ser::Error(400, "Invalid request body"));

                auto& data = body["data"];
                if (data.has("id"))
                {
                    std::string linkedId = data["id"].s();
                    wi->AddLinkedWorkItem(linkedId);
                    spdlog::info("  linked '{}'", linkedId);
                    auto result = Ser::WrapSingle(Ser::SerializeLinkedWorkItem(compositeId, linkedId));
                    return crow::response(201, result);
                }

                return crow::response(400, Ser::Error(400, "Missing data.id"));
            });

    // =============================================================
    // DELETE /projects/<id>/workitems/<wiId>/linkedworkitems/<linkedProjId>/<linkedWiId>
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>/linkedworkitems/<string>/<string>")
        .methods(crow::HTTPMethod::DELETE)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId, const std::string& linkedProjId,
                                        const std::string& linkedWiId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                std::string linkedCompositeId = linkedProjId + "/" + linkedWiId;
                spdlog::info("DELETE /projects/{}/workitems/{}/linkedworkitems/{}", projectId, workItemId,
                             linkedCompositeId);
                std::string compositeId = projectId + "/" + workItemId;
                auto* wi = dataStore.UpdateWorkItem(compositeId);
                if (!wi)
                    return crow::response(404, Ser::Error(404, "Work item not found"));

                if (wi->RemoveLinkedWorkItem(linkedCompositeId))
                    return crow::response(204);
                return crow::response(404, Ser::Error(404, "Link not found"));
            });

    // =============================================================
    // GET  /projects/<id>/workitems/<wiId>/comments
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>/comments")
        .methods(crow::HTTPMethod::GET)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                std::string compositeId = projectId + "/" + workItemId;
                spdlog::info("GET .../workitems/{}/comments", workItemId);
                if (!dataStore.GetWorkItem(compositeId))
                    return crow::response(404, Ser::Error(404, "Work item not found"));

                auto comments = dataStore.GetCommentsByWorkItem(compositeId);
                std::vector<crow::json::wvalue> arr;
                for (const auto* c : comments)
                    arr.push_back(Ser::SerializeComment(*c));

                auto result = Ser::WrapCollection(std::move(arr), static_cast<int>(arr.size()));
                return crow::response(200, result);
            });

    // =============================================================
    // POST /projects/<id>/workitems/<wiId>/comments
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>/comments")
        .methods(crow::HTTPMethod::POST)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                std::string compositeId = projectId + "/" + workItemId;
                spdlog::info("POST .../workitems/{}/comments", workItemId);
                if (!dataStore.GetWorkItem(compositeId))
                    return crow::response(404, Ser::Error(404, "Work item not found"));

                auto body = crow::json::load(req.body);
                if (!body || !body.has("data") || !body["data"].has("attributes"))
                    return crow::response(400, Ser::Error(400, "Invalid request body"));

                std::string cId = "comment-" + std::to_string(dataStore.NextCommentId());
                PolarionMockup::Comment comment(compositeId, cId);

                auto& attrs = body["data"]["attributes"];
                if (attrs.has("text") && attrs["text"].has("type") && attrs["text"].has("value"))
                    comment.SetText(attrs["text"]["type"].s(), attrs["text"]["value"].s());
                if (attrs.has("author"))
                    comment.SetAuthorId(attrs["author"].s());

                auto* created = dataStore.AddComment(compositeId, std::move(comment));
                if (!created)
                    return crow::response(500, Ser::Error(500, "Failed to create comment"));

                spdlog::info("  created comment '{}'", created->GetId());
                auto result = Ser::WrapSingle(Ser::SerializeComment(*created));
                return crow::response(201, result);
            });

    // =============================================================
    // PATCH /projects/<id>/workitems/<wiId>/comments/<commentId>
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>/comments/<string>")
        .methods(crow::HTTPMethod::PATCH)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId, const std::string& commentId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("PATCH .../comments/{}", commentId);
                auto* comment = dataStore.GetMutableComment(commentId);
                if (!comment)
                    return crow::response(404, Ser::Error(404, "Comment not found"));

                auto body = crow::json::load(req.body);
                if (!body || !body.has("data") || !body["data"].has("attributes"))
                    return crow::response(400, Ser::Error(400, "Invalid request body"));

                auto& attrs = body["data"]["attributes"];
                if (attrs.has("text") && attrs["text"].has("type") && attrs["text"].has("value"))
                    comment->SetText(attrs["text"]["type"].s(), attrs["text"]["value"].s());

                auto result = Ser::WrapSingle(Ser::SerializeComment(*comment));
                return crow::response(200, result);
            });

    // =============================================================
    // DELETE /projects/<id>/workitems/<wiId>/comments/<commentId>
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>/comments/<string>")
        .methods(crow::HTTPMethod::DELETE)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId, const std::string& commentId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("DELETE .../comments/{}", commentId);
                if (dataStore.DeleteComment(commentId))
                    return crow::response(204);
                return crow::response(404, Ser::Error(404, "Comment not found"));
            });

    // =============================================================
    // GET  /projects/<id>/workitems/<wiId>/attachments
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>/attachments")
        .methods(crow::HTTPMethod::GET)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                std::string compositeId = projectId + "/" + workItemId;
                spdlog::info("GET .../workitems/{}/attachments", workItemId);
                if (!dataStore.GetWorkItem(compositeId))
                    return crow::response(404, Ser::Error(404, "Work item not found"));

                auto attachments = dataStore.GetAttachmentsByWorkItem(compositeId);
                std::vector<crow::json::wvalue> arr;
                for (const auto* a : attachments)
                    arr.push_back(Ser::SerializeAttachment(*a));

                auto result = Ser::WrapCollection(std::move(arr), static_cast<int>(arr.size()));
                return crow::response(200, result);
            });

    // =============================================================
    // POST /projects/<id>/workitems/<wiId>/attachments
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>/attachments")
        .methods(crow::HTTPMethod::POST)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                std::string compositeId = projectId + "/" + workItemId;
                spdlog::info("POST .../workitems/{}/attachments", workItemId);
                if (!dataStore.GetWorkItem(compositeId))
                    return crow::response(404, Ser::Error(404, "Work item not found"));

                auto body = crow::json::load(req.body);
                if (!body || !body.has("data") || !body["data"].has("attributes"))
                    return crow::response(400, Ser::Error(400, "Invalid request body"));

                std::string aId = "attachment-" + std::to_string(dataStore.NextAttachmentId());
                PolarionMockup::Attachment attachment(compositeId, aId);

                auto& attrs = body["data"]["attributes"];
                if (attrs.has("fileName"))
                    attachment.SetFileName(attrs["fileName"].s());
                if (attrs.has("title"))
                    attachment.SetTitle(attrs["title"].s());
                if (attrs.has("contentType"))
                    attachment.SetContentType(attrs["contentType"].s());
                if (attrs.has("length"))
                    attachment.SetLength(static_cast<size_t>(attrs["length"].i()));
                if (attrs.has("author"))
                    attachment.SetAuthorId(attrs["author"].s());

                auto* created = dataStore.AddAttachment(compositeId, std::move(attachment));
                if (!created)
                    return crow::response(500, Ser::Error(500, "Failed to create attachment"));

                spdlog::info("  created attachment '{}'", created->GetId());
                auto result = Ser::WrapSingle(Ser::SerializeAttachment(*created));
                return crow::response(201, result);
            });

    // =============================================================
    // DELETE /projects/<id>/workitems/<wiId>/attachments/<attachmentId>
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/workitems/<string>/attachments/<string>")
        .methods(crow::HTTPMethod::DELETE)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& workItemId, const std::string& attachmentId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("DELETE .../attachments/{}", attachmentId);
                if (dataStore.DeleteAttachment(attachmentId))
                    return crow::response(204);
                return crow::response(404, Ser::Error(404, "Attachment not found"));
            });

    // =============================================================
    // GET /projects/<id>/collections
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/collections")
        .methods(crow::HTTPMethod::GET)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("GET /projects/{}/collections", projectId);
                if (!dataStore.GetProject(projectId))
                    return crow::response(404, Ser::Error(404, "Project not found"));

                auto collections = dataStore.GetCollectionsByProject(projectId);
                std::vector<crow::json::wvalue> arr;
                for (const auto* c : collections)
                    arr.push_back(Ser::SerializeCollection(*c));

                auto result = Ser::WrapCollection(std::move(arr), static_cast<int>(arr.size()));
                return crow::response(200, result);
            });

    // =============================================================
    // POST /projects/<id>/collections
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/collections")
        .methods(crow::HTTPMethod::POST)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("POST /projects/{}/collections", projectId);
                if (!dataStore.GetProject(projectId))
                    return crow::response(404, Ser::Error(404, "Project not found"));

                auto body = crow::json::load(req.body);
                if (!body || !body.has("data") || !body["data"].has("attributes"))
                    return crow::response(400, Ser::Error(400, "Invalid request body"));

                auto& attrs = body["data"]["attributes"];
                if (!attrs.has("id") || !attrs.has("name"))
                    return crow::response(400, Ser::Error(400, "Missing id or name"));

                std::string colId = attrs["id"].s();
                PolarionMockup::Collection col(projectId, colId, attrs["name"].s());
                if (attrs.has("description"))
                    col.SetDescription(attrs["description"].s());

                auto* created = dataStore.CreateCollection(projectId, std::move(col));
                if (!created)
                    return crow::response(409, Ser::Error(409, "Collection already exists"));

                spdlog::info("  created collection '{}'", created->GetId());
                auto result = Ser::WrapSingle(Ser::SerializeCollection(*created));
                return crow::response(201, result);
            });

    // =============================================================
    // PATCH /projects/<id>/collections/<colId>
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/collections/<string>")
        .methods(crow::HTTPMethod::PATCH)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& collectionId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("PATCH /projects/{}/collections/{}", projectId, collectionId);
                std::string compositeId = projectId + "/" + collectionId;
                auto* col = dataStore.GetMutableCollection(compositeId);
                if (!col)
                    return crow::response(404, Ser::Error(404, "Collection not found"));

                auto body = crow::json::load(req.body);
                if (!body || !body.has("data") || !body["data"].has("attributes"))
                    return crow::response(400, Ser::Error(400, "Invalid request body"));

                auto& attrs = body["data"]["attributes"];
                if (attrs.has("name"))
                    col->SetName(attrs["name"].s());
                if (attrs.has("description"))
                    col->SetDescription(attrs["description"].s());

                auto result = Ser::WrapSingle(Ser::SerializeCollection(*col));
                return crow::response(200, result);
            });

    // =============================================================
    // DELETE /projects/<id>/collections/<colId>
    // =============================================================
    CROW_ROUTE(app, "/polarion/rest/v1/projects/<string>/collections/<string>")
        .methods(crow::HTTPMethod::DELETE)(
            [&Authenticate, &dataStore](const crow::request& req, const std::string& projectId,
                                        const std::string& collectionId)
            {
                if (!Authenticate(req))
                    return crow::response(401, "Unauthorized");

                spdlog::info("DELETE /projects/{}/collections/{}", projectId, collectionId);
                std::string compositeId = projectId + "/" + collectionId;
                if (dataStore.DeleteCollection(compositeId))
                    return crow::response(204);
                return crow::response(404, Ser::Error(404, "Collection not found"));
            });

    spdlog::info(
        "Routes: health, projects, workitems (CRUD+bulk), linkedworkitems, comments, attachments, documents, collections");
    spdlog::info("Listening on http://0.0.0.0:18080");
    app.port(18080).multithreaded().run();

    spdlog::info("PolarionMockup shutting down");

    return 0;
}
