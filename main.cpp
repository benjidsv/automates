#include <iostream>
#include "StateMachine.hpp"

int main() {
    std::cout << "Projet Automates - GRP F2 - Benjamin ARBOUSSET, Johan DAO DUY, Maxime GIMER" << std::endl;

    bool choiceIsValid = false;
    StateMachineContainer c;

    while (!choiceIsValid) {
        std::cout << "Choisir un fichier d'automate : ";
        std::string choice;
        std::cin >> choice;

        try {
            c.base = StateMachine("/Users/benji/dev/cpp/automates/machines/F2-" + choice + ".txt");
            std::cout << c.base << std::endl;
            choiceIsValid = true;
        }
        catch (const std::runtime_error& e) {
            std::cout << e.what() << std::endl;
            continue;
        }
    }

    if (!c.base.synchronous) {
        std::cout << "L'automate n'est pas synchrone. Souhaitez vous le synchroniser ? (o/n) ";
        char choice;
        std::cin >> choice;

        if (choice == 'o') {
            c.base.Synchronize();
            std::cout << c.base << std::endl;
        }
    }

    if (!c.base.standard) {
        std::cout << "L'automate n'est pas standard. Souhaitez vous le standardiser ? (o/n) ";
        char choice;
        std::cin >> choice;

        if (choice == 'o') {
            c.base.Standardize();
            std::cout << c.base << std::endl;
        }
    }

    if (!c.base.deterministic ) {
        std::cout << "L'automate n'est pas déterministe. Souhaitez vous le déterminiser ? (o/n) ";
        char choice;
        std::cin >> choice;

        if (choice == 'o') {
            c.Determinize();
            std::cout << c.deterministic << std::endl;
        }
    }

    if (!c.base.complete) {
        std::cout << "L'automate n'est pas complet. Souhaitez vouc->base le compléter ? (o/n) ";
        char choice;
        std::cin >> choice;

        if (choice == 'o') {
            c.base.Complete();
            std::cout << c.base << std::endl;
        }
    }

    return 0;
}
