#include <iostream>
#include <string>

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