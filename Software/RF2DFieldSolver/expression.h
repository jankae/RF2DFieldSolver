#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QString>
#include <QMap>

namespace Expression {
    // Evaluates a mathematical expression with variable substitution.
    //
    // Supports + - * / ^, unary minus/plus, parentheses and numeric literals with
    // an optional trailing SI unit/prefix suffix (e.g. "0.15mm", "35um", "1k",
    // "4.3"). Literals are resolved in base SI units through Unit::FromString, so
    // the meter unit "m" and the SI prefixes are understood exactly like the rest
    // of the application. Identifiers are looked up in `symbols`.
    //
    // On success returns the finite value. On any error (parse failure, unknown
    // identifier, division by zero, non-finite result) returns NaN and, if `error`
    // is non-null, sets it to a human-readable message. On success `error` is
    // cleared to an empty string.
    double evaluate(const QString &expr, const QMap<QString, double> &symbols, QString *error = nullptr);
}

#endif // EXPRESSION_H
