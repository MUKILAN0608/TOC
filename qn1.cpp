#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <regex>
#include <algorithm>

std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

int main() {
    std::vector<std::string> code = {
        "x = 2 * 8",
        "y = x * 1",
        "z = y + 0"
    };
    
    std::unordered_map<std::string, std::string> values;
    std::vector<std::string> optimized;
    
    // Step 1: Constant folding and strength reduction
    for (auto &line : code) {
        line = trim(line);
        if (line.empty()) continue;
        
        std::regex assign_re(R"((\w+)\s*=\s*(.+))");
        std::smatch match;
        
        if (std::regex_match(line, match, assign_re)) {
            std::string lhs = match[1];
            std::string rhs = trim(match[2]);
            
            // Replace variables with their known values
            for (auto &pair : values) {
                std::regex var_pattern("\\b" + pair.first + "\\b");
                rhs = std::regex_replace(rhs, var_pattern, pair.second);
            }
            
            rhs = trim(rhs);
            
            // Constant folding: evaluate "num op num"
            std::regex const_expr(R"((\d+)\s*([\+\-\*/])\s*(\d+))");
            std::smatch cm;
            while (std::regex_search(rhs, cm, const_expr)) {
                int a = std::stoi(cm[1]);
                int b = std::stoi(cm[3]);
                char op = cm[2].str()[0];
                int result = 0;
                
                switch (op) {
                    case '+': result = a + b; break;
                    case '-': result = a - b; break;
                    case '*': result = a * b; break;
                    case '/': if (b != 0) result = a / b; break;
                }
                
                rhs = std::regex_replace(rhs, const_expr, std::to_string(result), std::regex_constants::format_first_only);
                rhs = trim(rhs);
            }
            
            // Strength reduction
            std::regex mult_one(R"((\w+|\d+)\s*\*\s*1\b)");
            rhs = std::regex_replace(rhs, mult_one, "$1");
            
            std::regex one_mult(R"(\b1\s*\*\s*(\w+|\d+))");
            rhs = std::regex_replace(rhs, one_mult, "$1");
            
            std::regex add_zero(R"((\w+|\d+)\s*\+\s*0\b)");
            rhs = std::regex_replace(rhs, add_zero, "$1");
            
            std::regex zero_add(R"(\b0\s*\+\s*(\w+|\d+))");
            rhs = std::regex_replace(rhs, zero_add, "$1");
            
            std::regex sub_zero(R"((\w+|\d+)\s*-\s*0\b)");
            rhs = std::regex_replace(rhs, sub_zero, "$1");
            
            rhs = trim(rhs);
            
            values[lhs] = rhs;
            optimized.push_back(lhs + " = " + rhs);
        }
    }
    
    // Step 2: Dead code elimination
    std::unordered_map<std::string, bool> used;
    std::vector<std::string> finalCode;
    
    // Mark the last variable as used
    if (!optimized.empty()) {
        std::regex assign_re(R"((\w+)\s*=\s*(.+))");
        std::smatch match;
        if (std::regex_match(optimized.back(), match, assign_re)) {
            used[match[1]] = true;
        }
    }
    
    // Backwards pass to find all used variables
    for (int i = (int)optimized.size() - 1; i >= 0; --i) {
        std::regex assign_re(R"((\w+)\s*=\s*(.+))");
        std::smatch match;
        
        if (std::regex_match(optimized[i], match, assign_re)) {
            std::string lhs = match[1];
            std::string rhs = match[2];
            
            if (used[lhs]) {
                finalCode.push_back(optimized[i]);
                
                // Mark variables in RHS as used
                std::regex var_re(R"(\b[a-zA-Z_]\w*\b)");
                for (auto it = std::sregex_iterator(rhs.begin(), rhs.end(), var_re);
                     it != std::sregex_iterator(); ++it) {
                    used[it->str()] = true;
                }
            }
        }
    }
    
    std::reverse(finalCode.begin(), finalCode.end());
    
    // Output
    std::cout << "Optimized code:\n";
    for (auto &line : finalCode) {
        std::cout << line << "\n";
    }
    
    return 0;
}
