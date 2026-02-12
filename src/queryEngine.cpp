/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "queryEngine.h"

#include <algorithm>
#include <cctype>

namespace PolarionMockup
{

    // ================================================================
    // Public API
    // ================================================================

    bool QueryEngine::Matches(const QueryNode* root, const WorkItem& item) const
    {
        if (root == nullptr)
        {
            return true; // null query matches everything
        }
        return Evaluate(root, item);
    }

    std::vector<const WorkItem*> QueryEngine::Filter(const std::string& queryString,
                                                     const std::vector<const WorkItem*>& items) const
    {
        QueryParser parser;
        auto root = parser.Parse(queryString);

        std::vector<const WorkItem*> result;
        for (const auto* item : items)
        {
            if (item && Matches(root.get(), *item))
            {
                result.push_back(item);
            }
        }
        return result;
    }

    // ================================================================
    // AST Evaluation
    // ================================================================

    bool QueryEngine::Evaluate(const QueryNode* node, const WorkItem& item) const
    {
        switch (node->GetNodeType())
        {
            case QueryNodeType::And:
            {
                for (const auto& child : node->GetChildren())
                {
                    if (!Evaluate(child.get(), item))
                    {
                        return false;
                    }
                }
                return true;
            }

            case QueryNodeType::Or:
            {
                for (const auto& child : node->GetChildren())
                {
                    if (Evaluate(child.get(), item))
                    {
                        return true;
                    }
                }
                return false;
            }

            case QueryNodeType::Not:
            {
                if (node->GetChildren().empty())
                {
                    return true;
                }
                return !Evaluate(node->GetChildren()[0].get(), item);
            }

            case QueryNodeType::FieldMatch:
            {
                auto values = ResolveField(node->GetField(), item);
                for (const auto& val : values)
                {
                    if (MatchesValue(val, node->GetValue()))
                    {
                        return true;
                    }
                }
                return false;
            }

            case QueryNodeType::FieldWildcard:
            {
                auto values = ResolveField(node->GetField(), item);
                for (const auto& val : values)
                {
                    if (MatchesPrefix(val, node->GetValue()))
                    {
                        return true;
                    }
                }
                return false;
            }

            case QueryNodeType::FieldRange:
            {
                auto values = ResolveField(node->GetField(), item);
                for (const auto& val : values)
                {
                    if (InRange(val, node->GetRangeLow(), node->GetRangeHigh(), node->IsRangeInclusive()))
                    {
                        return true;
                    }
                }
                return false;
            }
        }

        return false;
    }

    // ================================================================
    // Field Resolution
    // ================================================================

    std::vector<std::string> QueryEngine::ResolveField(const std::string& field, const WorkItem& item) const
    {
        // Convert field name to lowercase for case-insensitive field matching
        std::string f = field;
        std::transform(f.begin(), f.end(), f.begin(), [](unsigned char c) { return std::tolower(c); });

        if (f == "type")
            return {item.GetWorkItemType()};
        if (f == "status")
            return {item.GetStatus()};
        if (f == "title")
            return {item.GetTitle()};
        if (f == "severity")
            return {item.GetSeverity()};
        if (f == "priority")
            return {item.GetPriority()};
        if (f == "resolution")
            return {item.GetResolution()};
        if (f == "created")
            return {item.GetCreated()};
        if (f == "updated")
            return {item.GetUpdated()};
        if (f == "author")
            return {item.GetAuthorId()};
        if (f == "outlinenumber")
            return {item.GetOutlineNumber()};
        if (f == "id")
        {
            // Return the short ID part (after projectId/)
            const std::string& fullId = item.GetId();
            auto slash = fullId.find('/');
            if (slash != std::string::npos)
            {
                return {fullId.substr(slash + 1)};
            }
            return {fullId};
        }

        // Multi-valued fields
        if (f == "assignee" || f == "assignees" || f == "tags")
        {
            return item.GetAssigneeIds();
        }
        if (f == "categories" || f == "category")
        {
            return item.GetCategoryIds();
        }

        // Description value
        if (f == "description" || f == "body")
        {
            return {item.GetDescription().m_Value};
        }

        // Module / document
        if (f == "module" || f == "document")
        {
            return {item.GetModuleId()};
        }

        // Project
        if (f == "project")
        {
            return {item.GetProjectId()};
        }

        return {};
    }

    // ================================================================
    // String Matching Helpers
    // ================================================================

    static std::string ToLower(const std::string& s)
    {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    bool QueryEngine::MatchesValue(const std::string& fieldVal, const std::string& queryVal)
    {
        // Lucene-style: match if the query value appears as a word/substring
        // in the field value (case-insensitive).  This handles both
        // exact fields (status:approved) and text fields (title:braking).
        std::string lowerField = ToLower(fieldVal);
        std::string lowerQuery = ToLower(queryVal);
        return lowerField.find(lowerQuery) != std::string::npos;
    }

    bool QueryEngine::MatchesPrefix(const std::string& fieldVal, const std::string& prefix)
    {
        // Lucene-style: match if any whitespace-delimited token in the
        // field value starts with the prefix (case-insensitive).
        std::string lowerField = ToLower(fieldVal);
        std::string lowerPrefix = ToLower(prefix);

        size_t i = 0;
        while (i < lowerField.size())
        {
            // Skip whitespace
            while (i < lowerField.size() && std::isspace(static_cast<unsigned char>(lowerField[i])))
                ++i;
            // Start of token
            size_t start = i;
            while (i < lowerField.size() && !std::isspace(static_cast<unsigned char>(lowerField[i])))
                ++i;
            size_t len = i - start;
            if (len >= lowerPrefix.size() && lowerField.compare(start, lowerPrefix.size(), lowerPrefix) == 0)
            {
                return true;
            }
        }
        return false;
    }

    bool QueryEngine::InRange(const std::string& fieldVal, const std::string& low, const std::string& high, bool inclusive)
    {
        // Lexicographic comparison (works for ISO dates and numeric strings)
        std::string lv = ToLower(fieldVal);
        std::string ll = ToLower(low);
        std::string lh = ToLower(high);

        if (inclusive)
        {
            return lv >= ll && lv <= lh;
        }
        return lv > ll && lv < lh;
    }

} // namespace PolarionMockup
