#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

// Trim from start (in place)
static inline void ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

// Trim from end (in place)
static inline void rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

// Trim from both ends (in place)
static inline void trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}

// Trim from both ends (copying)
static inline std::string trim_copy(std::string s)
{
    trim(s);
    return s;
}

void defineVisitor(std::fstream& outputFile, const std::string& baseName, const std::vector<std::string>& types);
void defineAst(const std::string& outputDir, const std::string& baseName, const std::vector<std::string>& types);

int main(int argc, char** argv)
{
    std::string output_dir = "./";

    defineAst(output_dir, "Expr",
              {
                  "Assign   : Token name, std::shared_ptr<Expr<T>> value",                              // Assign class
                  "Binary   : std::shared_ptr<Expr<T>> left, Token op, std::shared_ptr<Expr<T>> right", // Binary class
                  "Call    : std::shared_ptr<Expr<T>> callee, Token paren, std::vector<std::shared_ptr<Expr<T>>> "
                  "arguments",                                      // Call class
                  "Grouping : std::shared_ptr<Expr<T>> expression", // Grouping class
                  "Literal  : LoxTypeRef value",                    // Literal class
                  "Logical  : std::shared_ptr<Expr<T>> left, Token op, std::shared_ptr<Expr<T>> right", // Logical class
                  "Unary    : Token op, std::shared_ptr<Expr<T>> right",                                // Unary class
                  "Variable : Token name" // Variable class
              });

    defineAst(output_dir, "Stmt",
              {
                  "Block      : std::vector<std::shared_ptr<Stmt<T>>> statements", // Block class
                  "Expression : std::shared_ptr<Expr<T>> expression",              // Expression class
                  "If         : std::shared_ptr<Expr<T>> condition, std::shared_ptr<Stmt<T>> thenBranch, "
                  "std::shared_ptr<Stmt<T>> elseBranch",                                           // If class
                  "Print      : std::shared_ptr<Expr<T>> expression",                              // Print class
                  "Var        : Token name, std::shared_ptr<Expr<T>> initializer",                 // Var class
                  "While      : std::shared_ptr<Expr<T>> condition, std::shared_ptr<Stmt<T>> body" // While class
              });

    return 0;
}

void defineAst(const std::string& outputDir, const std::string& baseName, const std::vector<std::string>& types)
{
    std::string path = outputDir + "/" + baseName + ".hpp";

    std::fstream outputFile;
    outputFile.open(path, std::ios::out);

    outputFile << "#pragma once" << std::endl;
    outputFile << "#include <string>" << std::endl;
    outputFile << "#include <vector>" << std::endl;
    outputFile << "#include <memory>" << std::endl << std::endl;

    outputFile << "#include \"LoxType.hpp\"" << std::endl;
    outputFile << "#include \"Token.hpp\"" << std::endl << std::endl;

    outputFile << "template <typename T>" << std::endl;
    outputFile << "class " << baseName << "Visitor;" << std::endl << std::endl;

    outputFile << "template <typename T>" << std::endl;
    outputFile << "class " << baseName << " {" << std::endl;

    outputFile << "public:" << std::endl;
    outputFile << "   virtual ~" << baseName << "() = default;" << std::endl;
    outputFile << "    virtual T accept(" << baseName << "Visitor<T>& visitor) const = 0;" << std::endl;
    outputFile << "};" << std::endl << std::endl;

    // forward declare sub types
    for (const std::string& type : types)
    {
        size_t colonPos = type.find(":");
        std::string className = type.substr(0, colonPos);
        trim(className);
        outputFile << "template<typename T>" << std::endl;
        outputFile << "    class " << className << baseName << ";" << std::endl << std::endl;
    }

    defineVisitor(outputFile, baseName, types);

    for (const std::string& type : types)
    {
        size_t colonPos = type.find(":");
        std::string className = type.substr(0, colonPos);
        trim(className);
        std::string fields = type.substr(colonPos + 1);

        outputFile << "template<typename T>" << std::endl;
        outputFile << "class " << className << baseName << " : public " << baseName << "<T>" << std::endl;
        outputFile << "{" << std::endl;
        outputFile << "public:" << std::endl;

        // Split fields
        std::istringstream iss(fields);
        std::vector<std::string> splitFields;
        std::string field;
        while (std::getline(iss, field, ','))
        {
            trim(field);
            splitFields.push_back(field);
        }

        // Constructor
        outputFile << "    " << className << baseName << "(";
        bool first = true;
        for (const std::string& f : splitFields)
        {
            if (!first)
                outputFile << ", ";
            outputFile << f;
            first = false;
        }
        outputFile << ")" << std::endl;

        // Constructor initialization list
        outputFile << "        : ";
        first = true;
        for (const std::string& f : splitFields)
        {
            if (!first)
                outputFile << ", ";

            size_t spacePos = f.find(" ");
            std::string fieldName = f.substr(spacePos + 1);

            // if there is a second space, extract the field name from it
            size_t secondSpacePos = fieldName.find(" ");
            if (secondSpacePos != std::string::npos)
            {
                fieldName = fieldName.substr(secondSpacePos + 1);
            }

            outputFile << fieldName << "(" << fieldName << ")";
            first = false;
        }
        outputFile << std::endl;
        outputFile << "    {" << std::endl;
        outputFile << "    }" << std::endl;

        // Visitor pattern
        outputFile << "    T accept(" << baseName << "Visitor<T>& visitor) const override" << std::endl;
        outputFile << "    {" << std::endl;
        outputFile << "        return visitor.visit" << className + baseName << "(*this);" << std::endl;
        outputFile << "    }" << std::endl;

        // Fields
        for (const std::string& f : splitFields)
        {
            outputFile << "    " << f << ";" << std::endl;
        }

        outputFile << "};" << std::endl;
        outputFile << std::endl;
    }

    outputFile.close();
}

void defineVisitor(std::fstream& outputFile, const std::string& baseName, const std::vector<std::string>& types)
{
    outputFile << "template <typename T>" << std::endl;
    outputFile << "class " << baseName << "Visitor" << std::endl;
    outputFile << "{" << std::endl;
    outputFile << "public:" << std::endl;

    outputFile << "    virtual ~" << baseName << "Visitor() = default;" << std::endl;

    for (const std::string& type : types)
    {
        size_t colonPos = type.find(":");
        std::string typeName = type.substr(0, colonPos);
        trim(typeName);
        std::string lowerBaseName = baseName;
        std::transform(lowerBaseName.begin(), lowerBaseName.end(), lowerBaseName.begin(), ::tolower);

        outputFile << "    virtual T visit" << typeName << baseName << "(const " << typeName << baseName << "<T>& "
                   << lowerBaseName << ") = 0;" << std::endl;
        outputFile << std::endl;
    }

    outputFile << "};" << std::endl;
}