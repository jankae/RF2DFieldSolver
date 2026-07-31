#include "expression.h"

#include "unit.h"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

// All SI prefixes understood by Unit, used when resolving numeric literals.
static const QString SI_PREFIXES = "fpnumkMGTP ";

struct Token {
    enum class Type { Number, Ident, Plus, Minus, Star, Slash, Caret, LParen, RParen, End };
    Type type = Type::End;
    double value = 0;   // valid for Number
    QString text;       // valid for Ident
};

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const QString &m) : std::runtime_error(m.toStdString()) {}
};

static bool isDigit(QChar c) { return c >= QChar('0') && c <= QChar('9'); }
static bool isLetter(QChar c) {
    return (c >= QChar('a') && c <= QChar('z')) || (c >= QChar('A') && c <= QChar('Z')) || c == QChar('_');
}
static bool isIdentChar(QChar c) { return isLetter(c) || isDigit(c); }

class Tokenizer {
public:
    explicit Tokenizer(const QString &s) : s(s), pos(0) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < s.size()) {
            QChar c = s.at(pos);
            if (c.isSpace()) {
                pos++;
                continue;
            }
            if (isDigit(c) || c == QChar('.')) {
                Token t;
                t.type = Token::Type::Number;
                t.value = readNumber();
                tokens.push_back(t);
                continue;
            }
            if (isLetter(c)) {
                Token t;
                t.type = Token::Type::Ident;
                t.text = readIdent();
                tokens.push_back(t);
                continue;
            }
            Token t;
            switch (c.toLatin1()) {
            case '+': t.type = Token::Type::Plus; break;
            case '-': t.type = Token::Type::Minus; break;
            case '*': t.type = Token::Type::Star; break;
            case '/': t.type = Token::Type::Slash; break;
            case '^': t.type = Token::Type::Caret; break;
            case '(': t.type = Token::Type::LParen; break;
            case ')': t.type = Token::Type::RParen; break;
            default:
                throw ParseError(QString("unexpected character '%1'").arg(c));
            }
            pos++;
            tokens.push_back(t);
        }
        Token end;
        end.type = Token::Type::End;
        tokens.push_back(end);
        return tokens;
    }

private:
    // Reads a numeric literal together with an optional trailing SI unit/prefix
    // suffix and resolves it in base units. This makes "0.15mm", "35um", "1k" and
    // "4.3" all evaluate correctly and prevents a suffix like "mm" from being
    // mistaken for an identifier.
    double readNumber() {
        int start = pos;
        while (pos < s.size() && (isDigit(s.at(pos)) || s.at(pos) == QChar('.'))) {
            pos++;
        }
        // optional exponent (only when actually followed by digits)
        if (pos < s.size() && (s.at(pos) == QChar('e') || s.at(pos) == QChar('E'))) {
            int save = pos;
            pos++;
            if (pos < s.size() && (s.at(pos) == QChar('+') || s.at(pos) == QChar('-'))) {
                pos++;
            }
            if (pos < s.size() && isDigit(s.at(pos))) {
                while (pos < s.size() && isDigit(s.at(pos))) {
                    pos++;
                }
            } else {
                // not an exponent after all, leave 'e' to be read as a suffix letter
                pos = save;
            }
        }
        // optional SI unit/prefix suffix letters
        while (pos < s.size() && isLetter(s.at(pos))) {
            pos++;
        }
        QString literal = s.mid(start, pos - start);
        double v = Unit::FromString(literal, "m", SI_PREFIXES);
        if (std::isnan(v)) {
            throw ParseError(QString("invalid number '%1'").arg(literal));
        }
        return v;
    }

    QString readIdent() {
        int start = pos;
        while (pos < s.size() && isIdentChar(s.at(pos))) {
            pos++;
        }
        return s.mid(start, pos - start);
    }

    const QString &s;
    int pos;
};

class Parser {
public:
    Parser(const std::vector<Token> &tokens, const QMap<QString, double> &symbols)
        : tokens(tokens), symbols(symbols), idx(0) {}

    double parse() {
        double v = parseExpr();
        if (peek().type != Token::Type::End) {
            throw ParseError("unexpected trailing input");
        }
        return v;
    }

private:
    const Token &peek() const { return tokens[idx]; }
    void advance() { idx++; }

    double parseExpr() { // + and -
        double v = parseTerm();
        while (true) {
            auto t = peek().type;
            if (t == Token::Type::Plus) {
                advance();
                v += parseTerm();
            } else if (t == Token::Type::Minus) {
                advance();
                v -= parseTerm();
            } else {
                break;
            }
        }
        return v;
    }

    double parseTerm() { // * and /
        double v = parseUnary();
        while (true) {
            auto t = peek().type;
            if (t == Token::Type::Star) {
                advance();
                v *= parseUnary();
            } else if (t == Token::Type::Slash) {
                advance();
                double d = parseUnary();
                if (d == 0.0) {
                    throw ParseError("division by zero");
                }
                v /= d;
            } else {
                break;
            }
        }
        return v;
    }

    double parseUnary() {
        if (peek().type == Token::Type::Minus) {
            advance();
            return -parseUnary();
        }
        if (peek().type == Token::Type::Plus) {
            advance();
            return parseUnary();
        }
        return parsePower();
    }

    double parsePower() { // ^ (right associative)
        double base = parsePrimary();
        if (peek().type == Token::Type::Caret) {
            advance();
            double exp = parseUnary();
            return std::pow(base, exp);
        }
        return base;
    }

    double parsePrimary() {
        const Token &t = peek();
        if (t.type == Token::Type::Number) {
            advance();
            return t.value;
        }
        if (t.type == Token::Type::Ident) {
            advance();
            auto it = symbols.find(t.text);
            if (it == symbols.end()) {
                throw ParseError(QString("unknown parameter '%1'").arg(t.text));
            }
            return it.value();
        }
        if (t.type == Token::Type::LParen) {
            advance();
            double v = parseExpr();
            if (peek().type != Token::Type::RParen) {
                throw ParseError("missing ')'");
            }
            advance();
            return v;
        }
        throw ParseError("expected a value");
    }

    const std::vector<Token> &tokens;
    const QMap<QString, double> &symbols;
    size_t idx;
};

} // namespace

double Expression::evaluate(const QString &expr, const QMap<QString, double> &symbols, QString *error)
{
    if (error) {
        *error = QString();
    }
    QString trimmed = expr.trimmed();
    if (trimmed.isEmpty()) {
        if (error) {
            *error = "empty expression";
        }
        return std::numeric_limits<double>::quiet_NaN();
    }
    try {
        Tokenizer tk(trimmed);
        auto tokens = tk.tokenize();
        Parser p(tokens, symbols);
        double v = p.parse();
        if (std::isnan(v) || std::isinf(v)) {
            if (error) {
                *error = "result is not finite";
            }
            return std::numeric_limits<double>::quiet_NaN();
        }
        return v;
    } catch (const ParseError &e) {
        if (error) {
            *error = QString::fromUtf8(e.what());
        }
        return std::numeric_limits<double>::quiet_NaN();
    }
}
