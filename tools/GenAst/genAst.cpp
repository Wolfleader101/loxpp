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
                  "Binary   : Expr& left, Token op, Expr& right", // Binary class
                  "Grouping : Expr& expression",                  // Grouping class
                  "Literal  : std::string value",                 // Literal class
                  "Unary    : Token op, Expr& right"              // Unary class
              });

    return 0;
}

void defineAst(const std::string& outputDir, const std::string& baseName, const std::vector<std::string>& types)
{

    std::string path = outputDir + "/" + baseName + ".hpp";

    std::fstream outputFile;
    outputFile.open(path, std::ios::out);

    outputFile << "#include <string>" << std::endl;
    outputFile << "#include <vector>" << std::endl;
    outputFile << std::endl;
    outputFile << "#include \"Token.hpp\"" << std::endl << std::endl;

    outputFile << "template <typename T>" << std::endl;
    outputFile << "class Visitor;" << std::endl << std::endl;

    outputFile << "template <typename T>" << std::endl;
    outputFile << "class " << baseName << " {" << std::endl;

    outputFile << "public:" << std::endl;

    // forward declare sub types
    for (const std::string& type : types)
    {
        size_t colonPos = type.find(":");
        std::string className = type.substr(0, colonPos);
        trim(className);
        outputFile << "    class " << className << ";" << std::endl;
    }

    defineVisitor(outputFile, baseName, types);

    outputFile << "    virtual T accept(Visitor& visitor) const = 0;" << std::endl;

    for (const std::string& type : types)
    {
        size_t colonPos = type.find(":");
        std::string className = type.substr(0, colonPos);
        trim(className);
        std::string fields = type.substr(colonPos + 1);

        outputFile << "class " << className << " : public " << baseName << std::endl;
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
        outputFile << "    " << className << "(";
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
            outputFile << fieldName << "(" << fieldName << ")";
            first = false;
        }
        outputFile << std::endl;
        outputFile << "    {" << std::endl;
        outputFile << "    }" << std::endl;

        // Visitor pattern
        outputFile << "    T accept(Visitor& visitor) const override" << std::endl;
        outputFile << "    {" << std::endl;
        outputFile << "        return visitor.visit" << className + baseName << "(*this);" << std::endl;
        outputFile << "    }" << std::endl;

        // Fields
        for (const std::string& f : splitFields)
        {
            outputFile << "    const " << f << ";" << std::endl;
        }

        outputFile << "};" << std::endl;
        outputFile << std::endl;
    }

    outputFile << "};" << std::endl << std::endl;

    outputFile.close();
}

void defineVisitor(std::fstream& outputFile, const std::string& baseName, const std::vector<std::string>& types)
{
    outputFile << "class Visitor" << std::endl;
    outputFile << "{" << std::endl;
    outputFile << "public:" << std::endl;

    for (const std::string& type : types)
    {
        size_t colonPos = type.find(":");
        std::string typeName = type.substr(0, colonPos);
        trim(typeName);
        std::string lowerBaseName = baseName;
        std::transform(lowerBaseName.begin(), lowerBaseName.end(), lowerBaseName.begin(), ::tolower);

        outputFile << "    virtual T visit" << typeName << baseName << "(" << typeName << "& " << lowerBaseName
                   << ") = 0;" << std::endl;
        outputFile << std::endl;
    }

    outputFile << "};" << std::endl;
}