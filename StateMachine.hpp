#ifndef AUTOMATES_STATEMACHINE_HPP
#define AUTOMATES_STATEMACHINE_HPP

#include <vector>
#include <string>
//TODO: DETERMINISER

// non déterministes (2+ transitions): 7, 22, 28, 29, 36, 43, 44
//                   (2+ entrées): 6, 20, 36, 39, 40
// asynchrones : 31-35

// automate:
// - synchrone:
//   - déterministe: 1 état initial MAX et 1 transition par lettre par état MAX x
//   - standard: 1 état initial MAX et aucune transition vers cet état x
//   - complet: chaque état a au moins 1 transition par lettre x
// - asynchrone: transition mot vide

class Transition;

class State {
public:
    int number;
    std::unordered_map<char, std::vector<std::weak_ptr<State>>> transitions;
    bool in, out;

    explicit State(int number, int alphabetLength);

    void AddTransition(char symbol, std::weak_ptr<State> to);
};

class StateMachine {
    std::vector<int> inputs, outputs;

public:
    int alphabetLength{};
    bool synchronous{}, deterministic{}, standard{}, complete{};
    explicit StateMachine();
    explicit StateMachine(const std::string& filePath);
    explicit StateMachine(const StateMachine& other);

    std::vector<std::shared_ptr<State>> states;

    void Evaluate();

    std::weak_ptr<State> AddState();

    void Standardize();

    void Complete();

    void Synchronize();
};

class StateMachineContainer {
public:
    bool determinized = false;
    
    StateMachine base;
    StateMachine deterministic;

    StateMachineContainer() = default;
    
    void Determinize();
};

std::ostream & operator<<(std::ostream & Str, StateMachine const & v);

#endif //AUTOMATES_STATEMACHINE_HPP
