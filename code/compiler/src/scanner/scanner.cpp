#include "scanner/scanner.hpp"

#include <istream>
#include <sstream>
#include <stdexcept>

#include "scanner/scanner_runtime.hpp"

#if COMPILER_SCANNER_HAS_FLEX
#include <FlexLexer.h>
#endif

namespace compiler::scanner {

namespace {

class StringViewStreamBuf final : public std::streambuf {
   public:
    explicit StringViewStreamBuf(const std::string_view source) {
        const auto begin = const_cast<char*>(source.data());
        setg(begin, begin, begin + static_cast<std::ptrdiff_t>(source.size()));
    }
};

}  // namespace

ScanResult scan(const std::string_view source) {
    auto& scanner = internal::runtime();
    scanner.reset();

#if COMPILER_SCANNER_HAS_FLEX
    StringViewStreamBuf input_buffer{source};
    std::istream input{&input_buffer};
    std::ostringstream output;
    yyFlexLexer lexer{&input, &output};
    ScanResult result{};

    while (const int token = lexer.yylex()) {
        result.tokens.push_back(ScannedToken{
            .kind = static_cast<TokenType>(token),
            .span = yylloc,
            .lexeme = std::string{yylval.lexeme},
        });
    }

    result.errors = scanner.take_errors();
    return result;
#else
    (void)source;
    throw std::runtime_error{"Scanner support requires Flex at build time."};
#endif
}

}  // namespace compiler::scanner
