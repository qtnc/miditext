#ifndef ______STRINGUTILS1
#define ______STRINGUTILS1
#include<string>
#include<vector>
#include<boost/algorithm/string.hpp>

#pragma GCC diagnostic ignored "-Wsign-compare"

using boost::starts_with;
using boost::ends_with;
using boost::istarts_with;
using boost::iends_with;
using boost::contains;
using boost::icontains;
using boost::iequals;
using boost::replace_all;
using boost::replace_all_copy;
using boost::trim;
using boost::trim_copy;
using boost::to_lower;
using boost::to_lower_copy;
using boost::to_upper;
using boost::to_upper_copy;
using boost::join;

template<class T> std::string itos (T n, int base) {
static const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";
char buf[64] = {0}, *out = &buf[63];
do {
*(--out) = digits[n%base];
n/=base;
} while(n>0);
return out;
}

inline std::vector<std::string> split (const std::string& input, const std::string& delims, bool compress = false) {
std::vector<std::string> result;
boost::split(result, input, boost::is_any_of(delims), compress? boost::token_compress_on : boost::token_compress_off);
return result;
}

inline void replace_translate (std::string& str, const std::string& src, const std::string& dst) {
for (auto& c: str) {
auto i = src.find(c);
if (i<src.size() && i<dst.size()) c = dst[i];
}}

std::string formatTime (int seconds);
std::string formatSize (long long filesize);

template <class T, class S> void positionToXY (const T& str, S pos, S& x, S& y) {
y = std::count(str.begin(), str.begin() + pos, '\n');
auto l = str.rfind('\n', pos? pos -1 : 0);
x = l==std::string::npos? pos : pos-l -1;
}

template <class T, class S> S xyToPosition (const T& str, S x, S y) {
if (y==0) return x;
for (size_t i=0, c=0, n=str.size(); i<n; i++) {
if (str[i]=='\n' && ++c==y) return i+x+1;
}
return static_cast<S>(std::string::npos);
}


#endif
