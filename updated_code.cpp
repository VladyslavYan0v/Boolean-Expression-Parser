#include <iostream>
#include <string>
#include <map>
#include <set>
#include <cassert>

using namespace std;

class Expression {
public:
    virtual ~Expression() {}
    virtual Expression* simplify() = 0;
    virtual void print() const = 0;
    virtual string toString() const = 0;
    virtual Expression* clone() const = 0;
    virtual Expression* substitute(const map<char, int>& vars) const = 0;
    virtual void findVariables(set<char>& vars) const = 0;

    virtual bool isConstant(int& val) const { return false; }
    virtual bool equals(const Expression* other) const = 0;
};

class Constant : public Expression {
    int value;
public:
    Constant(int v) : value(v) {}
    Expression* simplify() override { return new Constant(value); }
    void print() const override { cout << value; }
    string toString() const override { return to_string(value); }
    Expression* clone() const override { return new Constant(value); }
    Expression* substitute(const map<char, int>& vars) const override { return clone(); }
    void findVariables(set<char>& vars) const override {}
    bool isConstant(int& val) const override { val = value; return true; }
    bool equals(const Expression* other) const override {
        int otherVal;
        return other->isConstant(otherVal) && value == otherVal;
    }
};

class Variable : public Expression {
    char name;
public:
    Variable(char n) : name(n) {}
    Expression* simplify() override { return new Variable(name); }
    void print() const override { cout << name; }
    string toString() const override { return string(1, name); }
    Expression* clone() const override { return new Variable(name); }
    Expression* substitute(const map<char, int>& vars) const override {
        if (vars.count(name)) return new Constant(vars.at(name));
        return clone();
    }
    void findVariables(set<char>& vars) const override { vars.insert(name); }
    bool equals(const Expression* other) const override {
        const Variable* v = dynamic_cast<const Variable*>(other);
        return v && v->name == name;
    }
};

class NotOperation : public Expression {
public:
    Expression* operand;

    NotOperation(Expression* op) : operand(op) {}
    ~NotOperation() { delete operand; }

    void print() const override { cout << "!("; operand->print(); cout << ")"; }
    string toString() const override { return "!(" + operand->toString() + ")"; }
    Expression* clone() const override { return new NotOperation(operand->clone()); }
    Expression* substitute(const map<char, int>& vars) const override {
        return new NotOperation(operand->substitute(vars));
    }
    void findVariables(set<char>& vars) const override { operand->findVariables(vars); }

    bool equals(const Expression* other) const override {
        const NotOperation* notOp = dynamic_cast<const NotOperation*>(other);
        return notOp && operand->equals(notOp->operand);
    }

    Expression* simplify() override {
        Expression* simpOp = operand->simplify();
        int val;
        if (simpOp->isConstant(val)) {
            delete simpOp;
            return new Constant(val == 0 ? 1 : 0);
        }
        if (NotOperation* innerNot = dynamic_cast<NotOperation*>(simpOp)) {
            Expression* innerMost = innerNot->operand->clone();
            delete simpOp;
            return innerMost;
        }
        return new NotOperation(simpOp);
    }
};

class AndOperation : public Expression {
public:
    Expression* left;
    Expression* right;

    AndOperation(Expression* l, Expression* r) : left(l), right(r) {}
    ~AndOperation() { delete left; delete right; }

    void print() const override { cout << "("; left->print(); cout << "&"; right->print(); cout << ")"; }
    string toString() const override { return "(" + left->toString() + "&" + right->toString() + ")"; }
    Expression* clone() const override { return new AndOperation(left->clone(), right->clone()); }
    Expression* substitute(const map<char, int>& vars) const override {
        return new AndOperation(left->substitute(vars), right->substitute(vars));
    }
    void findVariables(set<char>& vars) const override {
        left->findVariables(vars); right->findVariables(vars);
    }
    bool equals(const Expression* other) const override {
        const AndOperation* andOp = dynamic_cast<const AndOperation*>(other);
        return andOp && left->equals(andOp->left) && right->equals(andOp->right);
    }

    Expression* simplify() override {
        Expression* l = left->simplify();
        Expression* r = right->simplify();
        int lVal, rVal;

        if (l->isConstant(lVal) && lVal == 0) { delete l; delete r; return new Constant(0); }
        if (r->isConstant(rVal) && rVal == 0) { delete l; delete r; return new Constant(0); }

        if (l->isConstant(lVal) && lVal == 1) { delete l; return r; }
        if (r->isConstant(rVal) && rVal == 1) { delete r; return l; }

        if (l->equals(r)) { delete r; return l; }

        if (NotOperation* notR = dynamic_cast<NotOperation*>(r)) {
            if (notR->operand->equals(l)) { delete l; delete r; return new Constant(0); }
        }
        if (NotOperation* notL = dynamic_cast<NotOperation*>(l)) {
            if (notL->operand->equals(r)) { delete l; delete r; return new Constant(0); }
        }

        return new AndOperation(l, r);
    }
};

class OrOperation : public Expression {
public:
    Expression* left;
    Expression* right;

    OrOperation(Expression* l, Expression* r) : left(l), right(r) {}
    ~OrOperation() { delete left; delete right; }

    void print() const override { cout << "("; left->print(); cout << "|"; right->print(); cout << ")"; }
    string toString() const override { return "(" + left->toString() + "|" + right->toString() + ")"; }
    Expression* clone() const override { return new OrOperation(left->clone(), right->clone()); }
    Expression* substitute(const map<char, int>& vars) const override {
        return new OrOperation(left->substitute(vars), right->substitute(vars));
    }
    void findVariables(set<char>& vars) const override {
        left->findVariables(vars); right->findVariables(vars);
    }
    bool equals(const Expression* other) const override {
        const OrOperation* orOp = dynamic_cast<const OrOperation*>(other);
        return orOp && left->equals(orOp->left) && right->equals(orOp->right);
    }

    Expression* simplify() override {
        Expression* l = left->simplify();
        Expression* r = right->simplify();
        int lVal, rVal;

        if (l->isConstant(lVal) && lVal == 1) { delete l; delete r; return new Constant(1); }
        if (r->isConstant(rVal) && rVal == 1) { delete l; delete r; return new Constant(1); }

        if (l->isConstant(lVal) && lVal == 0) { delete l; return r; }
        if (r->isConstant(rVal) && rVal == 0) { delete r; return l; }

        if (l->equals(r)) { delete r; return l; }

        if (NotOperation* notR = dynamic_cast<NotOperation*>(r)) {
            if (notR->operand->equals(l)) { delete l; delete r; return new Constant(1); }
        }
        if (NotOperation* notL = dynamic_cast<NotOperation*>(l)) {
            if (notL->operand->equals(r)) { delete l; delete r; return new Constant(1); }
        }

        if (AndOperation* andR = dynamic_cast<AndOperation*>(r)) {
            if (andR->left->equals(l) || andR->right->equals(l)) { delete r; return l; }
        }
        if (AndOperation* andL = dynamic_cast<AndOperation*>(l)) {
            if (andL->left->equals(r) || andL->right->equals(r)) { delete l; return r; }
        }

        return new OrOperation(l, r);
    }
};

class Parser {
    string input;
    int pos;
    char currentChar() { return pos < input.length() ? input[pos] : '\0'; }
    void nextChar() { pos++; }
    void skipWhitespace() { while (currentChar() == ' ' || currentChar() == '\n') nextChar(); }

    Expression* parseFactor() {
        skipWhitespace();
        char c = currentChar();
        if (c == '!') {
            nextChar();
            return new NotOperation(parseFactor());
        }
        if (c == '(') {
            nextChar();
            Expression* expr = parseExpr();
            skipWhitespace();
            if (currentChar() == ')') nextChar();
            return expr;
        }
        if (c == '0' || c == '1') {
            nextChar();
            return new Constant(c - '0');
        }
        if (c >= 'a' && c <= 'z') {
            nextChar();
            return new Variable(c);
        }
        return new Constant(0);
    }

    Expression* parseExpr() {
        Expression* left = parseFactor();
        skipWhitespace();
        while (currentChar() == '&' || currentChar() == '|') {
            char op = currentChar();
            nextChar();
            Expression* right = parseFactor();
            if (op == '&') left = new AndOperation(left, right);
            else left = new OrOperation(left, right);
            skipWhitespace();
        }
        return left;
    }

public:
    Expression* parse(const string& str) {
        input = str;
        pos = 0;
        return parseExpr();
    }
};

void runTest(const string& label, const string& inputStr, const string& expectedOutput) {
    Parser p;
    Expression* tree = p.parse(inputStr);
    Expression* simplified = tree->simplify();

    cout << label << ": " << tree->toString() << " => " << simplified->toString();

    if (simplified->toString() == expectedOutput) {
        cout << "  [PASSED]" << endl;
    }
    else {
        cout << "  [FAILED - Expected " << expectedOutput << "]" << endl;
    }

    delete tree;
    delete simplified;
}

void runSubstTest(const string& label, const string& inputStr, const map<char, int>& vars, const string& expectedOutput) {
    Parser p;
    Expression* tree = p.parse(inputStr);
    Expression* substituted = tree->substitute(vars);
    Expression* simplified = substituted->simplify();

    cout << label << ": " << tree->toString() << " => " << simplified->toString();

    if (simplified->toString() == expectedOutput) {
        cout << "  [PASSED]" << endl;
    }
    else {
        cout << "  [FAILED - Expected " << expectedOutput << "]" << endl;
    }

    delete tree;
    delete substituted;
    delete simplified;
}

void runUnitTests() {
    cout << "========================================" << endl;
    cout << "     RUNNING AUTOMATED UNIT TESTS       " << endl;
    cout << "========================================" << endl;

    cout << "\n--- 1. Basic Logical Operations ---" << endl;
    runTest("[Test 1.1] Logical AND", "1&0", "0");
    runTest("[Test 1.2] Logical OR ", "0|1", "1");
    runTest("[Test 1.3] Logical NOT", "!1", "0");

    cout << "\n--- 2. Algebraic Simplifications ---" << endl;
    runTest("[Test 2.1] a & 1      ", "a&1", "a");
    runTest("[Test 2.2] a | 0      ", "a|0", "a");
    runTest("[Test 2.3] Double NOT ", "!!a", "a");

    cout << "\n--- 3. Complex Nested Expressions ---" << endl;
    runTest("[Test 3.1] Nested     ", "(a&0)|(b&1)", "b");

    cout << "\n--- 4. Variable Substitution ---" << endl;
    map<char, int> testVars = { {'x', 1}, {'y', 0} };
    runSubstTest("[Test 4.1] Sub x=1,y=0", "x&y", testVars, "0");

    cout << "\n--- 5. Advanced Logic (Refactoring Targets) ---" << endl;
    runTest("[Test 5.1] a & a = a  ", "a&a", "a");
    runTest("[Test 5.2] a | a = a  ", "a|a", "a");
    runTest("[Test 5.3] a & !a = 0 ", "a&!a", "0");
    runTest("[Test 5.4] a | !a = 1 ", "a|!a", "1");
    runTest("[Test 5.5] Absorption ", "a|(a&b)", "a");

    cout << "\n========================================" << endl;
    cout << "       TEST EXECUTION COMPLETED         " << endl;
    cout << "========================================\n\n" << endl;
}

int main() {
    runUnitTests();

    string inputExpr;
    cout << "Enter expression (e.g., (a&0)|b): ";
    cin >> inputExpr;

    Parser parser;
    Expression* tree = parser.parse(inputExpr);

    cout << "Tree -> Expression: ";
    tree->print(); cout << endl;

    Expression* simplified = tree->simplify();
    cout << "Tree -> Simplified: ";
    simplified->print(); cout << endl;

    set<char> vars;
    simplified->findVariables(vars);

    map<char, int> varValues;
    for (char v : vars) {
        cout << "Enter value for " << v << " (0 or 1): ";
        int val;
        cin >> val;
        varValues[v] = val;
    }

    if (!vars.empty()) {
        Expression* substituted = simplified->substitute(varValues);
        cout << "Tree -> After substitution: ";
        substituted->print(); cout << endl;

        Expression* finalSimp = substituted->simplify();
        cout << "Tree -> Final simplified: ";
        finalSimp->print(); cout << endl;

        delete substituted;
        delete finalSimp;
    }

    delete tree;
    delete simplified;

    return 0;
}