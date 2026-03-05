#include "utils.hpp"

bool parseIPv4ToNetworkOrder(const std::string &ip, uint32_t &outNetworkOrder)
{
	unsigned int octets[4] = {0, 0, 0, 0};
	size_t octetIndex = 0;
	size_t value = 0;
	bool hasDigit = false;

	for (size_t i = 0; i < ip.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(ip[i]);
		if (std::isdigit(c)) {
			hasDigit = true;
			value = value * 10 + static_cast<size_t>(c - '0');
			if (value > 255)
				return false;
		} else if (c == '.') {
			if (!hasDigit || octetIndex >= 3)
				return false;
			octets[octetIndex++] = static_cast<unsigned int>(value);
			value = 0;
			hasDigit = false;
		} else {
			return false;
		}
	}

	if (!hasDigit || octetIndex != 3)
		return false;

	octets[3] = static_cast<unsigned int>(value);

	uint32_t hostOrder = (static_cast<uint32_t>(octets[0]) << 24)
					   | (static_cast<uint32_t>(octets[1]) << 16)
					   | (static_cast<uint32_t>(octets[2]) << 8)
					   | static_cast<uint32_t>(octets[3]);
	outNetworkOrder = htonl(hostOrder);
	return true;
}

std::string formatIPv4FromNetworkOrder(uint32_t networkOrderIp)
{
	uint32_t hostOrder = ntohl(networkOrderIp);
	std::ostringstream oss;
	oss << ((hostOrder >> 24) & 0xFF) << "."
		<< ((hostOrder >> 16) & 0xFF) << "."
		<< ((hostOrder >> 8) & 0xFF) << "."
		<< (hostOrder & 0xFF);
	return oss.str();
}
