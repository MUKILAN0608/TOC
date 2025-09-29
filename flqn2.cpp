#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <queue>
#include <algorithm>
#include <iomanip>

using namespace std;

// NFA State
struct NFAState {
    int id;
    map<char, set<int>> transitions;
    set<int> epsilonTransitions;
    bool isAccepting;
    
    NFAState(int id) : id(id), isAccepting(false) {}
};

// NFA
class NFA {
public:
    vector<NFAState> states;
    int startState;
    set<int> acceptStates;
    set<char> alphabet;
    
    NFA() : startState(0) {}
    
    int addState() {
        int id = states.size();
        states.push_back(NFAState(id));
        return id;
    }
    
    void addTransition(int from, char symbol, int to) {
        states[from].transitions[symbol].insert(to);
        if (symbol != '\0') alphabet.insert(symbol);
    }
    
    void addEpsilonTransition(int from, int to) {
        states[from].epsilonTransitions.insert(to);
    }
    
    void setAccepting(int state) {
        states[state].isAccepting = true;
        acceptStates.insert(state);
    }
};

// DFA
class DFA {
public:
    map<set<int>, int> stateMap;
    vector<set<int>> dfaStates;
    map<int, map<char, int>> transitions;
    int startState;
    set<int> acceptStates;
    set<char> alphabet;
    
    void print() {
        cout << "\nDFA Transition Table:" << endl;
        cout << string(50, '-') << endl;
        
        cout << setw(10) << "State" << " | ";
        for (char c : alphabet) {
            cout << setw(8) << c << " | ";
        }
        cout << setw(10) << "Accept" << endl;
        cout << string(50, '-') << endl;
        
        for (size_t i = 0; i < dfaStates.size(); i++) {
            if (i == startState) cout << "-> ";
            else cout << "   ";
            
            cout << setw(7) << i << " | ";
            
            for (char c : alphabet) {
                if (transitions[i].count(c)) {
                    cout << setw(8) << transitions[i][c] << " | ";
                } else {
                    cout << setw(8) << "-" << " | ";
                }
            }
            
            if (acceptStates.count(i)) {
                cout << setw(10) << "YES";
            } else {
                cout << setw(10) << "NO";
            }
            cout << endl;
        }
        cout << string(50, '-') << endl;
    }
    
    bool validate(const string& str) {
        int currentState = startState;
        
        for (char c : str) {
            if (alphabet.find(c) == alphabet.end()) {
                return false;
            }
            
            if (transitions[currentState].count(c)) {
                currentState = transitions[currentState][c];
            } else {
                return false;
            }
        }
        
        return acceptStates.count(currentState) > 0;
    }
};

class RegexToDFA {
private:
    string regex;
    
    set<int> epsilonClosure(const set<int>& states, const NFA& nfa) {
        set<int> closure = states;
        queue<int> q;
        
        for (int s : states) q.push(s);
        
        while (!q.empty()) {
            int state = q.front();
            q.pop();
            
            for (int eps : nfa.states[state].epsilonTransitions) {
                if (closure.find(eps) == closure.end()) {
                    closure.insert(eps);
                    q.push(eps);
                }
            }
        }
        
        return closure;
    }
    
    set<int> move(const set<int>& states, char symbol, const NFA& nfa) {
        set<int> result;
        
        for (int s : states) {
            if (nfa.states[s].transitions.count(symbol)) {
                for (int next : nfa.states[s].transitions.at(symbol)) {
                    result.insert(next);
                }
            }
        }
        
        return result;
    }
    
    NFA charNFA(char c) {
        NFA nfa;
        int start = nfa.addState();
        int end = nfa.addState();
        nfa.addTransition(start, c, end);
        nfa.startState = start;
        nfa.setAccepting(end);
        return nfa;
    }
    
    NFA concatenate(NFA nfa1, NFA nfa2) {
        NFA result = nfa1;
        
        int offset = result.states.size();
        for (const auto& state : nfa2.states) {
            result.addState();
        }
        
        for (size_t i = 0; i < nfa2.states.size(); i++) {
            for (const auto& trans : nfa2.states[i].transitions) {
                for (int to : trans.second) {
                    result.addTransition(i + offset, trans.first, to + offset);
                }
            }
            for (int eps : nfa2.states[i].epsilonTransitions) {
                result.addEpsilonTransition(i + offset, eps + offset);
            }
        }
        
        for (int acc : nfa1.acceptStates) {
            result.states[acc].isAccepting = false;
            result.addEpsilonTransition(acc, nfa2.startState + offset);
        }
        
        result.acceptStates.clear();
        for (int acc : nfa2.acceptStates) {
            result.setAccepting(acc + offset);
        }
        
        return result;
    }
    
    NFA alternate(NFA nfa1, NFA nfa2) {
        NFA result;
        int start = result.addState();
        result.startState = start;
        
        int offset1 = result.states.size();
        for (const auto& state : nfa1.states) {
            result.addState();
        }
        
        for (size_t i = 0; i < nfa1.states.size(); i++) {
            for (const auto& trans : nfa1.states[i].transitions) {
                for (int to : trans.second) {
                    result.addTransition(i + offset1, trans.first, to + offset1);
                }
            }
            for (int eps : nfa1.states[i].epsilonTransitions) {
                result.addEpsilonTransition(i + offset1, eps + offset1);
            }
        }
        
        int offset2 = result.states.size();
        for (const auto& state : nfa2.states) {
            result.addState();
        }
        
        for (size_t i = 0; i < nfa2.states.size(); i++) {
            for (const auto& trans : nfa2.states[i].transitions) {
                for (int to : trans.second) {
                    result.addTransition(i + offset2, trans.first, to + offset2);
                }
            }
            for (int eps : nfa2.states[i].epsilonTransitions) {
                result.addEpsilonTransition(i + offset2, eps + offset2);
            }
        }
        
        int end = result.addState();
        
        result.addEpsilonTransition(start, nfa1.startState + offset1);
        result.addEpsilonTransition(start, nfa2.startState + offset2);
        
        for (int acc : nfa1.acceptStates) {
            result.addEpsilonTransition(acc + offset1, end);
        }
        for (int acc : nfa2.acceptStates) {
            result.addEpsilonTransition(acc + offset2, end);
        }
        
        result.setAccepting(end);
        return result;
    }
    
    NFA star(NFA nfa) {
        NFA result;
        int start = result.addState();
        result.startState = start;
        
        int offset = result.states.size();
        for (const auto& state : nfa.states) {
            result.addState();
        }
        
        for (size_t i = 0; i < nfa.states.size(); i++) {
            for (const auto& trans : nfa.states[i].transitions) {
                for (int to : trans.second) {
                    result.addTransition(i + offset, trans.first, to + offset);
                }
            }
            for (int eps : nfa.states[i].epsilonTransitions) {
                result.addEpsilonTransition(i + offset, eps + offset);
            }
        }
        
        int end = result.addState();
        
        result.addEpsilonTransition(start, nfa.startState + offset);
        result.addEpsilonTransition(start, end);
        
        for (int acc : nfa.acceptStates) {
            result.addEpsilonTransition(acc + offset, end);
            result.addEpsilonTransition(acc + offset, nfa.startState + offset);
        }
        
        result.setAccepting(end);
        return result;
    }
    
    NFA parseRegex(const string& regex) {
        stack<NFA> nfaStack;
        stack<char> opStack;
        
        for (size_t i = 0; i < regex.length(); i++) {
            char c = regex[i];
            
            if (isalnum(c)) {
                nfaStack.push(charNFA(c));
            } else if (c == '(') {
                opStack.push(c);
            } else if (c == ')') {
                while (!opStack.empty() && opStack.top() != '(') {
                    processOperator(opStack.top(), nfaStack);
                    opStack.pop();
                }
                if (!opStack.empty()) opStack.pop();
            } else if (c == '|') {
                while (!opStack.empty() && opStack.top() != '(') {
                    processOperator(opStack.top(), nfaStack);
                    opStack.pop();
                }
                opStack.push(c);
            } else if (c == '*') {
                if (!nfaStack.empty()) {
                    NFA nfa = nfaStack.top();
                    nfaStack.pop();
                    nfaStack.push(star(nfa));
                }
            }
        }
        
        while (!opStack.empty()) {
            processOperator(opStack.top(), nfaStack);
            opStack.pop();
        }
        
        while (nfaStack.size() > 1) {
            NFA nfa2 = nfaStack.top(); nfaStack.pop();
            NFA nfa1 = nfaStack.top(); nfaStack.pop();
            nfaStack.push(concatenate(nfa1, nfa2));
        }
        
        return nfaStack.top();
    }
    
    void processOperator(char op, stack<NFA>& nfaStack) {
        if (op == '|' && nfaStack.size() >= 2) {
            NFA nfa2 = nfaStack.top(); nfaStack.pop();
            NFA nfa1 = nfaStack.top(); nfaStack.pop();
            nfaStack.push(alternate(nfa1, nfa2));
        }
    }
    
public:
    DFA convert(const string& regex) {
        this->regex = regex;
        
        NFA nfa = parseRegex(regex);
        
        DFA dfa;
        dfa.alphabet = nfa.alphabet;
        
        set<int> startClosure = epsilonClosure({nfa.startState}, nfa);
        
        queue<set<int>> unmarked;
        unmarked.push(startClosure);
        dfa.stateMap[startClosure] = 0;
        dfa.dfaStates.push_back(startClosure);
        dfa.startState = 0;
        
        for (int s : startClosure) {
            if (nfa.acceptStates.count(s)) {
                dfa.acceptStates.insert(0);
                break;
            }
        }
        
        while (!unmarked.empty()) {
            set<int> current = unmarked.front();
            unmarked.pop();
            
            int currentDFAState = dfa.stateMap[current];
            
            for (char symbol : nfa.alphabet) {
                set<int> moveResult = move(current, symbol, nfa);
                set<int> nextClosure = epsilonClosure(moveResult, nfa);
                
                if (!nextClosure.empty()) {
                    if (dfa.stateMap.find(nextClosure) == dfa.stateMap.end()) {
                        int newStateId = dfa.dfaStates.size();
                        dfa.stateMap[nextClosure] = newStateId;
                        dfa.dfaStates.push_back(nextClosure);
                        unmarked.push(nextClosure);
                        
                        for (int s : nextClosure) {
                            if (nfa.acceptStates.count(s)) {
                                dfa.acceptStates.insert(newStateId);
                                break;
                            }
                        }
                    }
                    
                    dfa.transitions[currentDFAState][symbol] = dfa.stateMap[nextClosure];
                }
            }
        }
        
        return dfa;
    }
};

int main() {
    RegexToDFA converter;
    
    // Example: (0|1)*1(0|1)(0|1) - binary strings with 1 at third position from end
    string regex = "(0|1)*1(0|1)(0|1)";
    
    cout << "Regular Expression: " << regex << endl;
    cout << "Description: Binary strings with '1' at third position from the end" << endl;
    
    DFA dfa = converter.convert(regex);
    dfa.print();
    
    // Test strings
    vector<string> testStrings = {"1101", "111", "0001"};
    
    cout << "\nString Validation Results:" << endl;
    cout << string(30, '-') << endl;
    
    for (const string& str : testStrings) {
        bool result = dfa.validate(str);
        cout << setw(10) << str << " : " << (result ? "ACCEPTED" : "REJECTED") << endl;
    }
    cout << string(30, '-') << endl;
    
    // Additional test cases
    cout << "\nAdditional Test Cases:" << endl;
    cout << string(30, '-') << endl;
    vector<string> moreTests = {"100", "1000", "101", "010", "1111", "00100"};
    for (const string& str : moreTests) {
        bool result = dfa.validate(str);
        cout << setw(10) << str << " : " << (result ? "ACCEPTED" : "REJECTED") << endl;
    }
    
    return 0;
}
