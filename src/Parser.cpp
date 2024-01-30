#include "Parser.hpp"

ParseException::ParseException(const std::string& message) : message(message)
{
}

const char* ParseException::what() const noexcept
{
    return message.c_str();
}

Parser::Parser(const std::vector<Token>& tokens, ILogger& logger) : tokens(tokens), logger(logger)
{
}

std::vector<std::shared_ptr<Stmt<LoxType>>> Parser::parse()
{
    std::vector<std::shared_ptr<Stmt<LoxType>>> statements;

    while (!isAtEnd())
    {
        statements.push_back(declaration());
    }

    return statements;
}

bool Parser::match(std::vector<TokenType> types)
{
    for (TokenType type : types)
    {
        if (check(type))
        {
            advance();
            return true;
        }
    }

    return false;
}

bool Parser::check(TokenType type)
{
    if (isAtEnd())
    {
        return false;
    }

    return peek().type == type;
}

Token Parser::advance()
{
    if (!isAtEnd())
    {
        current++;
    }

    return previous();
}

bool Parser::isAtEnd()
{
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::peek()
{
    return tokens[current];
}

Token Parser::previous()
{
    return tokens[current - 1];
}

std::shared_ptr<Stmt<LoxType>> Parser::declaration()
{
    try
    {
        if (match({TokenType::VAR}))
        {
            return varDeclaration();
        }

        return statement();
    }
    catch (ParseException& error)
    {
        synchronize();
        return nullptr;
    }
}

std::shared_ptr<Stmt<LoxType>> Parser::varDeclaration()
{
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

    std::shared_ptr<Expr<LoxType>> initializer = nullptr;
    if (match({TokenType::EQUAL}))
    {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_shared<VarStmt<LoxType>>(name, initializer);
}

std::shared_ptr<Stmt<LoxType>> Parser::statement()
{
    if (match({TokenType::FOR}))
    {
        return forStatement();
    }

    if (match({TokenType::IF}))
    {
        return ifStatement();
    }

    if (match({TokenType::PRINT}))
    {
        return printStatement();
    }

    if (match({TokenType::WHILE}))
    {
        return whileStatement();
    }

    if (match({TokenType::LEFT_BRACE}))
    {
        return block();
    }

    return expressionStatement();
}

std::shared_ptr<Stmt<LoxType>> Parser::ifStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    std::shared_ptr<Expr<LoxType>> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

    std::shared_ptr<Stmt<LoxType>> thenBranch = statement();
    std::shared_ptr<Stmt<LoxType>> elseBranch = nullptr;

    if (match({TokenType::ELSE}))
    {
        elseBranch = statement();
    }

    return std::make_shared<IfStmt<LoxType>>(condition, thenBranch, elseBranch);
}

std::shared_ptr<Stmt<LoxType>> Parser::whileStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    std::shared_ptr<Expr<LoxType>> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    std::shared_ptr<Stmt<LoxType>> body = statement();

    return std::make_shared<WhileStmt<LoxType>>(condition, body);
}

std::shared_ptr<Stmt<LoxType>> Parser::forStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    std::shared_ptr<Stmt<LoxType>> initializer;
    if (match({TokenType::SEMICOLON}))
    {
        initializer = nullptr;
    }
    else if (match({TokenType::VAR}))
    {
        initializer = varDeclaration();
    }
    else
    {
        initializer = expressionStatement();
    }

    std::shared_ptr<Expr<LoxType>> condition = nullptr;
    if (!check(TokenType::SEMICOLON))
    {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    std::shared_ptr<Expr<LoxType>> increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN))
    {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    std::shared_ptr<Stmt<LoxType>> body = statement();

    if (increment != nullptr)
    {
        std::vector<std::shared_ptr<Stmt<LoxType>>> statements;
        statements.push_back(body);
        statements.push_back(std::make_shared<ExpressionStmt<LoxType>>(increment));

        body = std::make_shared<BlockStmt<LoxType>>(statements);
    }

    if (condition == nullptr)
    {
        condition = std::make_shared<LiteralExpr<LoxType>>(true);
    }
    body = std::make_shared<WhileStmt<LoxType>>(condition, body);

    if (initializer != nullptr)
    {
        std::vector<std::shared_ptr<Stmt<LoxType>>> statements;
        statements.push_back(initializer);
        statements.push_back(body);

        body = std::make_shared<BlockStmt<LoxType>>(statements);
    }

    return body;
}

std::shared_ptr<Stmt<LoxType>> Parser::block()
{
    std::vector<std::shared_ptr<Stmt<LoxType>>> statements;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
    {
        statements.push_back(declaration());
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return std::make_shared<BlockStmt<LoxType>>(statements);
}

std::shared_ptr<Stmt<LoxType>> Parser::printStatement()
{
    std::shared_ptr<Expr<LoxType>> value = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_shared<PrintStmt<LoxType>>(value);
}

std::shared_ptr<Stmt<LoxType>> Parser::expressionStatement()
{
    std::shared_ptr<Expr<LoxType>> expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_shared<ExpressionStmt<LoxType>>(expr);
}

std::shared_ptr<Expr<LoxType>> Parser::expression()
{
    return assignment();
}

std::shared_ptr<Expr<LoxType>> Parser::assignment()
{
    std::shared_ptr<Expr<LoxType>> expr = orExpr();

    if (match({TokenType::EQUAL}))
    {
        Token equals = previous();
        std::shared_ptr<Expr<LoxType>> value = assignment();

        if (auto variable = std::dynamic_pointer_cast<VariableExpr<LoxType>>(expr))
        {
            Token name = variable->name;
            return std::make_shared<AssignExpr<LoxType>>(name, value);
        }

        error(equals, "Invalid assignment target.");
    }

    return expr;
}

std::shared_ptr<Expr<LoxType>> Parser::orExpr()
{
    std::shared_ptr<Expr<LoxType>> expr = andExpr();

    while (match({TokenType::OR}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxType>> right = andExpr();
        expr = std::make_shared<LogicalExpr<LoxType>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<LoxType>> Parser::andExpr()
{
    std::shared_ptr<Expr<LoxType>> expr = equality();

    while (match({TokenType::AND}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxType>> right = equality();
        expr = std::make_shared<LogicalExpr<LoxType>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<LoxType>> Parser::equality()
{
    std::shared_ptr<Expr<LoxType>> expr = comparison();

    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxType>> right = comparison();
        expr = std::make_shared<BinaryExpr<LoxType>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<LoxType>> Parser::comparison()
{
    std::shared_ptr<Expr<LoxType>> expr = term();

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxType>> right = term();
        expr = std::make_shared<BinaryExpr<LoxType>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<LoxType>> Parser::term()
{
    std::shared_ptr<Expr<LoxType>> expr = factor();

    while (match({TokenType::MINUS, TokenType::PLUS}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxType>> right = factor();
        expr = std::make_shared<BinaryExpr<LoxType>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<LoxType>> Parser::factor()
{
    std::shared_ptr<Expr<LoxType>> expr = unary();

    while (match({TokenType::SLASH, TokenType::STAR}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxType>> right = unary();
        expr = std::make_shared<BinaryExpr<LoxType>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<LoxType>> Parser::unary()
{
    if (match({TokenType::BANG, TokenType::MINUS}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxType>> right = unary();
        return std::make_shared<UnaryExpr<LoxType>>(op, right);
    }

    return primary();
}

std::shared_ptr<Expr<LoxType>> Parser::primary()
{
    if (match({TokenType::FALSE}))
        return std::make_shared<LiteralExpr<LoxType>>(false);

    if (match({TokenType::TRUE}))
        return std::make_shared<LiteralExpr<LoxType>>(true);

    if (match({TokenType::NIL}))
        return std::make_shared<LiteralExpr<LoxType>>(std::nullopt);

    if (match({TokenType::NUMBER, TokenType::STRING}))
        return std::make_shared<LiteralExpr<LoxType>>(previous().literal);

    if (match({TokenType::IDENTIFIER}))
        return std::make_shared<VariableExpr<LoxType>>(previous());

    if (match({TokenType::LEFT_PAREN}))
    {
        std::shared_ptr<Expr<LoxType>> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_shared<GroupingExpr<LoxType>>(expr);
    }

    throw error(peek(), "Expect expression.");
}

Token Parser::consume(TokenType type, const std::string& message)
{
    if (check(type))
    {
        return advance();
    }

    throw error(peek(), message);
}

ParseException Parser::error(Token token, const std::string& message)
{
    if (token.type == TokenType::END_OF_FILE)
    {
        logger.LogError(token.line, " at end", message);
    }
    else
    {
        logger.LogError(token.line, " at '" + token.lexeme + "'", message);
    }

    return ParseException(message);
}

void Parser::synchronize()
{
    advance();

    while (!isAtEnd())
    {
        if (previous().type == TokenType::SEMICOLON)
            return;

        switch (peek().type)
        {
        case TokenType::CLASS:
        case TokenType::FUN:
        case TokenType::VAR:
        case TokenType::FOR:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::PRINT:
        case TokenType::RETURN:
            return;
        }

        advance();
    }
}
