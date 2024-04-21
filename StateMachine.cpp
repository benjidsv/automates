#include "StateMachine.hpp"
#include <filesystem>
namespace fs = std::filesystem;
#include <fstream>
#include <iostream>

const char SYMBOLS[27] = {'#','a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};


std::ostream & operator<<(std::ostream & Str, StateMachine const & v) {
    // Print letters up to alphabetLength-th letter
    Str << "           |  ";
    char letter = 'a';
    std::generate_n(std::ostream_iterator<std::string>(std::cout, "  |  "),
                    v.alphabetLength,
                    [&letter]() { return std::string(1, letter++); });
    Str << std::endl;

    // Print states
    for (const auto& s : v.states) {
        Str << (s->in ? (s->out ? " E S |" : "  E  |") : (s->out ? "  S  |" : "     |"));
        Str << "  " << s->number << "  | ";

        for (int i = 1; i <= v.alphabetLength; ++i) {
            if (s->transitions.at(SYMBOLS[i]).empty()) Str << " X  | ";
            else { // xxxxx|
                int count = s->transitions.at(SYMBOLS[i]).size();
                switch (count) {
                    case 1:
                        Str << " " << s->transitions.at(SYMBOLS[i])[0].lock()->number << "  | ";
                        break;
                    case 3:
                        Str << "" << s->transitions.at(SYMBOLS[i])[0].lock()->number << s->transitions.at(SYMBOLS[i])[1].lock()->number << " | ";
                    default:
                        for (int j = 0; j < count; ++j) {
                            Str << s->transitions.at(SYMBOLS[i])[j].lock()->number;
                        }
                        int spaces = 4 - count;
                        std::string st (" ", spaces);
                        Str << (spaces > 0 ? std::string(spaces, ' ') : "") << "| ";
                }
            }
        }

        Str << std::endl;
    }

    Str << "Synchrone : " << (v.synchronous ? "oui" : "non") << "\nDéterministe : " << (v.deterministic ? "oui" : "non") <<
    "\nStandard : " << (v.standard ? "oui" : "non") << "\nComplet : " << (v.complete ? "oui" : "non");

    return Str;
}

StateMachine::StateMachine(const std::string& filePath) {
    fs::path path = filePath;
    if (!fs::is_regular_file(path)) throw std::runtime_error("Le chemin spécifié est invalide : " + filePath);

    // Lire le fichier
    std::ifstream file(filePath);

    int stateCount, inputCount, outputCount, transitionCount;
    alphabetLength = 0;
    file >> alphabetLength;
    file >> stateCount;

    // Génération des états de 0 à stateCount
    states.reserve(stateCount);
    for (int i = 0; i < stateCount; ++i) {
        states.push_back(std::make_shared<State>(i, alphabetLength));
    }

    // Lecture des entrées
    file >> inputCount;
    inputs.reserve(inputCount);
    for (int i = 0; i < inputCount; ++i) {
        int t;
        file >> t;
        states[t]->in = true;
        inputs.push_back(t);
    }
    // Sorties
    file >> outputCount;
    outputs.reserve(outputCount);
    for (int i = 0; i < outputCount; ++i) {
        int t;
        file >> t;
        states[t]->out = true;
        outputs.push_back(t);
    }

    // Lecture des transitions
    synchronous = true;
    file >> transitionCount;
    for (int i = 0; i < transitionCount; ++i) {
        int from, to;
        char symbol;

        file >> from >> symbol >> to;
        if (symbol == '#') synchronous = false;
        states[from]->AddTransition(symbol, states[to]);
    }

    Evaluate();
}

State::State(int number, int alphabetLength) {
    this->number = number;
    this->in = false;
    this->out = false;

    for (int i = 1; i < alphabetLength + 1; ++i) {
        std::vector<std::weak_ptr<State>> t;
        transitions.emplace(SYMBOLS[i], t);
    }
}

void State::AddTransition(char symbol, std::weak_ptr<State> to)
{
    transitions[symbol].push_back(to);
}

void StateMachine::Evaluate() {
    bool onlyOneInput = inputs.size() == 1;
    bool transitionTowardsInput = false;
    bool onlyOneTransitionPerLetter = true;
    complete = true;
    int input = inputs[0];

    for (const auto & state : states) {
        for (int j = 0; j < state->transitions.size(); ++j) {
            unsigned long size = state->transitions[SYMBOLS[j]].size();
            if (size > 1) onlyOneTransitionPerLetter = false;
            if (size == 0) complete = false;

            for (int k = 0; k < size; ++k) {
                if (state->transitions[SYMBOLS[j]][k].lock()->number == input) transitionTowardsInput = true;
            }
        }
    }

    if (onlyOneInput) {
        standard = !transitionTowardsInput;
        deterministic = onlyOneTransitionPerLetter;
    }
    else {
        standard = false;
        deterministic = false;
    }
}

StateMachine::StateMachine() = default;

void StateMachine::Synchronize() {

}

void StateMachine::Standardize() {
    if (standard) return;

    auto newState = AddState();
    newState.lock()->in = true;
    std::cout << "Création de l'état initial " << newState.lock()->number << std::endl;

    for (int i = 0; i < inputs.size(); ++i) {
        if (!states[i]->in) continue;

        if(states[i]->out) newState.lock()->out = true;

        for (int j = 0; j < states[i]->transitions.size(); ++j) {
            for (int k = 0; k < states[i]->transitions[SYMBOLS[j]].size(); ++k) {
                newState.lock()->AddTransition(SYMBOLS[j], states[i]->transitions[SYMBOLS[j]][k]);
            }
        }

        states[i]->in = false;
    }

    standard = true;
}

void StateMachine::Complete() {
    if (complete) return;

    auto newState = AddState();
    std::cout << "Création de l'état poubelle " << newState.lock()->number << std::endl;

    for (const auto & state : states) {
        for (int i = 0; i < state->transitions.size(); ++i) {
            if (state->transitions[SYMBOLS[i]].empty()) {
                state->AddTransition(SYMBOLS[i], newState);
            }
        }
    }

    complete = true;
}

std::weak_ptr<State> StateMachine::AddState() {
    auto newState = std::make_shared<State>(states.size(), alphabetLength);
    states.push_back(newState);

    return newState;
}

void StateMachineContainer::Determinize() {

}
