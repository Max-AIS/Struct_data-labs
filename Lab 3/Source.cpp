#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <stack>
#include <list>
#include <sstream>
#include <functional>

using namespace std;
using namespace chrono;

struct Variant {
    vector<int> vizy;
    int cost;
};

struct Employee {
    int id;
    vector<int> children;
    vector<Variant> variants;
};

vector<Employee> employees;
vector<int> dp;
vector<bool> visited;
vector<int> signatureOrder; // Для хранения порядка подписей

// Через массив
class StackArray {
private:
    int* data;
    int capacity;
    int top;

public:
    StackArray(int size = 100) {  // Уменьшил размер для лучшей производительности
        capacity = size;
        data = new int[capacity];
        top = -1;
    }

    ~StackArray() {
        delete[] data;
    }

    void push(int value) {
        if (top < capacity - 1) {
            data[++top] = value;
        }
    }

    int pop() {
        if (top >= 0) {
            return data[top--];
        }
        return -1;
    }

    bool empty() {
        return top == -1;
    }

    int peek() {
        if (top >= 0) {
            return data[top];
        }
        return -1;
    }
};

// Через связанный список
struct Node {
    int data;
    Node* next;

    Node(int val) : data(val), next(nullptr) {}
};

class StackList {
private:
    Node* head;

public:
    StackList() : head(nullptr) {}

    ~StackList() {
        while (!empty()) {
            pop();
        }
    }

    void push(int value) {
        Node* newNode = new Node(value);
        newNode->next = head;
        head = newNode;
    }

    int pop() {
        if (empty()) return -1;
        Node* temp = head;
        int value = temp->data;
        head = head->next;
        delete temp;
        return value;
    }

    bool empty() {
        return head == nullptr;
    }

    int peek() {
        if (empty()) return -1;
        return head->data;
    }
};

// Через STL
class StackSTL {
private:
    stack<int> st;

public:
    void push(int value) {
        st.push(value);
    }

    int pop() {
        if (st.empty()) return -1;
        int value = st.top();
        st.pop();
        return value;
    }

    bool empty() {
        return st.empty();
    }

    int peek() {
        if (st.empty()) return -1;
        return st.top();
    }
};

// Функция для вывода порядка подписей
void printSignatureOrder(const vector<pair<int, int>>& choices) {
    cout << "\n--- ПОРЯДОК ПОЛУЧЕНИЯ ПОДПИСЕЙ (ВИЗ) ---\n";
    cout << "Оптимальная последовательность получения подписей (от подчиненных к начальникам):\n\n";

    // Собираем всех чиновников, которые должны подписать
    vector<int> signers;
    for (const auto& choice : choices) {
        signers.push_back(choice.first);
        for (int v : employees[choice.first].variants[choice.second].vizy) {
            signers.push_back(v);
        }
    }

    // Удаляем дубликаты
    sort(signers.begin(), signers.end());
    signers.erase(unique(signers.begin(), signers.end()), signers.end());

    // Топологическая сортировка (от листьев к корню) с помощью стека
    vector<int> order;
    vector<bool> visitedNode(employees.size(), false);
    vector<bool> processed(employees.size(), false);

    // Используем стек для обхода в глубину
    StackList stack;

    for (int signer : signers) {
        if (!visitedNode[signer]) {
            stack.push(signer);

            while (!stack.empty()) {
                int current = stack.peek();

                if (!visitedNode[current]) {
                    visitedNode[current] = true;

                    // Находим выбранный вариант для текущего чиновника
                    bool hasChildren = false;
                    for (const auto& choice : choices) {
                        if (choice.first == current) {
                            // Добавляем подчиненных в стек (в обратном порядке)
                            for (int i = employees[current].variants[choice.second].vizy.size() - 1; i >= 0; i--) {
                                int child = employees[current].variants[choice.second].vizy[i];
                                if (find(signers.begin(), signers.end(), child) != signers.end() && !visitedNode[child]) {
                                    stack.push(child);
                                    hasChildren = true;
                                }
                            }
                            break;
                        }
                    }

                    if (!hasChildren) {
                        // Если нет подчиненных, отмечаем как обработанный
                        processed[current] = true;
                        order.push_back(current);
                        stack.pop();
                    }
                }
                else if (!processed[current]) {
                    // Все дети обработаны, добавляем текущий узел
                    processed[current] = true;
                    order.push_back(current);
                    stack.pop();
                }
                else {
                    stack.pop();
                }
            }
        }
    }

    // Выводим порядок
    for (size_t i = 0; i < order.size(); i++) {
        cout << "  " << (i + 1) << ". Чиновник " << order[i];

        // Находим стоимость для этого чиновника
        for (const auto& choice : choices) {
            if (choice.first == order[i]) {
                cout << " (стоимость: " << employees[order[i]].variants[choice.second].cost << ")";
                if (employees[order[i]].variants[choice.second].vizy.size() > 0) {
                    cout << " [требует визы у: ";
                    for (size_t j = 0; j < employees[order[i]].variants[choice.second].vizy.size(); j++) {
                        cout << employees[order[i]].variants[choice.second].vizy[j];
                        if (j < employees[order[i]].variants[choice.second].vizy.size() - 1) cout << ", ";
                    }
                    cout << "]";
                }
                break;
            }
        }
        cout << "\n";
    }

    cout << "\nИТОГО: " << order.size() << " подписей, общая сумма: ";
    int total = 0;
    for (const auto& choice : choices) {
        total += employees[choice.first].variants[choice.second].cost;
    }
    cout << total << "\n\n";
}

// Функции для тестирования производительности
int dfsWithStackArray(int u, vector<pair<int, int>>& choices) {
    StackArray stack(100);
    vector<int> state(employees.size(), 0);

    stack.push(u);

    while (!stack.empty()) {
        int current = stack.peek();

        if (state[current] == 0) {
            state[current] = 1;

            for (int i = employees[current].children.size() - 1; i >= 0; i--) {
                stack.push(employees[current].children[i]);
            }
        }
        else if (state[current] == 1) {
            int best = 1e9;
            int bestVariant = -1;

            for (size_t idx = 0; idx < employees[current].variants.size(); idx++) {
                const auto& var = employees[current].variants[idx];
                int sumCost = var.cost;
                bool possible = true;

                for (int v : var.vizy) {
                    if (v == current || dp[v] == 1e9) {
                        possible = false;
                        break;
                    }
                    sumCost += dp[v];
                }

                if (possible && sumCost < best) {
                    best = sumCost;
                    bestVariant = idx;
                }
            }

            dp[current] = best;
            if (bestVariant != -1) {
                choices.push_back({ current, bestVariant });
            }
            state[current] = 2;
            stack.pop();
        }
    }

    return dp[u];
}

int dfsWithStackList(int u, vector<pair<int, int>>& choices) {
    StackList stack;
    vector<int> state(employees.size(), 0);

    stack.push(u);

    while (!stack.empty()) {
        int current = stack.peek();

        if (state[current] == 0) {
            state[current] = 1;

            for (int i = employees[current].children.size() - 1; i >= 0; i--) {
                stack.push(employees[current].children[i]);
            }
        }
        else if (state[current] == 1) {
            int best = 1e9;
            int bestVariant = -1;

            for (size_t idx = 0; idx < employees[current].variants.size(); idx++) {
                const auto& var = employees[current].variants[idx];
                int sumCost = var.cost;
                bool possible = true;

                for (int v : var.vizy) {
                    if (v == current || dp[v] == 1e9) {
                        possible = false;
                        break;
                    }
                    sumCost += dp[v];
                }

                if (possible && sumCost < best) {
                    best = sumCost;
                    bestVariant = idx;
                }
            }

            dp[current] = best;
            if (bestVariant != -1) {
                choices.push_back({ current, bestVariant });
            }
            state[current] = 2;
            stack.pop();
        }
    }

    return dp[u];
}

int dfsWithStackSTL(int u, vector<pair<int, int>>& choices) {
    StackSTL stack;
    vector<int> state(employees.size(), 0);

    stack.push(u);

    while (!stack.empty()) {
        int current = stack.peek();

        if (state[current] == 0) {
            state[current] = 1;

            for (int i = employees[current].children.size() - 1; i >= 0; i--) {
                stack.push(employees[current].children[i]);
            }
        }
        else if (state[current] == 1) {
            int best = 1e9;
            int bestVariant = -1;

            for (size_t idx = 0; idx < employees[current].variants.size(); idx++) {
                const auto& var = employees[current].variants[idx];
                int sumCost = var.cost;
                bool possible = true;

                for (int v : var.vizy) {
                    if (v == current || dp[v] == 1e9) {
                        possible = false;
                        break;
                    }
                    sumCost += dp[v];
                }

                if (possible && sumCost < best) {
                    best = sumCost;
                    bestVariant = idx;
                }
            }

            dp[current] = best;
            if (bestVariant != -1) {
                choices.push_back({ current, bestVariant });
            }
            state[current] = 2;
            stack.pop();
        }
    }

    return dp[u];
}

// Функция для сравнения производительности
void comparePerformance(int root) {
    cout << "\n--- СРАВНЕНИЕ ПРОИЗВОДИТЕЛЬНОСТИ ---\n\n";

    vector<pair<int, int>> choicesArray, choicesList, choicesSTL;

    // Тест для массива
    dp.assign(employees.size(), -1);
    choicesArray.clear();
    auto start = high_resolution_clock::now();
    int resultArray = dfsWithStackArray(root, choicesArray);
    auto end = high_resolution_clock::now();
    auto durationArray = duration_cast<microseconds>(end - start);

    cout << "Реализация А (массив):\n";
    cout << "  Результат: ";
    if (resultArray >= 1e9) {
        cout << "Невозможно";
    }
    else {
        cout << resultArray;
    }
    cout << "\n  Время: " << durationArray.count() << " мкс\n\n";

    // Тест для списка
    dp.assign(employees.size(), -1);
    choicesList.clear();
    start = high_resolution_clock::now();
    int resultList = dfsWithStackList(root, choicesList);
    end = high_resolution_clock::now();
    auto durationList = duration_cast<microseconds>(end - start);

    cout << "Реализация Б (связный список):\n";
    cout << "  Результат: ";
    if (resultList >= 1e9) {
        cout << "Невозможно";
    }
    else {
        cout << resultList;
    }
    cout << "\n  Время: " << durationList.count() << " мкс\n\n";

    // Тест для STL
    dp.assign(employees.size(), -1);
    choicesSTL.clear();
    start = high_resolution_clock::now();
    int resultSTL = dfsWithStackSTL(root, choicesSTL);
    end = high_resolution_clock::now();
    auto durationSTL = duration_cast<microseconds>(end - start);

    cout << "Реализация В (STL):\n";
    cout << "  Результат: ";
    if (resultSTL >= 1e9) {
        cout << "Невозможно";
    }
    else {
        cout << resultSTL;
    }
    cout << "\n  Время: " << durationSTL.count() << " мкс\n\n";

    // Вывод порядка подписей (используем результаты из STL реализации)
    if (resultSTL < 1e9) {
        printSignatureOrder(choicesSTL);
    }
}

int main() {
    cout << "Введите количество чиновников N (N < 100): ";
    int N;
    cin >> N;

    employees.resize(N + 1);
    dp.assign(N + 1, -1);
    visited.assign(N + 1, false);

    for (int i = 1; i <= N; i++) {
        employees[i].id = i;
    }

    cout << "\n--- Ввод иерархии ---\n";
    cout << "Для каждого чиновника от 2 до N введите номер его непосредственного начальника.\n";
    cout << "Чиновник 1 - корень (начальник всех), для него начальника вводить не нужно.\n\n";

    for (int i = 2; i <= N; i++) {
        cout << "Начальник чиновника " << i << ": ";
        int mgr;
        cin >> mgr;

        if (mgr < 1 || mgr >= i) {
            cout << "Предупреждение: начальник должен иметь меньший номер и существовать. Попробуйте снова.\n";
            i--;
            continue;
        }

        employees[mgr].children.push_back(i);
    }

    cout << "\n--- Ввод вариантов виз и взяток ---\n";
    cout << "Для каждого чиновника (от 1 до N) введите:\n";
    cout << "- Количество наборов виз (K, 1..15)\n";
    cout << "- Для каждого набора: количество виз, затем список чиновников, затем стоимость взятки\n\n";

    for (int i = 1; i <= N; i++) {
        cout << "\nЧиновник " << i << ":\n";
        cout << "  Количество наборов виз: ";
        int K;
        cin >> K;

        if (K < 1 || K > 15) {
            cout << "  Ошибка: K должно быть от 1 до 15. Попробуйте снова.\n";
            i--;
            continue;
        }

        employees[i].variants.resize(K);

        for (int j = 0; j < K; j++) {
            cout << "  Набор " << (j + 1) << ":\n";
            cout << "    Количество виз: ";
            int vizCount;
            cin >> vizCount;

            if (vizCount < 0 || vizCount >(int)employees[i].children.size()) {
                cout << "    Ошибка: количество виз не может превышать количество подчиненных ("
                    << employees[i].children.size() << "). Попробуйте снова.\n";
                j--;
                continue;
            }

            employees[i].variants[j].vizy.resize(vizCount);

            if (vizCount > 0) {
                cout << "    Введите номера " << vizCount << " чиновников: ";
                for (int k = 0; k < vizCount; k++) {
                    cin >> employees[i].variants[j].vizy[k];
                }
            }

            cout << "    Стоимость взятки: ";
            cin >> employees[i].variants[j].cost;

            if (employees[i].variants[j].cost < 0) {
                cout << "    Ошибка: стоимость не может быть отрицательной. Попробуйте снова.\n";
                j--;
            }
        }
    }

    // Сравнение производительности всех трех реализаций
    comparePerformance(1);

    cout << "\nАвтор: Голиков М.А.\n";
    cout << "Группа: 020303-АИСа-025\n";

    return 0;
}