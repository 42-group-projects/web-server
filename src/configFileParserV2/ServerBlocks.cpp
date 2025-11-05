#include "ServerBlocks.hpp"
#include "parser_utils.hpp"

ServerBlocks::ServerBlocks(const TokenizeFile& tokenizedFile) : filePath(tokenizedFile.getFilePath())
{
	std::vector<std::vector<t_token> > rawServerBlocks = getRawServerBlocks(tokenizedFile.getTokens());
    
	for (size_t i = 0; i < rawServerBlocks.size(); i++)
	{
		t_server_block serverBlock;
		for (size_t j = 0; j < rawServerBlocks[i].size(); j++)
		{
			if (rawServerBlocks[i][j].str == "{" || rawServerBlocks[i][j].str == ";")
				parser_utils::unexpected(rawServerBlocks[i][j].str, rawServerBlocks[i][j], filePath);

			else if (rawServerBlocks[i][j].str == "location")
			{
				locationBlock(j, i, rawServerBlocks, serverBlock);
			}
			else
			{
				t_directive directive;
				directive.directive = rawServerBlocks[i][j];
				j++;
				while (j < rawServerBlocks[i].size() && rawServerBlocks[i][j].str != ";")
				{
					directive.options.push_back(rawServerBlocks[i][j]);
					j++;
				}
				serverBlock.directives.push_back(directive);
			}
		}
		serverBlocks.push_back(serverBlock);
	}
}

std::vector<std::vector<t_token> > ServerBlocks::getRawServerBlocks(const std::__1::vector<t_token>& tokens) const
{
    std::vector<std::vector<t_token> > rawServerBlocks;
    
    for (size_t i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].str != "server")
			parser_utils::expected("server block", tokens[i], filePath);
		i++;
		if (i >= tokens.size() || tokens[i].str != "{")
			parser_utils::expected("{", tokens[i], filePath);
		i++;

		size_t start = i;
		bool closed = false;
		int braceLevel = 1;

		while (i < tokens.size())
		{
			if (tokens[i].str == "{")
			{
				if (braceLevel == 1)
					braceLevel++;
				else
					parser_utils::unexpected("{", tokens[i], filePath);
			}
			else if (tokens[i].str == "}")
			{
				braceLevel--;
				if (braceLevel == 0)
				{
					closed = true;
					break;
				}
			}
			i++;
		}

		if (!closed)
			parser_utils::unclosed('{', tokens[start - 2].line, tokens[start - 2].col, filePath);

		rawServerBlocks.push_back(std::vector<t_token>(tokens.begin() + start, tokens.begin() + i));
	}
    for (size_t i = 0; i < rawServerBlocks.size(); )
    {
        if (rawServerBlocks[i].empty())
            rawServerBlocks.erase(rawServerBlocks.begin() + i);
        else
            i++;
    }
    return rawServerBlocks;
}

void ServerBlocks::locationBlock(size_t& j, size_t& i, std::vector<std::vector<t_token> >& rawServerBlocks, t_server_block& serverBlock)
{
	j++;
	if (j >= rawServerBlocks[i].size() || rawServerBlocks[i][j].str == "{" || rawServerBlocks[i][j].str == ";")
		parser_utils::unexpected(rawServerBlocks[i][j].str, rawServerBlocks[i][j], filePath);

	t_location_block location;
	if (rawServerBlocks[i][j].str == "=")
	{
		location.exact = true;
		j++;
	}
	else
		location.exact = false;
	location.name = rawServerBlocks[i][j];

	j++;
	if (j >= rawServerBlocks[i].size() || rawServerBlocks[i][j].str != "{")
		parser_utils::expected("{", rawServerBlocks[i][j], filePath);

	j++;
	while (j < rawServerBlocks[i].size() && rawServerBlocks[i][j].str != "}")
	{
		t_directive directive;
		directive.directive = rawServerBlocks[i][j];
		j++;
		while (j < rawServerBlocks[i].size() && rawServerBlocks[i][j].str != ";")
		{
			directive.options.push_back(rawServerBlocks[i][j]);
			j++;
		}
		location.directives.push_back(directive);
		if (j < rawServerBlocks[i].size() && rawServerBlocks[i][j].str == ";")
			j++;
	}
	if (j >= rawServerBlocks[i].size() || rawServerBlocks[i][j].str != "}")
		parser_utils::unclosed('{', location.name.line, location.name.col, filePath);

	serverBlock.locations.push_back(location);
}
const std::vector<t_server_block>& ServerBlocks::getServerBlocks() const { return serverBlocks; }

