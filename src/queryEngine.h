/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include "queryParser.h"
#include "workItem.h"

#include <string>
#include <vector>

namespace PolarionMockup
{

    class QueryEngine
    {
    public:

        QueryEngine() = default;

        // Returns true if the work item matches the query AST.
        // A null query (empty string) matches everything.
        bool Matches(const QueryNode* root, const WorkItem& item) const;

        // Filter a list of work items by a query string.
        // Parses the query, then evaluates each item.
        std::vector<const WorkItem*> Filter(
            const std::string& queryString,
            const std::vector<const WorkItem*>& items) const;

    private:

        bool Evaluate(const QueryNode* node, const WorkItem& item) const;

        // Resolve a field name to the work item's value(s).
        // Multi-valued fields (assignee, categories) return multiple strings.
        std::vector<std::string> ResolveField(const std::string& field,
                                              const WorkItem& item) const;

        // String comparison helpers
        static bool MatchesValue(const std::string& fieldVal, const std::string& queryVal);
        static bool MatchesPrefix(const std::string& fieldVal, const std::string& prefix);
        static bool InRange(const std::string& fieldVal,
                            const std::string& low, const std::string& high,
                            bool inclusive);
    };

} // namespace PolarionMockup
