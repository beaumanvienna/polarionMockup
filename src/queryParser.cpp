/* Copyright (c) 2026 JC Technolabs
   License: MIT */

#include "queryParser.h"

#include <cctype>
#include <stdexcept>

namespace PolarionMockup
{

    // ================================================================
    // QueryNode
    // ================================================================

    QueryNode::QueryNode(QueryNodeType type)
        : m_NodeType(type)
    {
    }

    QueryNodeType QueryNode::GetNodeType() const
    {
        return m_NodeType;
    }

    void QueryNode::AddChild(std::unique_ptr<QueryNode> child)
    {
        m_Children.push_back(std::move(child));
    }

    const std::vector<std::unique_ptr<QueryNode>>& QueryNode::GetChildren() const
    {
        return m_Children;
    }

    const std::string& QueryNode::GetField() const
    {
        return m_Field;
    }

    void QueryNode::SetField(const std::string& field)
    {
        m_Field = field;
    }

    const std::string& QueryNode::GetValue() const
    {
        return m_Value;
    }

    void QueryNode::SetValue(const std::string& value)
    {
        m_Value = value;
    }

    const std::string& QueryNode::GetRangeLow() const
    {
        return m_RangeLow;
    }

    void QueryNode::SetRangeLow(const std::string& low)
    {
        m_RangeLow = low;
    }

    const std::string& QueryNode::GetRangeHigh() const
    {
        return m_RangeHigh;
    }

    void QueryNode::SetRangeHigh(const std::string& high)
    {
        m_RangeHigh = high;
    }

    bool QueryNode::IsRangeInclusive() const
    {
        return m_RangeInclusive;
    }

    void QueryNode::SetRangeInclusive(bool inclusive)
    {
        m_RangeInclusive = inclusive;
    }

    // ================================================================
    // QueryParser — Tokenizer
    // ================================================================

    static bool IsWordChar(char c)
    {
        // Allow alphanumeric, underscore, hyphen, dot, asterisk, @, !, #, $, %, etc.
        // Basically anything that isn't a special delimiter
        return c != '\0' && c != ' ' && c != '\t' && c != '\n' &&
               c != ':' && c != '(' && c != ')' &&
               c != '[' && c != ']' && c != '{' && c != '}';
    }

    std::vector<Token> QueryParser::Tokenize(const std::string& input)
    {
        std::vector<Token> tokens;
        size_t i = 0;

        while (i < input.size())
        {
            char c = input[i];

            // Skip whitespace
            if (std::isspace(static_cast<unsigned char>(c)))
            {
                ++i;
                continue;
            }

            // Single-character tokens
            if (c == ':') { tokens.push_back({TokenType::Colon, ":"}); ++i; continue; }
            if (c == '(') { tokens.push_back({TokenType::LParen, "("}); ++i; continue; }
            if (c == ')') { tokens.push_back({TokenType::RParen, ")"}); ++i; continue; }
            if (c == '[') { tokens.push_back({TokenType::LBracket, "["}); ++i; continue; }
            if (c == ']') { tokens.push_back({TokenType::RBracket, "]"}); ++i; continue; }
            if (c == '{') { tokens.push_back({TokenType::LBrace, "{"}); ++i; continue; }
            if (c == '}') { tokens.push_back({TokenType::RBrace, "}"}); ++i; continue; }

            // Minus as prefix negation (only if followed by a word char)
            if (c == '-' && i + 1 < input.size() && IsWordChar(input[i + 1]))
            {
                tokens.push_back({TokenType::Minus, "-"});
                ++i;
                continue;
            }

            // Word (field name, value, or keyword)
            if (IsWordChar(c))
            {
                size_t start = i;
                while (i < input.size() && IsWordChar(input[i]))
                {
                    ++i;
                }
                std::string word = input.substr(start, i - start);

                // Check for keywords
                if (word == "AND")      { tokens.push_back({TokenType::And, word}); }
                else if (word == "OR")  { tokens.push_back({TokenType::Or, word}); }
                else if (word == "NOT") { tokens.push_back({TokenType::Not, word}); }
                else if (word == "TO")  { tokens.push_back({TokenType::To, word}); }
                else
                {
                    // Check for trailing wildcard: "val*"
                    if (word.size() > 1 && word.back() == '*')
                    {
                        word.pop_back();
                        tokens.push_back({TokenType::Word, word});
                        tokens.push_back({TokenType::Star, "*"});
                    }
                    else
                    {
                        tokens.push_back({TokenType::Word, word});
                    }
                }
                continue;
            }

            // Unknown character — skip
            ++i;
        }

        tokens.push_back({TokenType::Eof, ""});
        return tokens;
    }

    // ================================================================
    // QueryParser — Recursive Descent
    // ================================================================

    std::unique_ptr<QueryNode> QueryParser::Parse(const std::string& queryString)
    {
        m_Tokens = Tokenize(queryString);
        m_Pos = 0;

        if (IsAtEnd())
        {
            // Empty query matches everything
            return nullptr;
        }

        auto root = ParseOrExpr();

        if (!IsAtEnd())
        {
            throw std::runtime_error("QueryParser: unexpected token at position " +
                                     std::to_string(m_Pos));
        }

        return root;
    }

    // orExpr := andExpr ("OR" andExpr)*
    std::unique_ptr<QueryNode> QueryParser::ParseOrExpr()
    {
        auto left = ParseAndExpr();

        if (Current().m_Type == TokenType::Or)
        {
            auto orNode = std::make_unique<QueryNode>(QueryNodeType::Or);
            orNode->AddChild(std::move(left));

            while (Match(TokenType::Or))
            {
                orNode->AddChild(ParseAndExpr());
            }
            return orNode;
        }

        return left;
    }

    // andExpr := unaryExpr ("AND" unaryExpr)*
    std::unique_ptr<QueryNode> QueryParser::ParseAndExpr()
    {
        auto left = ParseUnaryExpr();

        if (Current().m_Type == TokenType::And)
        {
            auto andNode = std::make_unique<QueryNode>(QueryNodeType::And);
            andNode->AddChild(std::move(left));

            while (Match(TokenType::And))
            {
                andNode->AddChild(ParseUnaryExpr());
            }
            return andNode;
        }

        return left;
    }

    // unaryExpr := "NOT" unaryExpr | "-" fieldExpr | "(" query ")" | fieldExpr
    std::unique_ptr<QueryNode> QueryParser::ParseUnaryExpr()
    {
        // NOT prefix
        if (Match(TokenType::Not))
        {
            auto notNode = std::make_unique<QueryNode>(QueryNodeType::Not);
            notNode->AddChild(ParseUnaryExpr());
            return notNode;
        }

        // - prefix (shorthand for NOT)
        if (Match(TokenType::Minus))
        {
            auto notNode = std::make_unique<QueryNode>(QueryNodeType::Not);
            notNode->AddChild(ParseFieldExpr());
            return notNode;
        }

        // Grouped expression
        if (Match(TokenType::LParen))
        {
            auto inner = ParseOrExpr();
            if (!Match(TokenType::RParen))
            {
                throw std::runtime_error("QueryParser: expected ')'");
            }
            return inner;
        }

        return ParseFieldExpr();
    }

    // fieldExpr := FIELD ":" ( rangeExpr | value ["*"] )
    std::unique_ptr<QueryNode> QueryParser::ParseFieldExpr()
    {
        if (Current().m_Type != TokenType::Word)
        {
            throw std::runtime_error("QueryParser: expected field name, got '" +
                                     Current().m_Value + "'");
        }

        std::string field = Current().m_Value;
        Advance();

        if (!Match(TokenType::Colon))
        {
            throw std::runtime_error("QueryParser: expected ':' after field '" + field + "'");
        }

        // Range: [ lo TO hi ] or { lo TO hi }
        if (Current().m_Type == TokenType::LBracket)
        {
            return ParseRangeExpr(field, true);
        }
        if (Current().m_Type == TokenType::LBrace)
        {
            return ParseRangeExpr(field, false);
        }

        // Simple value or wildcard
        if (Current().m_Type != TokenType::Word)
        {
            throw std::runtime_error("QueryParser: expected value after '" + field + ":'");
        }

        std::string value = Current().m_Value;
        Advance();

        // Check for wildcard suffix
        if (Current().m_Type == TokenType::Star)
        {
            Advance();
            auto node = std::make_unique<QueryNode>(QueryNodeType::FieldWildcard);
            node->SetField(field);
            node->SetValue(value);  // value is the prefix (without *)
            return node;
        }

        auto node = std::make_unique<QueryNode>(QueryNodeType::FieldMatch);
        node->SetField(field);
        node->SetValue(value);
        return node;
    }

    // rangeExpr := "[" value "TO" value "]" | "{" value "TO" value "}"
    std::unique_ptr<QueryNode> QueryParser::ParseRangeExpr(const std::string& field, bool inclusive)
    {
        // Consume opening bracket/brace
        Advance();

        if (Current().m_Type != TokenType::Word)
        {
            throw std::runtime_error("QueryParser: expected range low value");
        }
        std::string low = Current().m_Value;
        Advance();

        if (!Match(TokenType::To))
        {
            throw std::runtime_error("QueryParser: expected 'TO' in range");
        }

        if (Current().m_Type != TokenType::Word)
        {
            throw std::runtime_error("QueryParser: expected range high value");
        }
        std::string high = Current().m_Value;
        Advance();

        // Consume closing bracket/brace
        TokenType expected = inclusive ? TokenType::RBracket : TokenType::RBrace;
        if (!Match(expected))
        {
            throw std::runtime_error("QueryParser: expected closing bracket/brace in range");
        }

        auto node = std::make_unique<QueryNode>(QueryNodeType::FieldRange);
        node->SetField(field);
        node->SetRangeLow(low);
        node->SetRangeHigh(high);
        node->SetRangeInclusive(inclusive);
        return node;
    }

    // ================================================================
    // Token navigation helpers
    // ================================================================

    const Token& QueryParser::Current() const
    {
        return m_Tokens[m_Pos];
    }

    const Token& QueryParser::Advance()
    {
        if (m_Pos < m_Tokens.size() - 1)
        {
            ++m_Pos;
        }
        return m_Tokens[m_Pos];
    }

    bool QueryParser::Match(TokenType type)
    {
        if (Current().m_Type == type)
        {
            Advance();
            return true;
        }
        return false;
    }

    bool QueryParser::IsAtEnd() const
    {
        return Current().m_Type == TokenType::Eof;
    }

} // namespace PolarionMockup
