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

std::vector<std::shared_ptr<Stmt<LoxTypeRef>>> Parser::parse()
{
    std::vector<std::shared_ptr<Stmt<LoxTypeRef>>> statements;

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

std::shared_ptr<Stmt<LoxTypeRef>> Parser::declaration()
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

std::shared_ptr<Stmt<LoxTypeRef>> Parser::varDeclaration()
{
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

    std::shared_ptr<Expr<LoxTypeRef>> initializer = nullptr;
    if (match({TokenType::EQUAL}))
    {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_shared<VarStmt<LoxTypeRef>>(name, initializer);
}

std::shared_ptr<Stmt<LoxTypeRef>> Parser::statement()
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

std::shared_ptr<Stmt<LoxTypeRef>> Parser::ifStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    std::shared_ptr<Expr<LoxTypeRef>> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

    std::shared_ptr<Stmt<LoxTypeRef>> thenBranch = statement();
    std::shared_ptr<Stmt<LoxTypeRef>> elseBranch = nullptr;

    if (match({TokenType::ELSE}))
    {
        elseBranch = statement();
    }

    return std::make_shared<IfStmt<LoxTypeRef>>(condition, thenBranch, elseBranch);
}

std::shared_ptr<Stmt<LoxTypeRef>> Parser::whileStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    std::shared_ptr<Expr<LoxTypeRef>> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    std::shared_ptr<Stmt<LoxTypeRef>> body = statement();

    return std::make_shared<WhileStmt<LoxTypeRef>>(condition, body);
}

std::shared_ptr<Stmt<LoxTypeRef>> Parser::forStatement()
{
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    std::shared_ptr<Stmt<LoxTypeRef>> initializer;
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

    std::shared_ptr<Expr<LoxTypeRef>> condition = nullptr;
    if (!check(TokenType::SEMICOLON))
    {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    std::shared_ptr<Expr<LoxTypeRef>> increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN))
    {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    std::shared_ptr<Stmt<LoxTypeRef>> body = statement();

    if (increment != nullptr)
    {
        std::vector<std::shared_ptr<Stmt<LoxTypeRef>>> statements;
        statements.push_back(body);
        statements.push_back(std::make_shared<ExpressionStmt<LoxTypeRef>>(increment));

        body = std::make_shared<BlockStmt<LoxTypeRef>>(statements);
    }

    if (condition == nullptr)
    {
        condition = std::make_shared<LiteralExpr<LoxTypeRef>>(std::make_shared<LoxType>(true));
    }
    body = std::make_shared<WhileStmt<LoxTypeRef>>(condition, body);

    if (initializer != nullptr)
    {
        std::vector<std::shared_ptr<Stmt<LoxTypeRef>>> statements;
        statements.push_back(initializer);
        statements.push_back(body);

        body = std::make_shared<BlockStmt<LoxTypeRef>>(statements);
    }

    return body;
}

std::shared_ptr<Stmt<LoxTypeRef>> Parser::block()
{
    std::vector<std::shared_ptr<Stmt<LoxTypeRef>>> statements;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd())
    {
        statements.push_back(declaration());
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return std::make_shared<BlockStmt<LoxTypeRef>>(statements);
}

std::shared_ptr<Stmt<LoxTypeRef>> Parser::printStatement()
{
    std::shared_ptr<Expr<LoxTypeRef>> value = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_shared<PrintStmt<LoxTypeRef>>(value);
}

std::shared_ptr<Stmt<LoxTypeRef>> Parser::expressionStatement()
{
    std::shared_ptr<Expr<LoxTypeRef>> expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_shared<ExpressionStmt<LoxTypeRef>>(expr);
}

std::shared_ptr<Expr<LoxTypeRef>> Parser::expression()
{
    return assignment();
}

std::shared_ptr<Expr<LoxTypeRef>> Parser::assignment()
{
    std::shared_ptr<Expr<LoxTypeRef>> expr = orExpr();

    if (match({TokenType::EQUAL}))
    {
        Token equals = previous();
        std::shared_ptr<Expr<LoxTypeRef>> value = assignment();

        if (auto variable = std::dynamic_pointer_cast<VariableExpr<LoxTypeRef>>(expr))
        {
            Token name = variable->name;
            return std::make_shared<AssignExpr<LoxTypeRef>>(name, value);
        }

        error(equals, "Invalid assignment target.");
    }

    return expr;
}

std::shared_ptr<Expr<LoxTypeRef>> Parser::orExpr()
{
    std::shared_ptr<Expr<LoxTypeRef>> expr = andExpr();

    while (match({TokenType::OR}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxTypeRef>> right = andExpr();
        expr = std::make_shared<LogicalExpr<LoxTypeRef>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<LoxTypeRef>> Parser::andExpr()
{
    std::shared_ptr<Expr<LoxTypeRef>> expr = equality();

    while (match({TokenType::AND}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxTypeRef>> right = equality();
        expr = std::make_shared<LogicalExpr<LoxTypeRef>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<LoxTypeRef>> Parser::equality()
{
    std::shared_ptr<Expr<LoxTypeRef>> expr = comparison();

    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxTypeRef>> right = comparison();
        expr = std::make_shared<BinaryExpr<LoxTypeRef>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<LoxTypeRef>> Parser::comparison()
{
    std::shared_ptr<Expr<LoxTypeRef>> expr = term();

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxTypeRef>> right = term();
        expr = std::make_shared<BinaryExpr<LoxTypeRef>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<LoxTypeRef>> Parser::term()
{
    std::shared_ptr<Expr<LoxTypeRef>> expr = factor();

    while (match({TokenType::MINUS, TokenType::PLUS}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxTypeRef>> right = factor();
        expr = std::make_shared<BinaryExpr<LoxTypeRef>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<LoxTypeRef>> Parser::factor()
{
    std::shared_ptr<Expr<LoxTypeRef>> expr = unary();

    while (match({TokenType::SLASH, TokenType::STAR}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxTypeRef>> right = unary();
        expr = std::make_shared<BinaryExpr<LoxTypeRef>>(expr, op, right);
    }

    return expr;
}

std::shared_ptr<Expr<LoxTypeRef>> Parser::unary()
{
    if (match({TokenType::BANG, TokenType::MINUS}))
    {
        Token op = previous();
        std::shared_ptr<Expr<LoxTypeRef>> right = unary();
        return std::make_shared<UnaryExpr<LoxTypeRef>>(op, right);
    }

    return call();
}

std::shared_ptr<Expr<LoxTypeRef>> Parser::call()
{
    std::shared_ptr<Expr<LoxTypeRef>> expr = primary();

    while (true)
    {
        if (match({TokenType::LEFT_PAREN}))
        {
            expr = finishCall(expr);
        }
        else
        {
            break;
        }
    }

    return expr;
}

std::shared_ptr<Expr<LoxTypeRef>> Parser::finishCall(std::shared_ptr<Expr<LoxTypeRef>> callee)
{
    std::vector<std::shared_ptr<Expr<LoxTypeRef>>> arguments;
    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            if (arguments.size() >= 255)
            {
                error(peek(), "Can't have more than 255 arguments.");
            }
            arguments.push_back(expression());
        } while (match({TokenType::COMMA}));
    }

    Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");

    return std::make_shared<CallExpr<LoxTypeRef>>(callee, paren, arguments);
}

std::shared_ptr<Expr<LoxTypeRef>> Parser::primary()
{
    if (match({TokenType::FALSE}))
        return std::make_shared<LiteralExpr<LoxTypeRef>>(std::make_shared<LoxType>(false));

    if (match({TokenType::TRUE}))
        return std::make_shared<LiteralExpr<LoxTypeRef>>(std::make_shared<LoxType>(true));

    if (match({TokenType::NIL}))
        return std::make_shared<LiteralExpr<LoxTypeRef>>(std::make_shared<LoxType>(std::nullopt));

    if (match({TokenType::NUMBER, TokenType::STRING}))
        return std::make_shared<LiteralExpr<LoxTypeRef>>(std::make_shared<LoxType>(previous().literal));

    if (match({TokenType::IDENTIFIER}))
        return std::make_shared<VariableExpr<LoxTypeRef>>(previous());

    if (match({TokenType::LEFT_PAREN}))
    {
        std::shared_ptr<Expr<LoxTypeRef>> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_shared<GroupingExpr<LoxTypeRef>>(expr);
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
