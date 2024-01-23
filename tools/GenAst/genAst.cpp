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

void defineAst(const std::string& outputDir, const std::string& baseName, const std::vector<std::string>& types);

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: GenAst <output directory>" << std::endl;
        exit(64);
    }

    std::string output_dir = argv[1];

    defineAst(output_dir, "Expr",
              {
                  "Binary   : Expr left, Token op, Expr right", // Binary class
                  "Grouping : Expr expression",                 // Grouping class
                  "Literal  : std::string value",               // Literal class
                  "Unary    : Token op, Expr right"             // Unary class
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
    outputFile << "#include \"Token.hpp\"" << std::endl;
    outputFile << std::endl;

    outputFile << "class " << baseName << "{};" << std::endl;

    for (const std::string& type : types)
    {
        size_t colonPos = type.find(":");
        std::string className = type.substr(0, colonPos);
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
            trim(field); // Assuming 'trim' is a function to trim whitespace
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

        // Fields
        for (const std::string& f : splitFields)
        {
            outputFile << "    const " << f << ";" << std::endl;
        }

        outputFile << "};" << std::endl;
        outputFile << std::endl;
    }

    outputFile.close();
}