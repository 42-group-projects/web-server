#pragma once

#include "../../include/imports.hpp"

bool parseIPv4ToNetworkOrder(const std::string &ip, uint32_t &outNetworkOrder);
std::string formatIPv4FromNetworkOrder(uint32_t networkOrderIp);
