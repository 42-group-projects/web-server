#pragma once
#include "../../include/imports.hpp"

typedef struct t_token
{
	std::string str;
	int         line;
	int         col;
} s_token;

class TokenizeFile
{
public:
	TokenizeFile(int argc, char **argv);
	const std::string& getFilePath() const;
	const std::vector<t_token>& getTokens() const;

private:
	std::string filePath;
	std::vector<t_token> tokens;

	std::vector<std::string> loadConfigFile(const std::string& path);
	void removeComments(std::vector<std::string>& rawConfig);
	void tokenize(std::vector<std::string>& rawConfig);
	void handleQuotedToken
	(
	    std::vector<t_token>& tokens,
	    std::vector<std::string>& rawConfig,
	    size_t& i, size_t& j,
	    const std::string& filePath
	);
};
