#include <iostream>
#include <vector>
#include <string>

class BrainfuckInterpreter {
public:
    void interpret(const std::string& code, const std::string& input = "") {
        std::vector<unsigned char> tape(30000, 0); // Memory tape with 30,000 cells initialized to zero
        int ptr = 0;                               // Memory pointer index
        int input_ptr = 0;                         // Input pointer index

        for (int pc = 0; pc < code.size(); ++pc) {
            switch (code[pc]) {
                case '>':
                    ptr = (ptr + 1) % tape.size();
                    break;
                case '<':
                    ptr = (ptr - 1 + tape.size()) % tape.size();
                    break;
                case '+':
                    ++tape[ptr];
                    break;
                case '-':
                    --tape[ptr];
                    break;
                case '.':
                    std::cout << static_cast<char>(tape[ptr]);
                    break;
                case ',':
                    if (input_ptr < input.size()) {
                        tape[ptr] = input[input_ptr++];
                    } else {
                        tape[ptr] = 0; // No more input, default to 0
                    }
                    break;
                case '[':
                    if (tape[ptr] == 0) {
                        int loop = 1;
                        while (loop > 0) {
                            ++pc;
                            if (code[pc] == '[') loop++;
                            else if (code[pc] == ']') loop--;
                        }
                    }
                    break;
                case ']':
                    if (tape[ptr] != 0) {
                        int loop = 1;
                        while (loop > 0) {
                            --pc;
                            if (code[pc] == '[') loop--;
                            else if (code[pc] == ']') loop++;
                        }
                    }
                    break;
            }
        }
    }
};

int main() {
    std::string code = "++++++++[>++++++++<-]>+.";
    std::string input = ""; // No input in this example
    BrainfuckInterpreter interpreter;
    interpreter.interpret(code, input); // Output: A
    return 0;
}
