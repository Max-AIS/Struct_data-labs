#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

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

int dfs(int u) {
    if (visited[u]) return dp[u];
    visited[u] = true;

    int best = 1e9;

    for (const auto& var : employees[u].variants) {
        int sumCost = var.cost;
        bool possible = true;

        for (int v : var.vizy) {
            if (v == u) {
                possible = false;
                break;
            }
            int childCost = dfs(v);
            if (childCost == 1e9) {
                possible = false;
                break;
            }
            sumCost += childCost;
        }

        if (possible) {
            best = min(best, sumCost);
        }
    }

    return dp[u] = best;
}

int main() {
    setlocale(LC_ALL, "Russian");
    cout << "--- Программа для минимизации взяток для получения лицензии ---\n\n";

    cout << "Введите количество чиновников N (N < 100): ";
    int N;
    cin >> N;

    employees.resize(N + 1);
    dp.assign(N + 1, -1);
    visited.assign(N + 1, false);

    // Инициализация ID
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
    cout << "- Для каждого набора: количество виз, затем список чиновников (непосредственные подчиненные), затем стоимость взятки\n";
    cout << "Если набор пустой (чиновник не требует виз), введите 0 и затем стоимость\n\n";

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
            cout << "    Количество виз (непосредственных подчиненных): ";
            int vizCount;
            cin >> vizCount;

            if (vizCount < 0 || vizCount > employees[i].children.size()) {
                cout << "    Ошибка: количество виз не может превышать количество непосредственных подчиненных ("
                    << employees[i].children.size() << "). Попробуйте снова.\n";
                j--;
                continue;
            }

            employees[i].variants[j].vizy.resize(vizCount);

            if (vizCount > 0) {
                cout << "    Введите номера " << vizCount << " чиновников (непосредственных подчиненных): ";
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

    cout << "\n--- Результат ---\n";
    int result = dfs(1);

    if (result >= 1e9) {
        cout << "Невозможно получить лицензию (нет допустимых наборов виз для корневого чиновника или его подчиненных).\n";
    }
    else {
        cout << "Минимальная сумма взяток: " << result << " долларов\n";
    }

    return 0;
}