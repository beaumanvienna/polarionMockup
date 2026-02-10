/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include "project.h"
#include "user.h"
#include "workItem.h"
#include "document.h"
#include "comment.h"
#include "attachment.h"
#include "collection.h"
#include "queryEngine.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace PolarionMockup
{

    class DataStore
    {
    public:
        DataStore();

        // --- Projects ---
        const Project* GetProject(const std::string& id) const;
        std::vector<const Project*> GetAllProjects() const;

        // --- Users ---
        const User* GetUser(const std::string& id) const;
        std::vector<const User*> GetAllUsers() const;

        // --- Work Items ---
        const WorkItem* GetWorkItem(const std::string& compositeId) const;
        std::vector<const WorkItem*> GetWorkItemsByProject(const std::string& projectId) const;
        std::vector<const WorkItem*> GetAllWorkItems() const;

        // Query work items using Lucene-style query string
        std::vector<const WorkItem*> QueryWorkItems(const std::string& projectId, const std::string& queryString) const;

        // --- Work Item mutations ---
        WorkItem* CreateWorkItem(const std::string& projectId, WorkItem&& item);
        WorkItem* UpdateWorkItem(const std::string& compositeId);
        bool DeleteWorkItem(const std::string& compositeId);

        // --- Documents ---
        const Document* GetDocument(const std::string& compositeId) const;
        std::vector<const Document*> GetDocumentsByProject(const std::string& projectId) const;

        // --- Comments ---
        Comment* AddComment(const std::string& workItemId, Comment&& comment);
        std::vector<const Comment*> GetCommentsByWorkItem(const std::string& workItemId) const;
        const Comment* GetComment(const std::string& commentId) const;
        Comment* GetMutableComment(const std::string& commentId);
        bool DeleteComment(const std::string& commentId);

        // --- Attachments ---
        Attachment* AddAttachment(const std::string& workItemId, Attachment&& attachment);
        std::vector<const Attachment*> GetAttachmentsByWorkItem(const std::string& workItemId) const;
        bool DeleteAttachment(const std::string& attachmentId);

        // --- Collections ---
        Collection* CreateCollection(const std::string& projectId, Collection&& collection);
        const Collection* GetCollection(const std::string& compositeId) const;
        Collection* GetMutableCollection(const std::string& compositeId);
        std::vector<const Collection*> GetCollectionsByProject(const std::string& projectId) const;
        bool DeleteCollection(const std::string& compositeId);

        // --- ID generation ---
        int NextCommentId();
        int NextAttachmentId();

    private:
        void LoadSeedData();

        void SeedUsers();
        void SeedProject();
        void SeedDocument();
        void SeedChapter1GeneralPerformance();
        void SeedChapter2PropulsionAndBrakes();
        void SeedChapter3SeatAndSteering();
        void SeedChapter4PaintAndLivery();

        void AddWorkItem(WorkItem& item);

        std::unordered_map<std::string, Project> m_Projects;
        std::unordered_map<std::string, User> m_Users;
        std::unordered_map<std::string, WorkItem> m_WorkItems;
        std::unordered_map<std::string, Document> m_Documents;
        std::unordered_map<std::string, Comment> m_Comments;
        std::unordered_map<std::string, Attachment> m_Attachments;
        std::unordered_map<std::string, Collection> m_Collections;

        int m_NextCommentId{1};
        int m_NextAttachmentId{1};

        // Seed-data constants
        static constexpr const char* PROJ_ID = "GoKartProcurement";
        static constexpr const char* SPACE_ID = "Specifications";
        static constexpr const char* DOC_ID = "TechnicalSpec";
    };

} // namespace PolarionMockup
