#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <cctype>
#include <sstream>

using namespace std;

class StackCodeGenerator {
private:
    vector<string> code;
    
    // Tokenize the expression
    vector<string> tokenize(const string& expr) {
        vector<string> tokens;
        string token;
        
        for (size_t i = 0; i < expr.length(); i++) {
            char c = expr[i];
            
            if (isspace(c)) {
                continue;
            } else if (isalnum(c) || c == '_') {
                token += c;
                // Check if next char is also part of identifier
                if (i + 1 >= expr.length() || 
                    (!isalnum(expr[i+1]) && expr[i+1] != '_')) {
                    tokens.push_back(token);
                    token.clear();
                }
            } else if (c == '+' || c == '-' || c == '*' || c == '/' || 
                       c == '(' || c == ')') {
                tokens.push_back(string(1, c));
            }
        }
        
        return tokens;
    }
    
    // Get operator precedence
    int precedence(const string& op) {
        if (op == "+" || op == "-") return 1;
        if (op == "*" || op == "/") return 2;
        return 0;
    }
    
    // Check if token is an operator
    bool isOperator(const string& token) {
        return token == "+" || token == "-" || token == "*" || token == "/";
    }
    
    // Convert infix to postfix using Shunting Yard algorithm
    vector<string> infixToPostfix(const vector<string>& tokens) {
        vector<string> output;
        stack<string> opStack;
        
        for (const string& token : tokens) {
            if (isalnum(token[0]) || token[0] == '_') {
                // Operand (variable or number)
                output.push_back(token);
            } else if (isOperator(token)) {
                // Operator
                while (!opStack.empty() && opStack.top() != "(" &&
                       precedence(opStack.top()) >= precedence(token)) {
                    output.push_back(opStack.top());
                    opStack.pop();
                }
                opStack.push(token);
            } else if (token == "(") {
                opStack.push(token);
            } else if (token == ")") {
                while (!opStack.empty() && opStack.top() != "(") {
                    output.push_back(opStack.top());
                    opStack.pop();
                }
                if (!opStack.empty()) {
                    opStack.pop(); // Remove '('
                }
            }
        }
        
        while (!opStack.empty()) {
            output.push_back(opStack.top());
            opStack.pop();
        }
        
        return output;
    }
    
public:
    // Generate stack machine code from arithmetic expression
    string generateCode(const string& expr) {
        code.clear();
        
        // Tokenize and convert to postfix
        vector<string> tokens = tokenize(expr);
        vector<string> postfix = infixToPostfix(tokens);
        
        // Generate code from postfix notation
        for (const string& token : postfix) {
            if (isOperator(token)) {
                // Generate operation instruction
                if (token == "+") code.push_back("ADD");
                else if (token == "-") code.push_back("SUB");
                else if (token == "*") code.push_back("MUL");
                else if (token == "/") code.push_back("DIV");
            } else {
                // Generate push instruction for operand
                code.push_back("PUSH " + token);
            }
        }
        
        // Convert to string output
        stringstream ss;
        for (size_t i = 0; i < code.size(); i++) {
            ss << code[i];
            if (i < code.size() - 1) ss << "\n";
        }
        
        return ss.str();
    }
};

int main() {
    StackCodeGenerator generator;
    
    // Test cases
    vector<string> testCases = {
        "(a+b)*c",
        "a+b*c",
        "(a+b)*(c+d)",
        "a*b+c*d",
        "((a+b)*c)-d"
    };
    
    for (const string& expr : testCases) {
        cout << "Input: " << expr << endl;
        cout << "Output:" << endl;
        cout << generator.generateCode(expr) << endl;
        cout << endl;
    }
    
    return 0;
}
