#ifndef FLAGS_H_
#define FLAGS_H_

#include <algorithm>
#include <array>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace flags {
using argument_map = std::unordered_map<std::string, std::string>;

namespace detail {
// Non-destructively parses the argv tokens.
// * If the token begins with a -, it will be considered an option.
// * If the token does not begin with a -, it will be considered a value for the
// previous option. If there was no previous option, it will be considered a
// positional argument.
struct parser {
	parser(const int argc, char** argv) : current_option_{nullptr} {
		for(int i = 1; i < argc; ++i)
			arguments_.emplace_back(argv[i]);
		for(auto& arg : arguments_)
			churn(arg);
		flush();
	}
	parser& operator=(const parser&) = delete;

	const argument_map& options() const { return options_; }
	const std::vector<std::string>& positional_arguments() const {
		return positional_arguments_;
	}

  private:
	// Advance the state machine for the current token.
	void churn(std::string& item) {
		item.at(0) == '-' ? on_option(item) : on_value(item);
	}

	// Consumes the current option if there is one.
	void flush() {
		if(current_option_)
			on_value();
	}

	void on_option(std::string& option) {
		// Consume the current_option and reassign it to the new option while
		// removing all leading dashes.
		flush();
		current_option_ = &option;
		current_option_->erase(0, current_option_->find_first_not_of('-'));

		// Handle a packed argument (--arg_name=value).
		const auto delimiter = current_option_->find_first_of('=');
		if(delimiter != std::string::npos) {
			auto value = current_option_->substr(delimiter+1, std::string::npos);
			current_option_->erase(delimiter, std::string::npos);
			on_value(value);
		}
	}

	void on_value(const std::string& value = "true") {
		// If there's not an option preceding the value, it's a positional
		// argument.
		if(!current_option_) {
			positional_arguments_.push_back(value);
			return;
		}
		// Consume the preceding option and assign its value.
		options_.emplace(*current_option_, value);
		current_option_ = nullptr;
	}

	std::string* current_option_;
	argument_map options_;
	std::vector<std::string> positional_arguments_;
	std::vector<std::string> arguments_;
};

// If a key exists, return an optional populated with its value.
inline bool get_value(std::string& value, const argument_map& options,
                                       const std::string& option) {
	const auto it = options.find(option);
	if(it == options.end())
		return false;
	value = it->second;
	return true;
}

// Coerces the string value of the given option into <T>.
// If the value cannot be properly parsed or the key does not exist, returns
// nullopt.
template <class T>
bool get(T& value, const argument_map& options, const std::string& option) {
	std::string sval;
	if(get_value(sval, options, option)) {
		if(std::istringstream(sval) >> value)
			return true;
	}
	return false;
}

// Since the values are already stored as strings, there's no need to use `>>`.
template <>
bool get(std::string& value, const argument_map& options,
                          const std::string& option) {
	return get_value(value, options, option);
}

// Special case for booleans: if the value is any of the below, the option will
// be considered falsy. Otherwise, it will be considered truthy just for being
// present.
const std::vector<std::string> falsities{"0", "n", "no", "f", "false"};

bool get(const argument_map& options, const std::string& option) {
	std::string sval;
	if(get_value(sval, options, option)) {
		return std::none_of(
		    falsities.begin(), falsities.end(),
		    [&sval](const std::string& falsity) { return sval == falsity; });
	}
	return false;
}
} // namespace detail

struct args {
	args(const int argc, char** argv) : parser_(argc, argv) {}

	template <class T>
	bool get(T& value, const std::string& option) const {
		return detail::get<T>(value, parser_.options(), option);
	}

	template <class T>
	bool get(T& value, const std::string& option, const T& def) const {
		if(detail::get<T>(value, parser_.options(), option))
			return true;
		value = def;
		return false;
	}

	template <class T>
	bool get(T& value, const std::string& option, const T&& def) const {
		if(detail::get<T>(value, parser_.options(), option))
			return true;
		value = def;
		return false;
	}

	bool get(const std::string& option) const {
		return detail::get(parser_.options(), option);
	}

	const std::vector<std::string>& positional() const {
		return parser_.positional_arguments();
	}

  private:
	const detail::parser parser_;
};

} // namespace flags

#endif // FLAGS_H_
