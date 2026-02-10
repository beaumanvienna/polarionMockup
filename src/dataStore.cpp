/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "dataStore.h"

namespace PolarionMockup
{

    DataStore::DataStore() { LoadSeedData(); }

    // ================================================================
    // Public query methods
    // ================================================================

    const Project* DataStore::GetProject(const std::string& id) const
    {
        auto it = m_Projects.find(id);
        return (it != m_Projects.end()) ? &it->second : nullptr;
    }

    std::vector<const Project*> DataStore::GetAllProjects() const
    {
        std::vector<const Project*> result;
        result.reserve(m_Projects.size());
        for (const auto& [key, proj] : m_Projects)
        {
            result.push_back(&proj);
        }
        return result;
    }

    const User* DataStore::GetUser(const std::string& id) const
    {
        auto it = m_Users.find(id);
        return (it != m_Users.end()) ? &it->second : nullptr;
    }

    std::vector<const User*> DataStore::GetAllUsers() const
    {
        std::vector<const User*> result;
        result.reserve(m_Users.size());
        for (const auto& [key, user] : m_Users)
        {
            result.push_back(&user);
        }
        return result;
    }

    const WorkItem* DataStore::GetWorkItem(const std::string& compositeId) const
    {
        auto it = m_WorkItems.find(compositeId);
        return (it != m_WorkItems.end()) ? &it->second : nullptr;
    }

    std::vector<const WorkItem*> DataStore::GetWorkItemsByProject(const std::string& projectId) const
    {
        std::vector<const WorkItem*> result;
        for (const auto& [key, wi] : m_WorkItems)
        {
            if (wi.GetProjectId() == projectId)
            {
                result.push_back(&wi);
            }
        }
        return result;
    }

    std::vector<const WorkItem*> DataStore::GetAllWorkItems() const
    {
        std::vector<const WorkItem*> result;
        result.reserve(m_WorkItems.size());
        for (const auto& [key, wi] : m_WorkItems)
        {
            result.push_back(&wi);
        }
        return result;
    }

    const Document* DataStore::GetDocument(const std::string& compositeId) const
    {
        auto it = m_Documents.find(compositeId);
        return (it != m_Documents.end()) ? &it->second : nullptr;
    }

    std::vector<const Document*> DataStore::GetDocumentsByProject(const std::string& projectId) const
    {
        std::vector<const Document*> result;
        for (const auto& [key, doc] : m_Documents)
        {
            if (doc.GetProjectId() == projectId)
            {
                result.push_back(&doc);
            }
        }
        return result;
    }

    // ================================================================
    // Query
    // ================================================================

    std::vector<const WorkItem*> DataStore::QueryWorkItems(const std::string& projectId,
                                                           const std::string& queryString) const
    {
        auto projectItems = GetWorkItemsByProject(projectId);

        if (queryString.empty())
        {
            return projectItems;
        }

        QueryEngine engine;
        return engine.Filter(queryString, projectItems);
    }

    // ================================================================
    // Mutations
    // ================================================================

    WorkItem* DataStore::CreateWorkItem(const std::string& projectId, WorkItem&& item)
    {
        std::string compositeId = item.GetId();
        if (m_WorkItems.count(compositeId))
        {
            return nullptr;
        }
        auto [it, inserted] = m_WorkItems.emplace(compositeId, std::move(item));
        return inserted ? &it->second : nullptr;
    }

    WorkItem* DataStore::UpdateWorkItem(const std::string& compositeId)
    {
        auto it = m_WorkItems.find(compositeId);
        return (it != m_WorkItems.end()) ? &it->second : nullptr;
    }

    bool DataStore::DeleteWorkItem(const std::string& compositeId) { return m_WorkItems.erase(compositeId) > 0; }

    // ================================================================
    // Comments
    // ================================================================

    Comment* DataStore::AddComment(const std::string& workItemId, Comment&& comment)
    {
        std::string id = comment.GetId();
        auto [it, inserted] = m_Comments.emplace(id, std::move(comment));
        return inserted ? &it->second : nullptr;
    }

    std::vector<const Comment*> DataStore::GetCommentsByWorkItem(const std::string& workItemId) const
    {
        std::vector<const Comment*> result;
        for (const auto& [key, c] : m_Comments)
        {
            if (c.GetWorkItemId() == workItemId)
                result.push_back(&c);
        }
        return result;
    }

    const Comment* DataStore::GetComment(const std::string& commentId) const
    {
        auto it = m_Comments.find(commentId);
        return (it != m_Comments.end()) ? &it->second : nullptr;
    }

    Comment* DataStore::GetMutableComment(const std::string& commentId)
    {
        auto it = m_Comments.find(commentId);
        return (it != m_Comments.end()) ? &it->second : nullptr;
    }

    bool DataStore::DeleteComment(const std::string& commentId) { return m_Comments.erase(commentId) > 0; }

    // ================================================================
    // Attachments
    // ================================================================

    Attachment* DataStore::AddAttachment(const std::string& workItemId, Attachment&& attachment)
    {
        std::string id = attachment.GetId();
        auto [it, inserted] = m_Attachments.emplace(id, std::move(attachment));
        return inserted ? &it->second : nullptr;
    }

    std::vector<const Attachment*> DataStore::GetAttachmentsByWorkItem(const std::string& workItemId) const
    {
        std::vector<const Attachment*> result;
        for (const auto& [key, a] : m_Attachments)
        {
            if (a.GetWorkItemId() == workItemId)
                result.push_back(&a);
        }
        return result;
    }

    bool DataStore::DeleteAttachment(const std::string& attachmentId) { return m_Attachments.erase(attachmentId) > 0; }

    // ================================================================
    // Collections
    // ================================================================

    Collection* DataStore::CreateCollection(const std::string& projectId, Collection&& collection)
    {
        std::string id = collection.GetId();
        if (m_Collections.count(id))
            return nullptr;
        auto [it, inserted] = m_Collections.emplace(id, std::move(collection));
        return inserted ? &it->second : nullptr;
    }

    const Collection* DataStore::GetCollection(const std::string& compositeId) const
    {
        auto it = m_Collections.find(compositeId);
        return (it != m_Collections.end()) ? &it->second : nullptr;
    }

    Collection* DataStore::GetMutableCollection(const std::string& compositeId)
    {
        auto it = m_Collections.find(compositeId);
        return (it != m_Collections.end()) ? &it->second : nullptr;
    }

    std::vector<const Collection*> DataStore::GetCollectionsByProject(const std::string& projectId) const
    {
        std::vector<const Collection*> result;
        for (const auto& [key, c] : m_Collections)
        {
            if (c.GetProjectId() == projectId)
                result.push_back(&c);
        }
        return result;
    }

    bool DataStore::DeleteCollection(const std::string& compositeId) { return m_Collections.erase(compositeId) > 0; }

    // ================================================================
    // ID generation
    // ================================================================

    int DataStore::NextCommentId() { return m_NextCommentId++; }
    int DataStore::NextAttachmentId() { return m_NextAttachmentId++; }

    // ================================================================
    // Seed data
    // ================================================================

    void DataStore::LoadSeedData()
    {
        SeedUsers();
        SeedProject();
        SeedDocument();

        SeedChapter1GeneralPerformance();
        SeedChapter2PropulsionAndBrakes();
        SeedChapter3SeatAndSteering();
        SeedChapter4PaintAndLivery();
    }

    void DataStore::SeedUsers()
    {
        m_Users.emplace("JC", User("JC", "JC"));
        m_Users.emplace("Bob", User("Bob", "Bob"));
    }

    void DataStore::SeedProject()
    {
        Project proj(PROJ_ID, "Go-Kart Procurement");
        proj.SetDescription("Technical specification for procuring go-karts for our go-kart venue.");
        m_Projects.emplace(PROJ_ID, std::move(proj));
    }

    void DataStore::SeedDocument()
    {
        Document doc(PROJ_ID, SPACE_ID, DOC_ID, "Go-Kart Technical Specification");
        m_Documents.emplace(doc.GetId(), std::move(doc));
    }

    void DataStore::AddWorkItem(WorkItem& item)
    {
        std::string compositeId = item.GetId();
        std::string docKey = std::string(PROJ_ID) + "/" + SPACE_ID + "/" + DOC_ID;

        auto docIt = m_Documents.find(docKey);
        if (docIt != m_Documents.end())
        {
            docIt->second.AddWorkItem(compositeId);
        }
        item.SetModuleId(docKey);

        m_WorkItems.emplace(compositeId, std::move(item));
    }

    // ----------------------------------------------------------------
    // Chapter 1 — General Performance
    //   Assignees: JC, Bob
    // ----------------------------------------------------------------
    void DataStore::SeedChapter1GeneralPerformance()
    {
        {
            WorkItem wi(PROJ_ID, "REQ-001");
            wi.SetTitle("Maximum speed");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("90.0");
            wi.SetSeverity("major");
            wi.SetOutlineNumber("1.1");
            wi.SetDescription("text/plain", "The go-kart shall achieve a maximum speed of no less than "
                                            "60 km/h on a flat, dry track surface.");
            wi.SetStatus("approved");
            wi.SetCreated("2026-02-01T10:00:00Z");
            wi.SetUpdated("2026-02-05T14:00:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            wi.AddAssignee("Bob");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-002");
            wi.SetTitle("Acceleration");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("80.0");
            wi.SetSeverity("major");
            wi.SetOutlineNumber("1.2");
            wi.SetDescription("text/plain", "The go-kart shall accelerate from 0 to 50 km/h in "
                                            "5 seconds or less.");
            wi.SetStatus("approved");
            wi.SetCreated("2026-02-01T10:05:00Z");
            wi.SetUpdated("2026-02-05T14:10:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            wi.AddAssignee("Bob");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-003");
            wi.SetTitle("Kart weight");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("70.0");
            wi.SetSeverity("normal");
            wi.SetOutlineNumber("1.3");
            wi.SetDescription("text/plain", "The total kart weight without driver shall not exceed 120 kg.");
            wi.SetStatus("open");
            wi.SetCreated("2026-02-01T10:10:00Z");
            wi.SetUpdated("2026-02-01T10:10:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            wi.AddAssignee("Bob");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-004");
            wi.SetTitle("Operating time");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("90.0");
            wi.SetSeverity("major");
            wi.SetOutlineNumber("1.4");
            wi.SetDescription("text/plain", "Continuous operating time shall be at least 30 minutes "
                                            "per full charge.");
            wi.SetStatus("reviewed");
            wi.SetCreated("2026-02-01T10:15:00Z");
            wi.SetUpdated("2026-02-04T09:00:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            wi.AddAssignee("Bob");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-005");
            wi.SetTitle("Operating temperature range");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("60.0");
            wi.SetSeverity("normal");
            wi.SetOutlineNumber("1.5");
            wi.SetDescription("text/plain", "The go-kart shall operate safely in ambient temperatures "
                                            "ranging from 5 degrees C to 40 degrees C.");
            wi.SetStatus("draft");
            wi.SetCreated("2026-02-01T10:20:00Z");
            wi.SetUpdated("2026-02-01T10:20:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            wi.AddAssignee("Bob");
            AddWorkItem(wi);
        }
    }

    // ----------------------------------------------------------------
    // Chapter 2 — Propulsion and Friction Brakes
    //   Assignees: JC
    //   Category:  PropFB
    // ----------------------------------------------------------------
    void DataStore::SeedChapter2PropulsionAndBrakes()
    {
        {
            WorkItem wi(PROJ_ID, "REQ-006");
            wi.SetTitle("Electric motor power");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("90.0");
            wi.SetSeverity("blocker");
            wi.SetOutlineNumber("2.1");
            wi.SetDescription("text/plain", "The go-kart shall be powered by an electric motor with "
                                            "a minimum peak output of 10 kW.");
            wi.SetStatus("approved");
            wi.SetCreated("2026-02-01T10:30:00Z");
            wi.SetUpdated("2026-02-06T11:00:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            wi.AddCategory("PropFB");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-007");
            wi.SetTitle("Hydraulic disc brakes");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("90.0");
            wi.SetSeverity("blocker");
            wi.SetOutlineNumber("2.2");
            wi.SetDescription("text/plain", "The go-kart shall feature hydraulic disc brakes on all "
                                            "four wheels.");
            wi.SetStatus("approved");
            wi.SetCreated("2026-02-01T10:35:00Z");
            wi.SetUpdated("2026-02-06T11:05:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            wi.AddCategory("PropFB");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-008");
            wi.SetTitle("Emergency braking distance");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("90.0");
            wi.SetSeverity("blocker");
            wi.SetOutlineNumber("2.3");
            wi.SetDescription("text/plain", "An emergency braking system shall bring the go-kart to a "
                                            "full stop within 3 meters from a speed of 40 km/h.");
            wi.SetStatus("reviewed");
            wi.SetCreated("2026-02-01T10:40:00Z");
            wi.SetUpdated("2026-02-04T09:30:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            wi.AddCategory("PropFB");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-009");
            wi.SetTitle("Battery capacity and charging");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("80.0");
            wi.SetSeverity("major");
            wi.SetOutlineNumber("2.4");
            wi.SetDescription("text/plain", "Battery capacity shall be at least 2.5 kWh with a full "
                                            "charging time under 2 hours.");
            wi.SetStatus("open");
            wi.SetCreated("2026-02-01T10:45:00Z");
            wi.SetUpdated("2026-02-01T10:45:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            wi.AddCategory("PropFB");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-010");
            wi.SetTitle("Regenerative braking");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("70.0");
            wi.SetSeverity("normal");
            wi.SetOutlineNumber("2.5");
            wi.SetDescription("text/plain", "The drive chain shall include a regenerative braking energy "
                                            "recovery system that feeds kinetic energy back to the battery.");
            wi.SetStatus("draft");
            wi.SetCreated("2026-02-01T10:50:00Z");
            wi.SetUpdated("2026-02-01T10:50:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            wi.AddCategory("PropFB");
            AddWorkItem(wi);
        }
    }

    // ----------------------------------------------------------------
    // Chapter 3 — Seat and Steering Wheel
    //   Assignees: JC
    // ----------------------------------------------------------------
    void DataStore::SeedChapter3SeatAndSteering()
    {
        {
            WorkItem wi(PROJ_ID, "REQ-011");
            wi.SetTitle("Adjustable seat");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("80.0");
            wi.SetSeverity("major");
            wi.SetOutlineNumber("3.1");
            wi.SetDescription("text/plain", "The seat shall be adjustable to accommodate drivers from "
                                            "150 cm to 200 cm in height.");
            wi.SetStatus("reviewed");
            wi.SetCreated("2026-02-01T11:00:00Z");
            wi.SetUpdated("2026-02-04T10:00:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-012");
            wi.SetTitle("Safety harness");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("90.0");
            wi.SetSeverity("blocker");
            wi.SetOutlineNumber("3.2");
            wi.SetDescription("text/plain", "A four-point safety harness shall be provided for "
                                            "each seat.");
            wi.SetStatus("approved");
            wi.SetCreated("2026-02-01T11:05:00Z");
            wi.SetUpdated("2026-02-06T11:30:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-013");
            wi.SetTitle("Steering wheel specification");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("70.0");
            wi.SetSeverity("normal");
            wi.SetOutlineNumber("3.3");
            wi.SetDescription("text/plain", "Steering wheel diameter shall be between 280 mm and "
                                            "320 mm with a quick-release hub.");
            wi.SetStatus("open");
            wi.SetCreated("2026-02-01T11:10:00Z");
            wi.SetUpdated("2026-02-01T11:10:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-014");
            wi.SetTitle("Lateral seat support");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("60.0");
            wi.SetSeverity("normal");
            wi.SetOutlineNumber("3.4");
            wi.SetDescription("text/plain", "The seat shall have lateral support padding to secure "
                                            "the driver during high-speed cornering.");
            wi.SetStatus("draft");
            wi.SetCreated("2026-02-01T11:15:00Z");
            wi.SetUpdated("2026-02-01T11:15:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("JC");
            AddWorkItem(wi);
        }
    }

    // ----------------------------------------------------------------
    // Chapter 4 — Paint, Livery
    //   Assignees: Bob
    // ----------------------------------------------------------------
    void DataStore::SeedChapter4PaintAndLivery()
    {
        {
            WorkItem wi(PROJ_ID, "REQ-015");
            wi.SetTitle("UV-resistant paint");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("60.0");
            wi.SetSeverity("normal");
            wi.SetOutlineNumber("4.1");
            wi.SetDescription("text/plain", "All exterior surfaces shall be coated with UV-resistant "
                                            "automotive-grade paint.");
            wi.SetStatus("open");
            wi.SetCreated("2026-02-01T11:30:00Z");
            wi.SetUpdated("2026-02-01T11:30:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("Bob");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-016");
            wi.SetTitle("Company logo placement");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("50.0");
            wi.SetSeverity("normal");
            wi.SetOutlineNumber("4.2");
            wi.SetDescription("text/plain", "Each go-kart shall carry the company logo on both sides "
                                            "and the front nose.");
            wi.SetStatus("draft");
            wi.SetCreated("2026-02-01T11:35:00Z");
            wi.SetUpdated("2026-02-01T11:35:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("Bob");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-017");
            wi.SetTitle("High-visibility number panels");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("70.0");
            wi.SetSeverity("normal");
            wi.SetOutlineNumber("4.3");
            wi.SetDescription("text/plain", "High-visibility number panels shall be mounted on the "
                                            "front and both sides of each go-kart.");
            wi.SetStatus("reviewed");
            wi.SetCreated("2026-02-01T11:40:00Z");
            wi.SetUpdated("2026-02-04T10:30:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("Bob");
            AddWorkItem(wi);
        }
        {
            WorkItem wi(PROJ_ID, "REQ-018");
            wi.SetTitle("Reflective strips");
            wi.SetWorkItemType("requirement");
            wi.SetPriority("60.0");
            wi.SetSeverity("normal");
            wi.SetOutlineNumber("4.4");
            wi.SetDescription("text/plain", "Reflective strips shall be applied to the rear and side "
                                            "pods for low-light visibility.");
            wi.SetStatus("open");
            wi.SetCreated("2026-02-01T11:45:00Z");
            wi.SetUpdated("2026-02-01T11:45:00Z");
            wi.SetAuthorId("JC");
            wi.AddAssignee("Bob");
            AddWorkItem(wi);
        }
    }

} // namespace PolarionMockup
