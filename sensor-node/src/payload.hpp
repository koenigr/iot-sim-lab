#pragma once
#include <cstdint>
#include <string>

#include "generator.hpp"

std::string make_payload_v1(const Sample& s, std::uint64_t seq,
                            const std::string& unit);
