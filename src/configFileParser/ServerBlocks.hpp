#pragma once

#include "TokenizeFile.hpp"

#include <vector>

typedef struct s_directive
{
    t_token directive;
    std::vector<t_token> options;
}   t_directive;

typedef struct s_location_block
{
    t_token name;
    bool exact;
    std::vector<t_directive> directives;
}   t_location_block;

typedef struct s_server_block
{
    std::vector<t_directive> directives;
    std::vector<t_location_block> locations;
}   t_server_block;

class ServerBlocks
{
public:
    ServerBlocks(const TokenizeFile& tokenizedFile);
    const std::vector<t_server_block>& getServerBlocks() const;
private:
    std::string filePath;
    std::vector<t_server_block> serverBlocks;

    std::vector<std::vector<t_token> > getRawServerBlocks(const std::vector<t_token>& tokens) const;
    void locationBlock(size_t& j, size_t& i, std::vector<std::vector<t_token> >& rawServerBlocks, t_server_block& serverBlock);
};