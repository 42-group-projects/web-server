#include "TokenizeFile.hpp"

#include "../../../src/errorHandling/ErrorWarning.hpp"
#include "../../../include/defaultConfigs.hpp"
#include "error_messages.hpp"

#include <fstream>

TokenizeFile::TokenizeFile(int argc, char **argv)
{
	std::vector<std::string> rawConfig;

	if (argc == 1)
	{
		warning("No file provided. Using default file.", "Configuration file");
		rawConfig = loadConfigFile(DEFAULT_CONF_FILE);
	}
	else
	{
		std::string arg(argv[1]);
		size_t ext = arg.rfind(".conf");

		if (ext != std::string::npos && ext + 5 == arg.size())
			rawConfig = loadConfigFile(arg);
		else
		{
			warning("Invalid file. Using default file.", "Configuration file");
			rawConfig = loadConfigFile(DEFAULT_CONF_FILE);
		}
	}

	removeComments(rawConfig);
	tokenize(rawConfig);
}

const std::string& TokenizeFile::getFilePath() const { return filePath; }
const std::vector<t_token>& TokenizeFile::getTokens() const { return tokens; }


std::vector<std::string> TokenizeFile::loadConfigFile(const std::string& path)
{
	std::ifstream file(path.c_str());

	if (!file)
	{
		if (path == DEFAULT_CONF_FILE)
			error("file '" + std::string(DEFAULT_CONF_FILE) + "' not found. Server will not start.", "Configuration file");
		else
		{
			warning("Couldn't open file. Using default file.", "Configuration file");
			return loadConfigFile(DEFAULT_CONF_FILE);
		}
	}

	filePath = path;
	std::vector<std::string> lines;
	std::string line;

	while (std::getline(file, line))
		lines.push_back(line);

	return lines;
}

void TokenizeFile::removeComments(std::vector<std::string>& rawConfig)
{
	for (size_t i = 0; i < rawConfig.size(); i++)
	{
		for (size_t j = 0; j < rawConfig[i].size(); j++)
		{
			if (rawConfig[i][j] == '#')
			{
				rawConfig[i].erase(j); break;
			}
		}
	}
}

void TokenizeFile::tokenize(std::vector<std::string>& rawConfig)
{
	std::string delimiters = " \t;{}";

	for (size_t i = 0; i < rawConfig.size(); i++)
	{
		int start = -1;
		int end = -1;

		for (size_t j = 0; j < rawConfig[i].size(); j++)
		{
			if (start == -1)
			{
				if (rawConfig[i][j] != ' ' && rawConfig[i][j] != '\t')
					start = j;
			}

			if (start != -1 && end == -1)
			{
				if (rawConfig[i][j] == '\'' || rawConfig[i][j] == '\"')
				{
					handleQuotedToken(tokens, rawConfig, i, j, filePath);
					start = -1;
					end = -1;
					continue;
				}

				if (j + 1 >= rawConfig[i].size() || delimiters.find(rawConfig[i][j + 1]) != std::string::npos)
					end = j;

				if (end != -1)
				{
					t_token token;
					token.line = i + 1;
					token.col = start + 1;
					token.str = rawConfig[i].substr(start, end - start + 1);
					tokens.push_back(token);
					start = -1;
					end = -1;
				}
			}
		}
	}
}

void TokenizeFile::handleQuotedToken
(
    std::vector<t_token>& tokens,
    std::vector<std::string>& rawConfig,
    size_t& i,
    size_t& j,
    const std::string& filePath
)
{
	char quote = rawConfig[i][j];
	std::string value;
	size_t lineStart = i;
	size_t colStart = j + 1;
	j++;
	bool closed = false;

	while (i < rawConfig.size())
	{
		while (j < rawConfig[i].size())
		{
			if (rawConfig[i][j] == quote)
			{
				closed = true;
				break;
			}

			value += rawConfig[i][j];
			j++;
		}

		if (closed)
			break;

		value += '\n';
		i++;
		j = 0;
	}

	if (!closed)
		error_messages::unclosed(quote, lineStart + 1, colStart, filePath);

	t_token token;
	token.line = lineStart + 1;
	token.col = colStart;
	token.str = value;
	tokens.push_back(token);
}

