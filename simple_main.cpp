#include "SimpleBot.h"
#include <iostream>

int main(int argc, char* argv[]) {
    SimpleBot bot;
    
    if (argc > 1 && std::string(argv[1]) == "auto") {
        bot.start();
        bot.add_task(GenType::CLASS, Lang::CPP, "AutoClass", 3);
        bot.add_task(GenType::FUNC, Lang::PY, "auto_func", 2);
        bot.add_task(GenType::PROG, Lang::JS, "auto_prog", 1);
        bot.add_task(GenType::FUNC, Lang::ASM_X64, "asm_func", 1);
        
        std::this_thread::sleep_for(std::chrono::seconds(2));
        bot.stop();
        return 0;
    }
    
    std::string type, lang, name;
    std::cout << "Type (class/func/prog): ";
    std::cin >> type;
    std::cout << "Lang (cpp/py/js/rust/go/asm): ";
    std::cin >> lang;
    std::cout << "Name: ";
    std::cin >> name;
    
    GenType gt = (type == "class") ? GenType::CLASS : 
                 (type == "func") ? GenType::FUNC : GenType::PROG;
    
    Lang l = (lang == "py") ? Lang::PY :
             (lang == "js") ? Lang::JS :
             (lang == "rust") ? Lang::RUST :
             (lang == "go") ? Lang::GO :
             (lang == "asm") ? Lang::ASM_X64 : Lang::CPP;
    
    std::cout << bot.generate_now(gt, l, name);
    return 0;
}