#include "generator/generator.hpp"
#include <iostream>
#include <unistd.h>
using namespace TinyParse;

Generator::Generator() :
    out(std::cout) {}

Generator::Generator(string file_path) :
    out(out_file)
{
    out_file = std::ofstream(file_path);
}

Rule::Node *Generator::get_entry_node(Rule::Node *node)
{
    switch (node->type)
    {
        case Rule::NodeKeyword: return node;
        case Rule::NodeToken: 
        {
            const Rule *rule = find_rule(node->value);
            if (rule != nullptr)
                return get_entry_node(rule->get_root());
            return node;
        }
    }
    
    if (node->children.size() > 0)
        return get_entry_node(node->children[0]);
    
    return nullptr;
}

void Generator::add_define(string name, string value)
{
    defines[value] = name;
}

void Generator::add_rule(Rule rule)
{
    rules.push_back(rule);
}

void Generator::write_string(string msg)
{
    out << msg;
}

void Generator::write_line(string line)
{
    for (int i = 0; i < scope; i++)
        out << "\t";
    out << line << std::endl;
}

bool Generator::has_define(string value)
{
    return defines.find(value) != defines.end();
}

const Rule *Generator::find_rule(string name) const
{
    for (int i = 0; i < rules.size(); i++)
        if (rules[i].get_name() == name)
            return &rules[i];
    
    return nullptr;
}

static bool file_exists(string name)
{
    return access(name.c_str(), F_OK) != -1;
}

static const char *search_paths[] =
{
    "",
    "../",
    "/include/",
    "/usr/include/",
    "/usr/local/include/",
};

void Generator::write_file(string file_path)
{
    // Find template file path
    string file = "tinyparse/" + file_path;
    string path = "";
    bool found = false;
    for (int i = 0; i < sizeof(search_paths) / sizeof(char *); i++)
    {
        if (access((string(search_paths[i]) + file).c_str(), F_OK) != -1)
        {
            path = search_paths[i];
            found = true;
            break;
        }
    }

    if (!found)
    {
        std::cout << "Error: Could not find template file '" << 
            file_path << "'" << std::endl;
        return;
    }

    // Open file and find its length
    std::ifstream in(path + file);
    in.seekg(0L, std::ifstream::end);
    int len = in.tellg();
    in.seekg(0L, std::ifstream::beg);

    // Read data to buffer
    vector<char> data(len);
    in.read(&data[0], len);
    in.close();

    // Write it to the out file
    out.write(&data[0], data.size());
}

Generator::~Generator()
{
    if (out_file.is_open())
        out_file.close();
}
