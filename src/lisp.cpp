#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <sstream>

class lisp_object {
public:
    virtual ~lisp_object() = default;
    virtual std::string toString() const = 0;
};

using lisp_object_ptr = std::shared_ptr<lisp_object>;

class lisp_number : public lisp_object {
    double value;

public:
    explicit lisp_number(double val) : value(val) {}
    double getValue() const { return value; }

    std::string toString() const override {
        return std::to_string(value);
    }
};

class lisp_symbol : public lisp_object {
    std::string name;

public:
    explicit lisp_symbol(const std::string& name) : name(name) {}

    std::string getName() const { return name; }

    std::string toString() const override {
        return name;
    }
};

class lisp_list : public lisp_object {
    std::vector<lisp_object_ptr> elements;

public:
    explicit lisp_list(const std::vector<lisp_object_ptr>& elems) : elements(elems) {}

    const std::vector<lisp_object_ptr>& getElements() const { return elements; }

    std::string toString() const override {
        std::ostringstream oss;
        oss << "(";
        for (size_t i = 0; i < elements.size(); ++i) {
            oss << elements[i]->toString();
            if (i < elements.size() - 1) {
                oss << " ";
            }
        }
        oss << ")";
        return oss.str();
    }
};

class parser {
    std::string input;
    size_t index;

public:
    explicit parser(const std::string& input) : input(input), index(0) {}

private:
    char currentChar() {
        return input[index];
    }

    void skipWhitespace() {
        while (index < input.length() && isspace(currentChar())) {
            index++;
        }
    }

public:
    lisp_object_ptr parse() {
        skipWhitespace();
        if (index >= input.length()) {
            throw std::runtime_error("Unexpected end of input");
        }

        char c = currentChar();

        if (c == '(') {
            index++;
            std::vector<lisp_object_ptr> elements;
            while (currentChar() != ')') {
                elements.push_back(parse());
                skipWhitespace();
            }
            index++;
            return std::make_shared<lisp_list>(elements);
        }
        else if (isdigit(c) || (c == '-' && index + 1 < input.length() && isdigit(input[index + 1]))) {
            std::string number;
            while (index < input.length() && (isdigit(currentChar()) || currentChar() == '.')) {
                number += currentChar();
                index++;
            }
            return std::make_shared<lisp_number>(std::stod(number));
        }
        else {
            std::string symbol;
            while (index < input.length() && !isspace(currentChar()) && currentChar() != ')') {
                symbol += currentChar();
                index++;
            }
            return std::make_shared<lisp_symbol>(symbol);
        }
    }
};


class interpreter {
    std::map<std::string, lisp_object_ptr> globalEnv;

public:
    interpreter() {
        globalEnv["+"] = std::make_shared<lisp_symbol>("+");
        globalEnv["-"] = std::make_shared<lisp_symbol>("-");
        globalEnv["*"] = std::make_shared<lisp_symbol>("*");
        globalEnv["/"] = std::make_shared<lisp_symbol>("/");
    }

    lisp_object_ptr eval(lisp_object_ptr expr) {
        if (auto number = dynamic_cast<lisp_number*>(expr.get())) {
            return expr;
        }
        else if (auto symbol = dynamic_cast<lisp_symbol*>(expr.get())) {
            return globalEnv[symbol->getName()];
        }
        else if (auto list = dynamic_cast<lisp_list*>(expr.get())) {
            const auto& elements = list->getElements();
            if (elements.empty()) {
                return nullptr;
            }

            auto operatorSymbol = dynamic_cast<lisp_symbol*>(elements[0].get());
            std::vector<lisp_object_ptr> args(elements.begin() + 1, elements.end());

            if (operatorSymbol->getName() == "+") {
                return evalAdd(args);
            }
            else if (operatorSymbol->getName() == "-") {
                return evalSubtract(args);
            }
            else if (operatorSymbol->getName() == "*") {
                return evalMultiply(args);
            }
            else if (operatorSymbol->getName() == "/") {
                return evalDivide(args);
            }
            else {
                throw std::runtime_error("Unknown operator: " + operatorSymbol->getName());
            }
        }
        throw std::runtime_error("Unknown expression");
    }

private:
    lisp_object_ptr evalAdd(const std::vector<lisp_object_ptr>& args) {
        double sum = 0;
        for (const auto& arg : args) {
            sum += dynamic_cast<lisp_number*>(eval(arg).get())->getValue();
        }
        return std::make_shared<lisp_number>(sum);
    }

    lisp_object_ptr evalSubtract(const std::vector<lisp_object_ptr>& args) {
        double result = dynamic_cast<lisp_number*>(eval(args[0]).get())->getValue();
        for (size_t i = 1; i < args.size(); i++) {
            result -= dynamic_cast<lisp_number*>(eval(args[i]).get())->getValue();
        }
        return std::make_shared<lisp_number>(result);
    }

    lisp_object_ptr evalMultiply(const std::vector<lisp_object_ptr>& args) {
        double product = 1;
        for (const auto& arg : args) {
            product *= dynamic_cast<lisp_number*>(eval(arg).get())->getValue();
        }
        return std::make_shared<lisp_number>(product);
    }

    lisp_object_ptr evalDivide(const std::vector<lisp_object_ptr>& args) {
        double result = dynamic_cast<lisp_number*>(eval(args[0]).get())->getValue();
        for (size_t i = 1; i < args.size(); i++) {
            result /= dynamic_cast<lisp_number*>(eval(args[i]).get())->getValue();
        }
        return std::make_shared<lisp_number>(result);
    }
};


int main() {
    std::string input = "(+ 1 2 (* 3 4))";
    parser parser(input);
    interpreter interpreter;

    try {
        auto parsedExpression = parser.parse();
        auto result = interpreter.eval(parsedExpression);
        std::cout << "Result: " << result->toString() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}