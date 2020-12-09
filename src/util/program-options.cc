/*
 * This file is open source software, licensed to you under the terms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.// Custom "validator" that gets called by the internals of Boost.Test. This allows for reading associations into an
// unordered map and for multiple occurances of the option name to appear and be merged.
 */
/*
 * Copyright (C) 2017 ScyllaDB
 */

#include <seastar/util/program-options.hh>

#include <regex>
#include <algorithm>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/combine.hpp>

namespace bpo = boost::program_options;

namespace seastar {

namespace program_options {

sstring get_or_default(const string_map& ss, const sstring& key, const sstring& def) {
    const auto iter = ss.find(key);
    if (iter != ss.end()) {
        return iter->second;
    }

    return def;
}

static void parse_map_associations(const std::string& v, string_map& ss) {
    static const std::regex colon(":");

    std::sregex_token_iterator s(v.begin(), v.end(), colon, -1);
    const std::sregex_token_iterator e;
    while (s != e) {
        const sstring p = std::string(*s++);

        const auto i = p.find('=');
        if (i == sstring::npos) {
            throw bpo::invalid_option_value(p);
        }

        auto k = p.substr(0, i);
        auto v = p.substr(i + 1, p.size());
        ss[std::move(k)] = std::move(v);
    };
}

std::vector<uint8_t> parse_string_comma(std::string str) {
    std::vector<uint8_t> uint8_vec = {};
    std::vector<std::string> str_vec;
    boost::split(str_vec, str, boost::is_any_of(","));
    for (auto i : str_vec) {
        boost::trim(i);
        uint8_vec.push_back(boost::lexical_cast<uint8_t>(i));
        sort(uint8_vec.begin(), uint8_vec.end());
    }
    return uint8_vec;
}

void validate(boost::any& out, const std::vector<std::string>& in, string_map*, int) {
    if (out.empty()) {
        out = boost::any(string_map());
    }

    auto* ss = boost::any_cast<string_map>(&out);

    for (const auto& s : in) {
        parse_map_associations(s, *ss);
    }
}

std::ostream& operator<<(std::ostream& os, const string_map& ss) {
    int n = 0;

    for (const auto& e : ss) {
        if (n > 0) {
            os << ":";
        }

        os << e.first << "=" << e.second;
        ++n;
    }

    return os;
}

std::istream& operator>>(std::istream& is, string_map& ss) {
    std::string str;
    is >> str;

    parse_map_associations(str, ss);
    return is;
}

}

}
