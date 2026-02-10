/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace PolarionMockup
{

    // ================================================================
    // AST Node Types
    // ================================================================

    enum class QueryNodeType
    {
        And,
        Or,
        Not,
        FieldMatch,
        FieldRange,
        FieldWildcard
    };

    class QueryNode
    {
    public:

        QueryNode() = default;
        explicit QueryNode(QueryNodeType type);
        virtual ~QueryNode() = default;

        QueryNodeType GetNodeType() const;

        // Boolean node children
        void AddChild(std::unique_ptr<QueryNode> child);
        const std::vector<std::unique_ptr<QueryNode>>& GetChildren() const;

        // Field operations
        const std::string& GetField() const;
        void SetField(const std::string& field);

        const std::string& GetValue() const;
        void SetValue(const std::string& value);

        // Range operations
        const std::string& GetRangeLow() const;
        void SetRangeLow(const std::string& low);

        const std::string& GetRangeHigh() const;
        void SetRangeHigh(const std::string& high);

        bool IsRangeInclusive() const;
        void SetRangeInclusive(bool inclusive);

    private:

        QueryNodeType m_NodeType{QueryNodeType::And};
        std::string m_Field;
        std::string m_Value;
        std::string m_RangeLow;
        std::string m_RangeHigh;
        bool m_RangeInclusive{true};
        std::vector<std::unique_ptr<QueryNode>> m_Children;
    };

    // ================================================================
    // Token Types (internal to parser, exposed for testability)
    // ================================================================

    enum class TokenType
    {
        Word,          // field name or value
        Colon,         // :
        LParen,        // (
        RParen,        // )
        LBracket,      // [
        RBracket,      // ]
        LBrace,        // {
        RBrace,        // }
        And,           // AND
        Or,            // OR
        Not,           // NOT
        To,            // TO
        Minus,         // - (prefix negation)
        Star,          // * (wildcard suffix)
        Eof
    };

    struct Token
    {
        TokenType m_Type{TokenType::Eof};
        std::string m_Value;
    };

    // ================================================================
    // QueryParser — Lucene-style recursive descent parser
    //
    // Grammar:
    //   query     := orExpr
    //   orExpr    := andExpr ("OR" andExpr)*
    //   andExpr   := unaryExpr ("AND" unaryExpr)*
    //   unaryExpr := "NOT" unaryExpr
    //              | "-" fieldExpr
    //              | "(" query ")"
    //              | fieldExpr
    //   fieldExpr := FIELD ":" ( rangeExpr | value ["*"] )
    //   rangeExpr := "[" value "TO" value "]"
    //              | "{" value "TO" value "}"
    // ================================================================

    class QueryParser
    {
    public:

        QueryParser() = default;

        std::unique_ptr<QueryNode> Parse(const std::string& queryString);

    private:

        std::vector<Token> Tokenize(const std::string& input);

        std::unique_ptr<QueryNode> ParseOrExpr();
        std::unique_ptr<QueryNode> ParseAndExpr();
        std::unique_ptr<QueryNode> ParseUnaryExpr();
        std::unique_ptr<QueryNode> ParseFieldExpr();
        std::unique_ptr<QueryNode> ParseRangeExpr(const std::string& field, bool inclusive);

        const Token& Current() const;
        const Token& Advance();
        bool Match(TokenType type);
        bool IsAtEnd() const;

        std::vector<Token> m_Tokens;
        size_t m_Pos{0};
    };

} // namespace PolarionMockup
