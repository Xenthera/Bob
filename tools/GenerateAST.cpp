//
// Created by Bobby Lucero on 5/21/23.
//
#include <iostream>
#include <vector>
#include <fstream>
#include "../headers/helperFunctions/HelperFunctions.h"

void defineType(std::ofstream &out, std::string baseName, std::string className, std::string fieldList)
{
    out << "template <typename T>\n";
    out << "struct " << className << "Expr : " << baseName << "<T>" << ", Visitor<T>" << "\n{\n";
    std::vector<std::string> fields = splitString(fieldList, ", ");
    for(std::string field : fields)
    {
        std::cout << trim(field) << std::endl;
        std::string fieldClass = splitString(trim(field), " ")[0];
        std::string fieldName = splitString(trim(field), " ")[1];
        fieldClass = trim(fieldClass) == "Expr" ? "Expr<T>" : trim(fieldClass);
        out << "    const " << fieldClass << " " << fieldName << ";\n";
    }
    fieldList = replaceSubstring(fieldList, "Expr", "Expr<T>");
    out << "\n    " << className << "Expr(" << fieldList << ") : ";

    std::string explicitDeclaration;
    for(std::string field : fields)
    {
        std::cout << trim(field) << std::endl;
        std::string fieldClass = splitString(trim(field), " ")[0];
        std::string fieldName = splitString(trim(field), " ")[1];
        fieldClass = trim(fieldClass) == "Expr" ? "Expr<T>" : trim(fieldClass);

        explicitDeclaration += trim(fieldName) + "(" + trim(fieldName) + "), ";
    }
    explicitDeclaration = trim(explicitDeclaration);
    explicitDeclaration.pop_back();

    out << explicitDeclaration;

    out << "\n    {\n";
    out << "    }\n";

    out << "    T accept(Visitor<T> visitor){\n";
    out << "        return visitor.visit" << className << "Expr(this);\n";
    out << "    }\n";

    out << "};" << std::endl;
}

void defineAst(std::string outputDir, std::string baseName, const std::vector<std::string>& types)
{
    std::ofstream outFile(outputDir);

    if(outFile.is_open())
    {
        for(std::string type : types)
        {
            std::vector<std::string> type_split = splitString(type, "$");
            std::string className = trim(type_split[0]);
            std::string fields = trim(type_split[1]);
            defineType(outFile, baseName, className, fields);
        }
    }
}



int main(int argc, char* argv[]){
    if(argc != 2)
    {
        std::cerr << "Usage " << argv[0] << " <output directory>" << std::endl;
        std::exit(64);
    }
    std::string outputDir = argv[1];

    defineAst(outputDir, "Expr", {
            "Binary   $ Expr left, Token oper, Expr right",
            "Grouping $ Expr expression",
            "Literal  $ std::string value",
            "Unary    $ Token oper, Expr right"
    });
}



