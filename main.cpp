#include <iostream>
#include "StateMachine.hpp"

int main() {
    while (true) {
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
            catch (const std::runtime_error &e) {
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

        if (!c.base.deterministic) {
            std::cout << "Voici l'automate déterministe et complet équivalent : " << std::endl;

            c.Determinize();
            c.deterministic.Complete();
            std::cout << c.deterministic << std::endl;
        }

        std::cout << "Souhaitez vous voir l'automate complémentaire ? (o/n)";
        char choice;
        std::cin >> choice;

        if (choice == 'o') {
            if (c.determinized) {
                std::cout << "Le complémentaire est calculé à l'aide de l'automate déterministe." << std::endl;
                std::cout << c.deterministic.Complimentary() << std::endl;
                c.deterministic.Complimentary(); // reverse
            } else {
                std::cout << "Le complémentaire est calculé à l'aide de l'automate original." << std::endl;
                if (!c.base.complete) c.base.Complete();
                std::cout << "Complétion de l'automate..." << std::endl;
                std::cout << c.base << std::endl;
                std::cout << c.base.Complimentary() << std::endl;
                c.base.Complimentary();
            }
        }

        while (true) {
            std::cout << "Test de reconnaissance (exit pour sortir): ";
            std::string input;
            std::cin >> input;
            if (input == "exit") break;

            std::cout << (c.deterministic.Test(input) ? "Le mot est reconnu" : "Le mot n'est pas reconnu") << std::endl;
        }
    }

    return 0;
}
