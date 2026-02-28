#include <iostream>
#include <string>
#include <cassert>

using namespace std;

typedef struct BTN {
    int dat;
    BTN* lt, * rt;
} BINTRN, * BINTRP;

int currentChar = ' ';
string var;
int* varValues = nullptr;
bool* foundVars = nullptr;

void nsym();
BINTRP inpexp();
BINTRP nwnode(int, BINTRP, BINTRP);
BINTRP expr();
BINTRP factor();
BINTRP simpl(BINTRP);
int numb();
void prnexp(BINTRP);
void deltr(BINTRP);
void substituteVariables(BINTRP p);
void findVariables(BINTRP p);
void runUnitTests();

int main() {
    runUnitTests();

    BINTRP t, p;

    cout << "Enter the variables in the expression: ";
    cin >> var;

    varValues = new int[256];
    foundVars = new bool[256];

    cout << "Expression: ";
    t = inpexp();
    cout << endl;

    cout << "Tree -> Expression: ";
    prnexp(t); cout << endl;

    p = simpl(t);
    cout << "Tree -> Simplified: ";
    prnexp(p); cout << endl;

    for (int i = 0; i < 256; ++i) foundVars[i] = false;
    findVariables(p);

    for (int i = 0; i < var.size(); ++i) {
        if (foundVars[(int)var[i]]) {
            cout << "Enter value for " << var[i] << ": ";
            cin >> varValues[(int)var[i]];
        }
    }

    substituteVariables(p);
    cout << "Tree -> After variable substitution: ";
    prnexp(p); cout << endl;

    p = simpl(p);
    cout << "Tree -> Final simplified: ";
    prnexp(p); cout << endl;

    delete[] varValues;
    delete[] foundVars;
    deltr(p);

    return 0;
}


void runUnitTests() {
    cout << "========================================" << endl;
    cout << "     RUNNING AUTOMATED UNIT TESTS       " << endl;
    cout << "========================================" << endl;

    varValues = new int[256]();
    foundVars = new bool[256]();

    const int T = -1;
    const int F = -0;

    cout << "\n--- 1. Basic Logical Operations ---" << endl;

    cout << "[Test 1.1] Logical AND: ";
    BINTRP and2_tree = nwnode('&', nwnode(T, nullptr, nullptr), nwnode(F, nullptr, nullptr));
    prnexp(and2_tree); cout << " => ";
    BINTRP and2 = simpl(and2_tree);
    prnexp(and2);
    assert(and2->dat == F && "1 & 0 should be 0");
    cout << "  [PASSED]" << endl;
    deltr(and2);

    cout << "[Test 1.2] Logical OR: ";
    BINTRP or2_tree = nwnode('|', nwnode(F, nullptr, nullptr), nwnode(T, nullptr, nullptr));
    prnexp(or2_tree); cout << " => ";
    BINTRP or2 = simpl(or2_tree);
    prnexp(or2);
    assert(or2->dat == T && "0 | 1 should be 1");
    cout << "  [PASSED]" << endl;
    deltr(or2);

    cout << "[Test 1.3] Logical NOT: ";
    BINTRP not1_tree = nwnode('!', nwnode(T, nullptr, nullptr), nullptr);
    prnexp(not1_tree); cout << " => ";
    BINTRP not1 = simpl(not1_tree);
    prnexp(not1);
    assert(not1->dat == F && "!1 should be 0");
    cout << "  [PASSED]" << endl;
    deltr(not1);

    cout << "\n--- 2. Algebraic Simplifications with Variables ---" << endl;

    cout << "[Test 2.1] a & 1: ";
    BINTRP rule1_tree = nwnode('&', nwnode('a', nullptr, nullptr), nwnode(T, nullptr, nullptr));
    prnexp(rule1_tree); cout << " => ";
    BINTRP rule1 = simpl(rule1_tree);
    prnexp(rule1);
    assert(rule1->dat == 'a' && "a & 1 should be a");
    cout << "  [PASSED]" << endl;
    deltr(rule1);

    cout << "[Test 2.2] a | 0: ";
    BINTRP rule4_tree = nwnode('|', nwnode('a', nullptr, nullptr), nwnode(F, nullptr, nullptr));
    prnexp(rule4_tree); cout << " => ";
    BINTRP rule4 = simpl(rule4_tree);
    prnexp(rule4);
    assert(rule4->dat == 'a' && "a | 0 should be a");
    cout << "  [PASSED]" << endl;
    deltr(rule4);

    cout << "[Test 2.3] Double Negation (!!a): ";
    BINTRP rule6_tree = nwnode('!', nwnode('!', nwnode('a', nullptr, nullptr), nullptr), nullptr);
    prnexp(rule6_tree); cout << " => ";
    BINTRP rule6 = simpl(rule6_tree);
    prnexp(rule6);
    assert(rule6->dat == 'a' && "!!a should be a");
    cout << "  [PASSED]" << endl;
    deltr(rule6);

    cout << "\n--- 3. Complex Nested Expressions ---" << endl;

    cout << "[Test 3.1] Nested simplification: ";
    BINTRP complex1_tree = nwnode('|',
        nwnode('&', nwnode('a', nullptr, nullptr), nwnode(F, nullptr, nullptr)),
        nwnode('&', nwnode('b', nullptr, nullptr), nwnode(T, nullptr, nullptr))
    );
    prnexp(complex1_tree); cout << " => ";
    BINTRP complex1 = simpl(complex1_tree);
    prnexp(complex1);
    assert(complex1->dat == 'b' && "(a & 0) | (b & 1) should be b");
    cout << "  [PASSED]" << endl;
    deltr(complex1);

    cout << "\n--- 4. Variable Substitution ---" << endl;

    cout << "[Test 4.1] Substituting x=1, y=0 in (x&y): ";
    BINTRP sub1_tree = nwnode('&', nwnode('x', nullptr, nullptr), nwnode('y', nullptr, nullptr));
    prnexp(sub1_tree); cout << " => ";

    varValues['x'] = 1;
    varValues['y'] = 0;
    substituteVariables(sub1_tree);

    prnexp(sub1_tree); cout << " => ";
    BINTRP sub1 = simpl(sub1_tree);
    prnexp(sub1);
    assert(sub1->dat == F && "1 & 0 should simplify to 0");
    cout << "  [PASSED]" << endl;
    deltr(sub1);

    cout << "\n--- 5. Advanced Boolean Logic (Refactoring Targets) ---" << endl;

    cout << "[Test 5.1] Idempotence AND: ";
    BINTRP idem_and_tree = nwnode('&', nwnode('a', nullptr, nullptr), nwnode('a', nullptr, nullptr));
    prnexp(idem_and_tree); cout << " => ";
    BINTRP idem_and = simpl(idem_and_tree);
    prnexp(idem_and);
    if (idem_and->dat == 'a') cout << "  [PASSED]" << endl;
    else cout << "  [FAILED - Expected 'a']" << endl;
    deltr(idem_and);

    cout << "[Test 5.2] Idempotence OR:  ";
    BINTRP idem_or_tree = nwnode('|', nwnode('a', nullptr, nullptr), nwnode('a', nullptr, nullptr));
    prnexp(idem_or_tree); cout << " => ";
    BINTRP idem_or = simpl(idem_or_tree);
    prnexp(idem_or);
    if (idem_or->dat == 'a') cout << "  [PASSED]" << endl;
    else cout << "  [FAILED - Expected 'a']" << endl;
    deltr(idem_or);

    cout << "[Test 5.3] Complement AND:  ";
    BINTRP comp_and_tree = nwnode('&', nwnode('a', nullptr, nullptr), nwnode('!', nwnode('a', nullptr, nullptr), nullptr));
    prnexp(comp_and_tree); cout << " => ";
    BINTRP comp_and = simpl(comp_and_tree);
    prnexp(comp_and);
    if (comp_and->dat == F) cout << "  [PASSED]" << endl;
    else cout << "  [FAILED - Expected '0']" << endl;
    deltr(comp_and);

    cout << "[Test 5.4] Complement OR:   ";
    BINTRP comp_or_tree = nwnode('|', nwnode('a', nullptr, nullptr), nwnode('!', nwnode('a', nullptr, nullptr), nullptr));
    prnexp(comp_or_tree); cout << " => ";
    BINTRP comp_or = simpl(comp_or_tree);
    prnexp(comp_or);
    if (comp_or->dat == T) cout << "  [PASSED]" << endl;
    else cout << "  [FAILED - Expected '1']" << endl;
    deltr(comp_or);

    cout << "[Test 5.5] Absorption:      ";
    BINTRP abs_tree = nwnode('|',
        nwnode('a', nullptr, nullptr),
        nwnode('&', nwnode('a', nullptr, nullptr), nwnode('b', nullptr, nullptr))
    );
    prnexp(abs_tree); cout << " => ";
    BINTRP abs_res = simpl(abs_tree);
    prnexp(abs_res);
    if (abs_res->dat == 'a') cout << "  [PASSED]" << endl;
    else cout << "  [FAILED - Expected 'a']" << endl;
    deltr(abs_res);

    cout << "\n========================================" << endl;
    cout << "       TEST EXECUTION COMPLETED         " << endl;
    cout << "========================================\n\n" << endl;

    delete[] varValues;
    delete[] foundVars;
    varValues = nullptr;
    foundVars = nullptr;
}


void nsym() {
    if (currentChar == EOF) return;
    while ((currentChar = getchar()) == ' ' || currentChar == '\n');
}

BINTRP inpexp() {
    nsym(); return expr();
}

BINTRP expr() {
    BINTRP p = factor();
    while (string("&|").find(currentChar) != string::npos) {
        char op = currentChar;
        nsym();
        p = nwnode(op, p, factor());
    }
    return p;
}

BINTRP factor() {
    if (currentChar == '!') {
        nsym();
        return nwnode('!', factor(), nullptr);
    }

    if (currentChar == '(') {
        nsym();
        BINTRP p = expr();
        if (currentChar != ')') {
            cout << "ERROR: expected ')'\n";
            return nullptr;
        }
        nsym();
        return p;
    }

    if (string(var).find(currentChar) != string::npos) {
        char v = currentChar;
        nsym();
        return nwnode(v, nullptr, nullptr);
    }

    return nwnode(-numb(), nullptr, nullptr);
}

int numb() {
    if (currentChar == '0' || currentChar == '1') {
        int val = currentChar - '0';
        nsym();
        return val;
    }
    cout << "ERROR: expected 0 or 1\n";
    return -1;
}

BINTRP nwnode(int v, BINTRP pl, BINTRP pr) {
    BINTRP p = new BINTRN;
    p->dat = v;
    p->lt = pl;
    p->rt = pr;
    return p;
}

void prnexp(BINTRP p) {
    if (!p) return;
    if (p->dat <= 0) cout << -p->dat;
    else if (p->dat == '!') {
        cout << '!' << '('; prnexp(p->lt); cout << ')';
    }
    else if (p->dat == '&' || p->dat == '|') {
        cout << '('; prnexp(p->lt); cout << (char)p->dat; prnexp(p->rt); cout << ')';
    }
    else cout << (char)p->dat;
}

BINTRP simpl(BINTRP p) {
    if (!p) return nullptr;
    if (p->dat < 0 || (p->dat >= 'a' && p->dat <= 'z')) {
        findVariables(p);
        return p;
    }

    p->lt = simpl(p->lt);
    p->rt = simpl(p->rt);
    BINTRP pl = p->lt, pr = p->rt;

    switch (p->dat) {
    case '&':
        if (pl && pl->dat == -0 || pr && pr->dat == -0) {
            deltr(pl); deltr(pr);
            return nwnode(-0, nullptr, nullptr);
        }
        if (pl && pl->dat == -1) { delete p; return pr; }
        if (pr && pr->dat == -1) { delete p; return pl; }
        break;
    case '|':
        if (pl && pl->dat == -1 || pr && pr->dat == -1) {
            deltr(pl); deltr(pr);
            return nwnode(-1, nullptr, nullptr);
        }
        if (pl && pl->dat == -0) { delete p; return pr; }
        if (pr && pr->dat == -0) { delete p; return pl; }
        break;
    case '!':
        if (pl && pl->dat == -0) { delete p; return nwnode(-1, nullptr, nullptr); }
        if (pl && pl->dat == -1) { delete p; return nwnode(-0, nullptr, nullptr); }
        if (pl && pl->dat == '!') {
            BINTRP inner = pl->lt;
            delete pl; delete p;
            return inner;
        }
        break;
    }

    findVariables(p);
    return p;
}

void deltr(BINTRP p) {
    if (!p) return;
    deltr(p->lt); deltr(p->rt); delete p;
}

void substituteVariables(BINTRP p) {
    if (!p) return;
    if (p->dat >= 'a' && p->dat <= 'z') {
        int val = varValues[p->dat];
        p->dat = val ? -1 : -0;
        return;
    }
    substituteVariables(p->lt);
    substituteVariables(p->rt);
}

void findVariables(BINTRP p) {
    if (!p) return;
    if (p->dat >= 'a' && p->dat <= 'z') {
        foundVars[p->dat] = true;
    }
    findVariables(p->lt);
    findVariables(p->rt);
}