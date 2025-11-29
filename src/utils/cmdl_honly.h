#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

/*
* Command line parser - Header only.
*
* Copyright (c) 2025 - Pratim Majumder
*/

namespace dns_cash {

	class CMDLineParser {
	private:
		unsigned short dns_port = 53;
		bool enable_affinity = false;
		unsigned int target_tcount = 8;
		unsigned int system_ltpc = 0;
		bool user_entered_any_arg = false;

		static bool match_user_arg(const std::string& arg, const std::initializer_list<std::string>& names) {
			for (const auto& n : names) {
				if (arg == "--" + n || arg == "-" + n)
					return true;
			}
			return false;
		}

	public:
		CMDLineParser(int argc, char* argv[]) {
			this->user_entered_any_arg = argc > 1;

			try {
				for (int i = 1; i < argc; ++i) {
					std::string arg = argv[i];
					std::string value;

					// Extract value if in form --arg=5 or -arg=5
					size_t eq_sys_pos = arg.find('=');
					if (eq_sys_pos != std::string::npos) {
						value = arg.substr(eq_sys_pos + 1);
						arg = arg.substr(0, eq_sys_pos);
					}

					if (match_user_arg(arg, { "port", "p" })) {
						if (value.empty() && i + 1 < argc) value = argv[++i];
						if (value.empty()) throw std::invalid_argument("missing port value");
						dns_port = static_cast<unsigned short>(std::stoi(value));
					}
					else if (match_user_arg(arg, { "tcount", "t" })) {
						if (value.empty() && i + 1 < argc) value = argv[++i];
						if (value.empty()) throw std::invalid_argument("missing tcount value");
						target_tcount = std::stoi(value);
					}
					else if (match_user_arg(arg, { "ltpc", "l" })) {
						if (value.empty() && i + 1 < argc) value = argv[++i];
						if (value.empty()) throw std::invalid_argument("missing ltpc value");
						system_ltpc = std::stoi(value);
					}
					else if (match_user_arg(arg, { "affinity", "a" })) {
						enable_affinity = true;
					}
					else if (match_user_arg(arg, { "help", "h" })) {
						print_help_message();
						std::exit(0);
					}
					else if (match_user_arg(arg, { "-help", "--help", "--h" })) { // extra safety
						print_help_message();
						std::exit(0);
					}
					else {
						std::cerr << "Unknown argument: " << arg << "\n";
						print_help_message();
						std::exit(1);
					}
				}

				// Validation rules
				if (enable_affinity && system_ltpc == 0) {
					std::cerr << "Error: --affinity requires --ltpc\n";
					print_help_message();
					std::exit(1);
				}
				if (enable_affinity && target_tcount != 8) {
					std::cerr << "Error: --affinity and --tcount cannot coexist\n";
					print_help_message();
					std::exit(1);
				}

			}
			catch (const std::exception& e) {
				std::cerr << "Command-line parsing error: " << e.what() << "\n";
				print_help_message();
				std::exit(1);
			}
			catch (...) {
				std::cerr << "Command-line parsing error" << "\n";
				print_help_message();
				std::exit(3);
			}
		}

		unsigned short get_user_req_port() const { return dns_port; }
		unsigned int get_user_req_tcount() const { return target_tcount; }
		unsigned int get_user_provided_ltpc() const { return system_ltpc; }
		bool should_enable_affinity() const { return enable_affinity; }
		bool should_enable_commands() const { return user_entered_any_arg; }

		void print_help_message() const {
			std::cout << R"(
dns_cash Command-Line Options
-----------------------------------
All arguments are optional unless specified otherwise.
Formats supported: --arg, -arg, --arg=val, -arg=val, --arg val, -arg val

Options:
  -p, --port <num>
        Set the DNS service port number.
        Default: 53
        Example: --port 8053  or  -p=8053

  -t, --tcount <num>
        Specify target thread count for DNS worker threads.
        Default: 8
        Example: --tcount 12  or  -t=12
        Note: Cannot be used with --affinity.

  -l, --ltpc <num>
        Specify logical threads per core value.
        Required if --affinity is enabled.
        Example: --ltpc 2  or  -l=2

  -a, --affinity
        Enable CPU affinity binding.
        Requires: --ltpc
        Incompatible with: --tcount

  -h, --h, -help, --help
        Show this detailed help message.

Examples:
  dns_cash --port=8053 --tcount=4
  dns_cash -a -l 2
  dns_cash --affinity --ltpc=4
)";
		}
	};

}
