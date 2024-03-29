/**
 * @file    base64.h
 * @brief   BASE64编解码方法
 */

#ifndef BASE64_H

#include <string>

namespace vxUtil
{
	void base64Encode(const std::string& input, std::string& output);

	void base64Decode(const std::string& input, std::string& output);
}


#endif // BASE64_H