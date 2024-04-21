#include "StateMachine.hpp"
#include <filesystem>
namespace fs = std::filesystem;
#include <fstream>
#include <iostream>
#include <set>

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
                        Str << "" << s->transitions.at(SYMBOLS[i])[0].lock()->number << s->transitions.at(SYMBOLS[i])[1].lock()->number << s->transitions.at(SYMBOLS[i])[2].lock()->number << " | ";
                        break;
                    default:
                        for (int j = 0; j < count; ++j) {
                            Str << s->transitions.at(SYMBOLS[i])[j].lock()->number;
                        }
                        int spaces = 4 - count;
                        std::string st (" ", spaces);
                        Str << (spaces > 0 ? std::string(spaces, ' ') : "") << "| ";
                        break;
                }
            }
        }

        Str << std::endl;
    }

    Str << "Synchrone : " << (v.synchronous ? "oui" : "non") << "\nDéterministe : " << (v.deterministic ? "oui" : "non") <<
    "\nStandard : " << (v.standard ? "oui" : "non") << "\nComplet : " << (v.complete ? "oui" : "non");

    return Str;
}

bool operator<(const State& s, const State& other)  {
    return s.number < other.number;
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
    auto toShared = to.lock();

    auto it = std::find_if(transitions[symbol].begin(), transitions[symbol].end(),
                        [&toShared](const std::weak_ptr<State>& wp) {
        auto sp = wp.lock();
        return sp && sp == toShared;
    });

    if (it != transitions[symbol].end()) return;

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

    for (int i : inputs) {
        if(states[i]->out) newState.lock()->out = true;
        states[i]->in = false;

        for (int j = 0; j < states[i]->transitions.size(); ++j) {
            for (int k = 0; k < states[i]->transitions[SYMBOLS[j]].size(); ++k) {
                newState.lock()->AddTransition(SYMBOLS[j], states[i]->transitions[SYMBOLS[j]][k]);
            }
        }

        states[i]->in = false;
    }

    inputs = {newState.lock()->number};
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
    deterministic = base.Determinize();
    determinized = true;
}

StateMachine StateMachine::Determinize()  {
    StateMachine result;
    result.alphabetLength = alphabetLength;  // Assuming the alphabet length is the same for the DFA
    result.synchronous = synchronous;
    result.deterministic = true;  // The result is definitely a DFA

    std::map<std::set<int>, std::shared_ptr<State>> stateMap; // Maps sets of NFA states to DFA states
    std::queue<std::shared_ptr<State>> stateQueue; // Queue to manage states for processing

    // Initialize the start set with all NFA initial states
    std::set<int> startSet;
    bool startOut = false;
    for (int i : inputs) {
        startSet.insert(states[i]->number);
        if (states[i]->out) startOut = true;
    }

    auto startState = std::make_shared<State>(0, alphabetLength);
    startState->compositeStates = startSet;
    startState->in = true;  // Marking it as an initial state
    if (startOut) {startState->out = true; result.outputs.push_back(startState->number); }
    result.states.push_back(startState);
    result.inputs.push_back(0);  // This is the initial state of the DFA
    stateMap[startSet] = startState;
    stateQueue.push(startState);

    while (!stateQueue.empty()) {
        auto currentState = stateQueue.front();
        stateQueue.pop();

        // Process each symbol in the alphabet
        for (int i = 1; i <= alphabetLength; ++i) {
            std::set<int> newStateSet;
            bool isOut = false;

            // Determine the new set of states for this symbol
            for (int nfaState : currentState->compositeStates) {
                auto nfaStatePtr = states[nfaState];
                for (auto& transState : nfaStatePtr->transitions[SYMBOLS[i]]) {
                    if (auto spt = transState.lock()) {
                        newStateSet.insert(spt->number);
                        isOut |= spt->out; // Check if any of the NFA states had an accepting condition
                    }
                }
            }

            if (!newStateSet.empty()) {
                // Check if the new state set is already covered by an existing state
                bool isNewStateNeeded = true;
                std::shared_ptr<State> targetState = nullptr;
                for (const auto& existingState : result.states) {
                    if (std::includes(existingState->compositeStates.begin(), existingState->compositeStates.end(),
                                      newStateSet.begin(), newStateSet.end())) {
                        currentState->AddTransition(SYMBOLS[i], existingState);
                        isNewStateNeeded = false;
                        break;
                    }
                }

                if (isNewStateNeeded && stateMap.find(newStateSet) == stateMap.end()) {
                    // Create new DFA state if it does not exist
                    auto newState = std::make_shared<State>(result.states.size(), alphabetLength);
                    newState->compositeStates = newStateSet;
                    newState->out = isOut; // Set if any state in the set is an output state
                    result.states.push_back(newState);
                    stateMap[newStateSet] = newState;
                    stateQueue.push(newState);
                    if (isOut) {
                        result.outputs.push_back(newState->number);  // Add to outputs if it's an accepting state
                    }
                    targetState = newState;
                }

                // Add transition
                if (targetState != nullptr) currentState->AddTransition(SYMBOLS[i], stateMap[newStateSet]);
            }
        }
    }

    result.Evaluate();
    return result; // Return the new DFA
}

bool StateMachine::Test(std::string input) {
    int i = 0;

    int currentState = inputs[0];

    while (true) {
        char letter = input[i];
        if (states[currentState]->transitions[letter].empty()) return states[currentState]->out;

        currentState = states[currentState]->transitions[letter][0].lock()->number;
        i++;
    }
}

StateMachine StateMachine::Complimentary() {
    StateMachine result;
    result.states = states;
    result.inputs = inputs;
    for (int i = 0; i < states.size(); ++i) {
        if (!states[i]->out) {outputs.push_back(i); result.states[i]->out = true;}
        else result.states[i]->out = false;
    }
    result.alphabetLength = alphabetLength;
    result.Evaluate();

    return result;
}
