#include <fstream>
#include <iostream>
#include <string>
#include <vector>

void runCode(std::string& code);

void runFile(const char* path);
void runPrompt();

void error(int line, const std::string& message);
void reportError(int line, const std::string& where, const std::string& message);

enum class TokenType {
    // single char tokens
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,

    // single or two char tokens
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,

    // literals
    IDENTIFIER,
    STRING,
    NUMBER,

    // keywords
    AND,
    CLASS,
    ELSE,
    FALSE,
    FUN,
    FOR,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,

    END_OF_FILE
};

constexpr const char* TokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::LEFT_PAREN:
            return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN:
            return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE:
            return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE:
            return "RIGHT_BRACE";
        case TokenType::COMMA:
            return "COMMA";
        case TokenType::DOT:
            return "DOT";
        case TokenType::MINUS:
            return "MINUS";
        case TokenType::PLUS:
            return "PLUS";
        case TokenType::SEMICOLON:
            return "SEMICOLON";
        case TokenType::SLASH:
            return "SLASH";
        case TokenType::STAR:
            return "STAR";
        case TokenType::BANG:
            return "BANG";
        case TokenType::BANG_EQUAL:
            return "BANG_EQUAL";
        case TokenType::EQUAL:
            return "EQUAL";
        case TokenType::EQUAL_EQUAL:
            return "EQUAL_EQUAL";
        case TokenType::GREATER:
            return "GREATER";
        case TokenType::GREATER_EQUAL:
            return "GREATER_EQUAL";
        case TokenType::LESS:
            return "LESS";
        case TokenType::LESS_EQUAL:
            return "LESS_EQUAL";
        case TokenType::IDENTIFIER:
            return "IDENTIFIER";
        case TokenType::STRING:
            return "STRING";
        case TokenType::NUMBER:
            return "NUMBER";
        case TokenType::AND:
            return "AND";
        case TokenType::CLASS:
            return "CLASS";
        case TokenType::ELSE:
            return "ELSE";
        case TokenType::FALSE:
            return "FALSE";
        case TokenType::FUN:
            return "FUN";
        case TokenType::FOR:
            return "FOR";
        case TokenType::IF:
            return "IF";
        case TokenType::NIL:
            return "NIL";
        case TokenType::OR:
            return "OR";
        case TokenType::PRINT:
            return "PRINT";
        case TokenType::RETURN:
            return "RETURN";
        case TokenType::SUPER:
            return "SUPER";
        case TokenType::THIS:
            return "THIS";
        case TokenType::TRUE:
            return "TRUE";
        case TokenType::VAR:
            return "VAR";
        case TokenType::WHILE:
            return "WHILE";
        case TokenType::END_OF_FILE:
            return "END_OF_FILE";
    }
};

class Token {
   public:
    Token(TokenType type, const std::string& lexeme, const std::string& literal, int line)
        : type(type), lexeme(lexeme), literal(literal), line(line) {}

    std::string toString() { return std::string(TokenTypeToString(type)) + " " + lexeme + " " + literal; }

   private:
    TokenType type;
    std::string lexeme;
    std::string literal;
    int line;
};

class Scanner {
   public:
    Scanner(const std::string& source) : source(source) {}

    std::vector<Token> scanTokens() {
        while (!isAtEnd()) {
            // We are at the beginning of the next lexeme.
            start = current;
            scanToken();
        }

        tokens.push_back(Token(TokenType::END_OF_FILE, "", "", line));
        return tokens;
    }

   private:
    std::string source;
    std::vector<Token> tokens;
    int start = 0;
    int current = 0;
    int line = 1;

    void scanToken() {
        char c = advance();
        switch (c) {
            case '(':
                addToken(TokenType::LEFT_PAREN);
                break;
            case ')':
                addToken(TokenType::RIGHT_PAREN);
                break;
            case '{':
                addToken(TokenType::LEFT_BRACE);
                break;
            case '}':
                addToken(TokenType::RIGHT_BRACE);
                break;
            case ',':
                addToken(TokenType::COMMA);
                break;
            case '.':
                addToken(TokenType::DOT);
                break;
            case '-':
                addToken(TokenType::MINUS);
                break;
            case '+':
                addToken(TokenType::PLUS);
                break;
            case ';':
                addToken(TokenType::SEMICOLON);
                break;
            case '*':
                addToken(TokenType::STAR);
                break;
            case '!':
                addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
                break;
            case '=':
                addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
                break;
            case '<':
                addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
                break;
            case '>':
                addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
                break;
            case '/':
                if (match('/')) {
                    // A comment goes until the end of the line.
                    while (peek() != '\n' && !isAtEnd()) {
                        advance();
                    }
                } else {
                    addToken(TokenType::SLASH);
                }
                break;
            case ' ':
            case '\r':
            case '\t':
                // Ignore whitespace.
                break;
            case '\n':
                line++;
                break;
            case '"':
                readString();
                break;
            default:
                error(line, "Unexpected character.");
                break;
        }
    }

    bool isAtEnd() { return current >= source.length(); }

    char advance() {
        current++;
        return source[current - 1];
    }

    void addToken(TokenType type) { addToken(type, ""); }

    void addToken(TokenType type, const std::string& literal) {
        std::string text = source.substr(start, current - start);
        tokens.push_back(Token(type, text, literal, line));
    }

    bool match(char expected) {
        if (isAtEnd()) {
            return false;
        }

        if (source[current] != expected) {
            return false;
        }

        current++;
        return true;
    }

    char peek() {
        if (isAtEnd()) {
            return '\0';
        }

        return source[current];
    }

    void readString() {
        while (peek() != '"' && !isAtEnd()) {
            if (peek() == '\n') line++;
            advance();
        }

        if (isAtEnd()) {
            error(line, "Unterminated string.");
            return;
        }

        // The closing ".
        advance();

        // trim the srounding quotes
        std::string value = source.substr(start + 1, current - start - 2);
        addToken(TokenType::STRING, value);
    }
};

static bool hadError = false;

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cout << "Usage: loxpp [script]" << std::endl;
        exit(64);
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        runPrompt();
    }

    return 0;
}

void runFile(const char* path) {
    std::ifstream file_stream(path);

    if (file_stream.is_open()) {
        std::string file_contents((std::istreambuf_iterator<char>(file_stream)), (std::istreambuf_iterator<char>()));
        file_stream.close();

        runCode(file_contents);

        if (hadError) {
            exit(65);
        }

    } else {
        std::cerr << "Error: Unable to open the file." << std::endl;
    }
}

void runPrompt() {
    for (;;) {
        std::cout << "> ";
        std::string line;
        std::getline(std::cin, line);

        if (std::cin.eof()) {
            std::cout << std::endl;
            break;
        }

        runCode(line);
        hadError = false;
    }
}

void runCode(std::string& code) {
    Scanner scanner = Scanner(code);
    std::vector<Token> tokens = scanner.scanTokens();

    for (Token token : tokens) {
        std::cout << token.toString() << std::endl;
    }
}

void error(int line, const std::string& message) { reportError(line, "", message); }

void reportError(int line, const std::string& where, const std::string& message) {
    hadError = true;
    std::cerr << "[line " << line << "] Error" << where << ": " << message << std::endl;
}