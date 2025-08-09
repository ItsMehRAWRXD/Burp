#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <limits>

void generateClass(std::ostream &os, const std::string &className) {
    os << "class " << className << " {\n"
       << "public:\n"
       << "    " << className << "();\n"
       << "private:\n"
       << "    // add members here\n"
       << "};\n";
}

void generateFunction(std::ostream &os, const std::string &funcName) {
    os << "void " << funcName << "() {\n"
       << "    // code here\n"
       << "}\n";
}

int main() {
    std::cout << "=== CodeBot ===\n";

    while (true) {
        std::cout << "\nChoose an option:\n"
                  << "1) Generate class template\n"
                  << "2) Generate function template\n"
                  << "3) Exit\n"
                  << "Enter choice: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cerr << "Invalid input.\n";
            return 1;
        }
        if (choice == 3) {
            std::cout << "Goodbye!\n";
            break;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::string name;
        std::cout << "Enter name: ";
        std::getline(std::cin, name);

        std::string outputFile;
        std::cout << "Enter output filename (e.g., MyFile.cpp): ";
        std::getline(std::cin, outputFile);

        std::ofstream ofs(outputFile);
        if (!ofs) {
            std::cerr << "Failed to open " << outputFile << " for writing.\n";
            continue;
        }

        if (choice == 1) {
            generateClass(ofs, name);
        } else if (choice == 2) {
            generateFunction(ofs, name);
        } else {
            std::cerr << "Unknown choice.\n";
            continue;
        }
        ofs.close();

        std::cout << "File " << outputFile << " generated successfully.\n";

        char compileChoice;
        std::cout << "Compile the generated file now? (y/n): ";
        std::cin >> compileChoice;
        if (compileChoice == 'y' || compileChoice == 'Y') {
            std::string outputBinary = outputFile.substr(0, outputFile.find_last_of('.'));
            std::string command = "g++ " + outputFile + " -o " + outputBinary;
            int ret = system(command.c_str());
            if (ret == 0) {
                std::cout << "Compilation succeeded. Run the program? (y/n): ";
                char runChoice;
                std::cin >> runChoice;
                if (runChoice == 'y' || runChoice == 'Y') {
                    std::string runCmd = "./" + outputBinary;
                    system(runCmd.c_str());
                }
            } else {
                std::cerr << "Compilation failed.\n";
            }
        }
    }

    return 0;
}