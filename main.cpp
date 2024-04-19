#include <iostream>
#include "StateMachine.hpp"

int main() {
    std::cout << "Projet Automates - GRP F2 - Benjamin ARBOUSSET, Johan DAO DUY, Maxime GIMER" << std::endl;

    bool choiceIsValid = false;
    StateMachine s;

    while (!choiceIsValid) {
        std::cout << "Choisir un fichier d'automate : ";
        std::string choice;
        std::cin >> choice;

        try {
            s = StateMachine("/Users/benji/dev/cpp/automates/machines/F2-" + choice + ".txt");
            std::cout << s << std::endl;
            choiceIsValid = true;
        }
        catch (const std::runtime_error& e) {
            std::cout << e.what() << std::endl;
            continue;
        }
    }

    if (!s.synchronous) {
        std::cout << "L'automate n'est pas synchrone. Souhaitez vous le synchroniser ? (o/n) ";
        char choice;
        std::cin >> choice;

        if (choice == 'o') {
            s.Synchronize();
            std::cout << s << std::endl;
        }
    }

    if (!s.standard) {
        std::cout << "L'automate n'est pas standard. Souhaitez vous le standardiser ? (o/n) ";
        char choice;
        std::cin >> choice;

        if (choice == 'o') {
            s.Standardize();
            std::cout << s << std::endl;
        }
    }

    if (!s.deterministic ) {
        std::cout << "L'automate n'est pas déterministe. Souhaitez vous le déterminiser ? (o/n) ";
        char choice;
        std::cin >> choice;

        if (choice == 'o') {
            s.Determinize();
            std::cout << s << std::endl;
        }
    }

    if (!s.complete) {
        std::cout << "L'automate n'est pas complet. Souhaitez vous le compléter ? (o/n) ";
        char choice;
        std::cin >> choice;

        if (choice == 'o') {
            s.Complete();
            std::cout << s << std::endl;
        }
    }

    return 0;
}
