#include "payload.hpp"

#include <iomanip>
#include <sstream>

std::string make_payload_v1(const Sample& s, std::uint64_t seq,
                            const std::string& unit) {
	std::ostringstream os;
	os << R"({"v":1,"ts":)" << s.timestamp << R"(,"value":)" << std::fixed
	   << std::setprecision(2) << s.value << R"(,"unit":")" << unit
	   << R"(","seq":)" << seq << '}';
	return os.str();
}
