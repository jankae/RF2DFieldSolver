#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QString>
#include <QStringList>
#include <QMap>

namespace Expression {
    // Evaluates a mathematical expression with variable substitution.
    //
    // Supports + - * / ^, unary minus/plus, parentheses, the functions max()/min()
    // (variadic), abs() and sqrt(), and numeric literals with an optional trailing
    // SI unit/prefix suffix (e.g. "0.15mm", "35um", "1k", "4.3"). Literals are
    // resolved in base SI units through Unit::FromString, so the meter unit "m" and
    // the SI prefixes are understood exactly like the rest of the application.
    // Identifiers that are not a function name are looked up in `symbols`.
    //
    // On success returns the finite value. On any error (parse failure, unknown
    // identifier, division by zero, non-finite result) returns NaN and, if `error`
    // is non-null, sets it to a human-readable message. On success `error` is
    // cleared to an empty string.
    double evaluate(const QString &expr, const QMap<QString, double> &symbols, QString *error = nullptr);

    // The reserved function names understood by evaluate(). Parameters may not use
    // any of these as their name (see isValidParameterName).
    QStringList functionNames();

    // Whether `name` is usable as a parameter name: a non-empty identifier
    // ([A-Za-z_][A-Za-z0-9_]*) that is not a reserved function name. Names that
    // fail this can never be referenced from an expression.
    bool isValidParameterName(const QString &name);
}

#endif // EXPRESSION_H
