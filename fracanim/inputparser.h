#pragma once

#include <vector>
#include <string>
#include <algorithm>

class InputParser {
public:
	InputParser() = delete;
	InputParser(const std::vector<std::string>& args)
		: _tokens(args) {}
	const std::string& getCmdOption(const std::string& option) const {
		auto itr = std::find(_tokens.begin(), _tokens.end(), option);
		if (itr != _tokens.end() && ++itr != _tokens.end()) {
			return *itr;
		}
		static const std::string empty_string("");
		return empty_string;
	}
	const std::string& getThisOption() const {
		return getCmdOption(_option);
	}
	bool cmdOptionExists(const std::string& option) const {
		const auto res = (std::find(_tokens.begin(), _tokens.end(), option) != _tokens.end());
		if (res)
			_option = option;
		return res;
	}
private:
	mutable std::string _option;
	std::vector<std::string> _tokens;
};