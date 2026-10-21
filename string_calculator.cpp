// EECS 348 Lab 8 - String-Based Calculator
// Validates and adds numbers that are kept as strings the whole time.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std; //simplifies

// A number split into pieces so it is easy to work with.
struct Number {
    bool negative; //True if the number is negative
    string intPart;   // digits before the decimal point
    string fracPart;  // digits after the decimal point (may be empty)
};

bool isDigit(char c) {
    return c >= '0' && c <= '9'; //within this range will be a valid digit
}

// ---------------------------------------------------------------
// Step 1: Validation. Format is (+|-)A(.B)
// ---------------------------------------------------------------
bool isValidDouble(const string& s) {
    size_t i = 0;

    // optional sign
    if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
        i++; 
    }

    // at least one digit before the decimal point
    size_t intDigits = 0;
    while (i < s.size() && isDigit(s[i])) {
        i++;
        intDigits++;
    }
    if (intDigits == 0) return false;

    // nothing else? then it's a valid integer like "123"
    if (i == s.size()) return true;

    // otherwise the next character must be a decimal point
    if (s[i] != '.') return false;
    i++;

    // at least one digit after the decimal point
    size_t fracDigits = 0;
    while (i < s.size() && isDigit(s[i])) {
        i++;
        fracDigits++;
    }
    if (fracDigits == 0) return false;

    // we must have used up the whole string (rejects "2.3.4", "1e3", ...)
    return i == s.size();
}

// ---------------------------------------------------------------
// Step 2: Split a VALID string into sign / integer part / fraction part
// ---------------------------------------------------------------
Number parseNumber(const string& s) {
    Number n;
    n.negative = false;
    size_t i = 0;

    if (s[0] == '+' || s[0] == '-') {
        n.negative = (s[0] == '-');
        i = 1;
    }

    while (i < s.size() && s[i] != '.') {
        n.intPart += s[i];
        i++;
    }

    if (i < s.size()) {                 // we stopped at a '.'
        n.fracPart = s.substr(i + 1);
    }
    return n;
}

// Pad both numbers with zeros so they have the same number of digits
// before and after the decimal point. Then the decimal points line up
// and we can treat them like two big whole numbers.
void alignNumbers(Number& a, Number& b) {
    while (a.fracPart.size() < b.fracPart.size()) a.fracPart += '0';
    while (b.fracPart.size() < a.fracPart.size()) b.fracPart += '0';

    while (a.intPart.size() < b.intPart.size()) a.intPart = "0" + a.intPart;
    while (b.intPart.size() < a.intPart.size()) b.intPart = "0" + b.intPart;
}

// ---------------------------------------------------------------
// Step 3: Grade-school math on digit strings of EQUAL length
// ---------------------------------------------------------------

// a + b, both non-negative digit strings of the same length
string addDigits(const string& a, const string& b) {
    string result = "";
    int carry = 0;
    for (int i = (int)a.size() - 1; i >= 0; i--) {
        int sum = (a[i] - '0') + (b[i] - '0') + carry;
        result = string(1, (char)('0' + sum % 10)) + result;
        carry = sum / 10;
    }
    if (carry > 0) result = "1" + result;
    return result;
}

// a - b, same length, and a >= b (so the answer is never negative)
string subDigits(const string& a, const string& b) {
    string result = "";
    int borrow = 0;
    for (int i = (int)a.size() - 1; i >= 0; i--) {
        int diff = (a[i] - '0') - (b[i] - '0') - borrow;
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result = string(1, (char)('0' + diff)) + result;
    }
    return result;
}

// ---------------------------------------------------------------
// Step 4: Turn the raw digits back into a nice-looking number
// ---------------------------------------------------------------
string formatResult(string digits, size_t fracLen, bool negative) {
    // make sure there is at least one digit before the decimal point
    while (digits.size() <= fracLen) digits = "0" + digits;

    string intPart  = digits.substr(0, digits.size() - fracLen);
    string fracPart = digits.substr(digits.size() - fracLen);

    // remove leading zeros (but keep at least one digit)
    size_t start = 0;
    while (start < intPart.size() - 1 && intPart[start] == '0') start++;
    intPart = intPart.substr(start);

    // remove trailing zeros from the fraction
    while (!fracPart.empty() && fracPart[fracPart.size() - 1] == '0') {
        fracPart.erase(fracPart.size() - 1);
    }

    bool isZero = (intPart == "0" && fracPart.empty());

    string result = intPart;
    if (!fracPart.empty()) result += "." + fracPart;
    if (negative && !isZero) result = "-" + result;   // no "-0"
    return result;
}

// Add two valid number strings and return the sum as a string.
string addNumbers(const string& x, const string& y) {
    Number a = parseNumber(x);
    Number b = parseNumber(y);
    alignNumbers(a, b);

    // glue integer and fraction digits together: "12" + "50" -> "1250"
    string da = a.intPart + a.fracPart;
    string db = b.intPart + b.fracPart;
    size_t fracLen = a.fracPart.size();

    string digits;
    bool negative;

    if (a.negative == b.negative) {
        // same sign: add the sizes, keep the sign
        digits = addDigits(da, db);
        negative = a.negative;
    } else {
        // different signs: subtract smaller size from larger size,
        // and the answer takes the sign of the larger one.
        // (equal-length digit strings compare correctly as text)
        if (da >= db) {
            digits = subDigits(da, db);
            negative = a.negative;
        } else {
            digits = subDigits(db, da);
            negative = b.negative;
        }
    }
    return formatResult(digits, fracLen, negative);
}

// ---------------------------------------------------------------
// Step 5: Read the file and process each line
// ---------------------------------------------------------------
int main(int argc, char* argv[]) {
    string filename;
    if (argc > 1) {
        filename = argv[1];
    } else {
        cout << "Enter the input file name: ";
        cin >> filename;
    }

    ifstream file(filename.c_str());
    if (!file) {
        cout << "ERROR: could not open file \"" << filename << "\"" << endl;
        return 1;
    }

    string line;
    int lineNumber = 0;

    while (getline(file, line)) {
        lineNumber++;
        cout << "line " << lineNumber << ": ";

        // split the line into whitespace-separated tokens
        vector<string> tokens;
        istringstream iss(line);
        string token;
        while (iss >> token) {
            tokens.push_back(token);
        }

        if (tokens.size() != 2) {
            cout << "ERROR: expected exactly two numbers" << endl;
            continue;
        }

        bool ok1 = isValidDouble(tokens[0]);
        bool ok2 = isValidDouble(tokens[1]);

        if (!ok1 || !ok2) {
            cout << "ERROR: invalid number";
            if (!ok1) cout << " \"" << tokens[0] << "\"";
            if (!ok2) cout << " \"" << tokens[1] << "\"";
            cout << endl;
            continue;
        }

        cout << tokens[0] << " + " << tokens[1] << " = "
             << addNumbers(tokens[0], tokens[1]) << endl;
    }

    return 0;
}