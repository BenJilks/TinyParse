#include "generator/generator_cpp.hpp"
#include <string>
using std::string;
using namespace TinyParse;

string GeneratorCpp::find_type_name(string name)
{
    const Rule *rule = find_rule(name);
    printf("%s\n", name.c_str());
    if (rule != nullptr)
        return "Node" + name + "<Token, TokenType>*";
    
    return "Token";
}

void GeneratorCpp::generate_list(Rule::Node *node)
{
	for (auto child : node->children)
		generate_rule_node(child);
}

void GeneratorCpp::generate_keyword(Rule::Node *node)
{
    string name = node->value;
    if (has_define(name)) name = defines[name];

	write_line("lexer.match(TokenType::" + 
		name + ", \"" + node->value + "\");");
}

void GeneratorCpp::generate_token(Rule::Node *node)
{
    if (!node->has_label)
        generate_keyword(node);

    string match;
    string name = node->value;
    if (find_rule(name) != nullptr)
    {
        match = "new Node" + name + "<Token, TokenType>(lexer)";
    }
    else
    {
        match = "lexer.match(TokenType::" + name + 
		    ", \"" + node->label + "\")";
    }

    if (node->is_array)
        write_line(node->label + ".push_back(" + match + ");");
    else
        write_line(node->label + " = " + match + ";");
}

#define GEN_MOD(start, end) \
{ \
    Rule::Node *option = node->children[0]; \
    Rule::Node *entry = get_entry_node(node); \
    string is_next = "lexer.get_look().type == TokenType::" + \
        (has_define(entry->value) ? \
        defines[entry->value] : entry->value); \
     \
    write_line(start); \
    write_line("{"); \
    start_scope(); \
        generate_rule_node(option); \
    end_scope(); \
    write_line(string("}") + end); \
}

void GeneratorCpp::generate_optional(Rule::Node *node) GEN_MOD("if (" + is_next + ")", "")
void GeneratorCpp::generate_any(Rule::Node *node) GEN_MOD("while (" + is_next + ")", "")
void GeneratorCpp::generate_one_or_more(Rule::Node *node) GEN_MOD("do", " while (" + is_next + ");")

void GeneratorCpp::generate_rule_node(Rule::Node *node)
{
    printf("test: %i\n", node->type);
	switch (node->type)
	{
		case Rule::NodeList: generate_list(node); break;
		case Rule::NodeKeyword: generate_keyword(node); break;
		case Rule::NodeToken: generate_token(node); break;
        case Rule::NodeOptional: generate_optional(node); break;
        case Rule::NodeAny: generate_any(node); break;
        case Rule::NodeOneOrMore: generate_one_or_more(node); break;
		default: break; // Do error
	}
}

void GeneratorCpp::generate_rule_vars(Rule &rule)
{
    for (auto label : rule.get_labels())
    {
        string type_name = find_type_name(label->value);
        if (label->is_array)
            write_line("std::vector<" + type_name + "> " + label->label + ";");
        else
            write_line(type_name + " " + label->label + ";");
    }
}

void GeneratorCpp::generate_rule(Rule &rule)
{
    string name = rule.get_name();
    write_line("");
    write_line("template<typename Token, typename TokenType>");
    write_line("class Node" + name + " : public Node<Token, TokenType>");
    write_line("{");

    write_line("public:");
    start_scope();
        write_line("Node" + name + "(" +
            "TinyLib::LexerInterface<Token, TokenType> &lexer)");
        write_line("{");
        
        start_scope();
            generate_rule_node(rule.get_root());
        end_scope();

        write_line("}");
        for (auto label : rule.get_labels())
        {
            write_line("inline const auto &get_" + label->label + 
                "() const { return " + label->label + "; }");
        }
        write_line("");
    end_scope();

    write_line("private:");
    start_scope();
        generate_rule_vars(rule);
    end_scope();

    write_line("");
    write_line("};");
}

void GeneratorCpp::generate_definitions()
{
    for (auto rule : rules)
    {
        write_line("");
        write_line("template<typename Token, typename TokenType>");
        write_line("class Node" + rule.get_name() + ";");
    }
}

void GeneratorCpp::generate(string project_name)
{
    write_file("cpp/cpp_tinylib.hpp");

    write_line("#ifndef TINY_PARSE_HPP");
    write_line("#define TINY_PARSE_HPP\n");

    write_line("namespace TinyParse\n{");
    write_file("cpp/cpp_header.txt");

    start_scope();
        generate_definitions();

        // Write node classes for all rules
        for (auto rule : rules)
            generate_rule(rule);
    end_scope();

    write_line("}");
    write_line("");
    write_line("#endif");
}
