#include "generator/generator_cpp.hpp"
using namespace TinyParse;

void GeneratorCpp::generate_rule_parse(Rule *rule)
{
    
}

void GeneratorCpp::generate_rule(Rule *rule)
{
    string name = rule->get_name();
    write_line("\n\ttemplate<typename Token>");
    write_line("\tclass Node" + name + " : public Node<Token> \n\t{");

    write_line("\tpublic:");
    write_line("\t\tNode" + name + "(Node *parent, LexerInterface<Token> &lexer)\n\t\t{");
    generate_rule_parse(rule);
    write_line("\t\t}");
    for (const auto &label : rule->get_labels())
    {
        write_line("\t\tinline const auto &get_" + label.first + 
            "() const { return " + label.first + "; }");
    }

    write_line("\n\tprivate:");
    for (const auto &label : rule->get_labels())
        write_line("\t\tToken " + label.first + ";");

    write_line("\n\t};\n");
}

void GeneratorCpp::generate_definitions()
{
    for (auto rule : rules)
    {
        write_line("\n\ttemplate<typename Token>");
        write_line("\tclass Node" + rule->get_name() + ";");
    }
}

void GeneratorCpp::generate(string project_name)
{
    write_file("cpp/cpp_tinylib.hpp");

    write_line("#ifndef TINY_PARSE_HPP");
    write_line("#define TINY_PARSE_HPP\n");

    write_line("namespace TinyParse\n{");
    write_file("cpp/cpp_header.txt");
    generate_definitions();

    // Write node classes for all rules
    for (auto rule : rules)
        generate_rule(rule);

    write_line("}");
    write_line("\n#endif");
}
