#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#include <iostream>
#include <string>
#include <vector>
#include <ctime> // Для роботи з датою та часом (структура tm, time_t, mktime)
#include <algorithm> // Для функцій find та sort
#include <iomanip> // Для форматування виводу (setprecision, fixed)
#include <fstream> // Для роботи з файлами (ofstream)
#include <sstream> // Для роботи з потоками в пам'яті (stringstream)
#include <limits>// Для numeric_limits, потрібний для очищення буфера
#include <windows.h> 
#include <clocale>

using namespace std;

// Функція для очищення буфера вводу
// Це критично важливо після використання cin >> X (де X - число, слово),
// оскільки залишковий '\n' (ентер) може бути помилково зчитаний наступним getline().
void clearInputBuffer() {
    // Якщо попередній ввід cin завершився з помилкою (наприклад, замість числа ввели літери)
    if (cin.fail()) {
        cin.clear(); // Скидаємо прапорці помилок cin
    }
    // Ігноруємо всі символи до кінця рядка ('\n'), щоб очистити буфер
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Допоміжна функція для перевірки, чи є елемент 'it' у векторі рядків 'tmp'.
// Приймаємо вектор як const reference для ефективності (не копіюємо великий вектор).
bool findEl(string it, const vector<string>& tmp) {
    // Використовуємо стандартний алгоритм find. Якщо ітератор не дорівнює end(), значить, елемент знайдено.
    return find(tmp.begin(), tmp.end(), it) != tmp.end();
}

// --- Структури даних ---

// Структура для зберігання інформації про гаманець/карту
struct Wallet {
    string cardNum; // Номер карти (зберігаємо як рядок)
    int cvv; // Захисний код (CVV)
    tm date; // Дата кінця терміну дії карти. Використовуємо tm (структура часу C)
};

// Структура для зберігання інформації про витрату
struct Expenses {
    double sym; // Сума витрати
    string type; // Тип/категорія витрати (наприклад, "Їжа", "Транспорт")
    tm date; // Дата здійснення витрати. Знову ж, tm
};

// Структура для агрегації витрат за типом (для рейтингу категорій)
struct Type {
    string type; // Назва категорії
    double sym = 0.0; // Сумарна витрачена сума у цій категорії

    // Перевантаження оператора == для порівняння об'єктів Type з рядком.
    // Це дозволяє легко використовувати std::find для пошуку категорії у векторі.
    bool operator==(const std::string& other_type) const {
        return type == other_type;
    }
};

// --- Основний клас системи ---
class System {
private:
    string client; // Ім'я користувача системи
    vector<Wallet> uw; // Вектор усіх гаманців (user wallets)
    vector<Expenses> ue; // Вектор усіх витрат (user expenses)
public:

    // Метод додавання нового користувача
    void addUser() {
        cout << "Введіть ім'я користувача: " << endl;
        // Використовуємо getline, оскільки ім'я може містити пробіли
        getline(cin, client);
    }

    // Метод додавання нового гаманця/карти
    void addWal() {
        string cn;
        int cvv;
        int month, year;

        cout << "Введіть номер карти: " << endl;
        getline(cin, cn);

        cout << "Введіть захисний код карти(3 цифри на звороті): " << endl;
        cin >> cvv;
        clearInputBuffer(); // Очищаємо після cin >> int

        cout << "Введіть рік закінчення терміну дії карти: " << endl;
        cin >> year;
        clearInputBuffer(); // Очищаємо після cin >> int

        cout << "Введіть місяць закінчення терміну дії карти: " << endl;
        cin >> month;
        clearInputBuffer(); // Очищаємо після cin >> int

        Wallet newWal;
        newWal.date = {}; // Ініціалізуємо tm нулями
        newWal.cardNum = cn;
        newWal.cvv = cvv;
        // tm_year - це роки з 1900. tm_mon - це місяці з 0 (0=січень).
        newWal.date.tm_year = year - 1900;
        newWal.date.tm_mon = month - 1;

        uw.push_back(newWal); // Додаємо до вектора
    }

    // Метод додавання нової витрати
    void addExpenses() {
        double sym; // Сума
        string type; // Тип
        int month, year, day;

        cout << "Введіть суму операції: " << endl;
        cin >> sym;
        clearInputBuffer(); // Очищаємо після cin >> double

        cout << "Введіть тип операції: " << endl;
        cin >> type; // Читаємо одне слово (тип)
        clearInputBuffer(); // Очищаємо після cin >> string

        cout << "Введіть рік здійснення операції: " << endl;
        cin >> year;
        clearInputBuffer();

        cout << "Введіть місяць здійснення операції: " << endl;
        cin >> month;
        clearInputBuffer();

        cout << "Введіть число здійснення операції: " << endl;
        cin >> day;
        clearInputBuffer();

        Expenses newEx;
        newEx.date = {}; // Ініціалізуємо tm нулями
        newEx.sym = sym;
        newEx.type = type;
        // Встановлюємо дату
        newEx.date.tm_year = year - 1900;
        newEx.date.tm_mon = month - 1;
        newEx.date.tm_mday = day;
        // mktime нормалізує всі поля tm (наприклад, день тижня, день року), що важливо для коректних порівнянь time_t
        mktime(&newEx.date);

        ue.push_back(newEx);
    }

    // Метод виводу статистики за датою/періодом
    void showStats() {
        int choise;
        cout << "Введіть цифру, відповідно до якої належить тип статистика: за конкретний день(1) за конкретний період(2): " << endl;
        cin >> choise;
        clearInputBuffer(); // Критично важливо очистити буфер після cin >> int!

        switch (choise) {
        case 1: { // Статистика за конкретний день
            vector<string> type;
            double allSym = 0.0;
            int month, year, day;

            cout << "Введіть рік: " << endl; cin >> year; clearInputBuffer();
            int target_year = year - 1900;

            cout << "Введіть місяць: " << endl; cin >> month; clearInputBuffer();
            int target_month = month - 1;

            cout << "Введіть число: " << endl; cin >> day; clearInputBuffer();

            // Проходимо по всіх витратах і порівнюємо tm поля
            for (const Expenses& tmp : ue) {
                if (tmp.date.tm_mon == target_month
                    && tmp.date.tm_mday == day
                    && tmp.date.tm_year == target_year) {
                    allSym += tmp.sym;
                    // Збираємо унікальні типи витрат
                    if (!findEl(tmp.type, type)) {
                        type.push_back(tmp.type);
                    }
                }
            }

            cout << "Витрати за цей день: " << fixed << setprecision(2) << allSym << endl;
            cout << "Всі типи витрати: " << endl;
            for (const string& s : type) {
                cout << s << endl;
            }
            break;
        }

        case 2: { // Статистика за конкретний період

            vector<string> type;
            double allSym = 0.0;
            int mon1, year1, day1, mon2, year2, day2;

            // Ввід початкової дати
            cout << "Введіть рік першої дати: " << endl; cin >> year1; clearInputBuffer();
            cout << "Введіть місяць першої дати: " << endl; cin >> mon1; clearInputBuffer();
            cout << "Введіть число першої дати: " << endl; cin >> day1; clearInputBuffer();

            // Ввід кінцевої дати
            cout << "Введіть рік другої дати: " << endl; cin >> year2; clearInputBuffer();
            cout << "Введіть місяць другої дати: " << endl; cin >> mon2; clearInputBuffer();
            cout << "Введіть число другої дати: " << endl; cin >> day2; clearInputBuffer();

            // Формуємо time_t (кількість секунд з 1970) для коректного порівняння діапазонів

            // 1. Початкова дата
            tm start_tm = { 0 };
            start_tm.tm_year = year1 - 1900;
            start_tm.tm_mon = mon1 - 1;
            start_tm.tm_mday = day1;
            time_t start_time = mktime(&start_tm);

            // 2. Кінцева дата (включаємо весь день до 23:59:59)
            tm end_tm = { 0 };
            end_tm.tm_year = year2 - 1900;
            end_tm.tm_mon = mon2 - 1;
            end_tm.tm_mday = day2;
            end_tm.tm_hour = 23; end_tm.tm_min = 59; end_tm.tm_sec = 59;
            time_t end_time = mktime(&end_tm);

            // 3. Порівняння
            for (const Expenses& tmp : ue) {
                tm expense_tm = tmp.date; // Створюємо копію, бо mktime змінює tm
                time_t expense_time = mktime(&expense_tm); // Отримуємо time_t для витрати

                // Перевіряємо, чи дата витрати входить у діапазон (включно)
                if (expense_time != (time_t)-1 && expense_time >= start_time && expense_time <= end_time) {
                    allSym += tmp.sym;
                    if (!findEl(tmp.type, type)) {
                        type.push_back(tmp.type);
                    }
                }
            }

            cout << "Витрати за цей період: " << fixed << setprecision(2) << allSym << endl;
            cout << "Всі типи витрати: " << endl;
            for (const string& s : type) {
                cout << s << endl;
            }

            break;
        }
        default:
            cout << "Невірний вибір. Введіть 1 або 2." << endl;
            break;
        }
    }

    // Метод виводу ТОП-3 витрат/категорій за період
    void showRating() {
        int choise;

        cout << "Введіть цифру, відповідно до якої належить тип статистика: ТОП-3 витрати(1) ТОП-3 категорії(2): " << endl;
        cin >> choise;
        clearInputBuffer();

        // Використовуємо stringstream, щоб зібрати вивід для консолі та файлу
        stringstream output_buffer;

        // Введення періоду для рейтингу
        int mon1, year1, day1, mon2, year2, day2;
        cout << "Введіть рік першої дати: " << endl; cin >> year1; clearInputBuffer();
        cout << "Введіть місяць першої дати: " << endl; cin >> mon1; clearInputBuffer();
        cout << "Введіть число першої дати: " << endl; cin >> day1; clearInputBuffer();
        cout << "Введіть рік другої дати: " << endl; cin >> year2; clearInputBuffer();
        cout << "Введіть місяць другої дати: " << endl; cin >> mon2; clearInputBuffer();
        cout << "Введіть число другої дати: " << endl; cin >> day2; clearInputBuffer();

        // Формування меж time_t (аналогічно showStats case 2)
        tm start_tm = { 0 };
        start_tm.tm_year = year1 - 1900; start_tm.tm_mon = mon1 - 1; start_tm.tm_mday = day1;
        time_t start_time = mktime(&start_tm);

        tm end_tm = { 0 };
        end_tm.tm_year = year2 - 1900; end_tm.tm_mon = mon2 - 1; end_tm.tm_mday = day2;
        end_tm.tm_hour = 23; end_tm.tm_min = 59; end_tm.tm_sec = 59;
        time_t end_time = mktime(&end_tm);

        switch (choise) {
        case 1: { // ТОП-3 за Сумами Витрат (кожна витрата окремо)

            vector<Expenses> filtered_expenses;

            // 1. Фільтрація витрат за періодом
            for (const Expenses& tmp : ue) {
                tm expense_tm = tmp.date;
                time_t expense_time = mktime(&expense_tm);
                if (expense_time != (time_t)-1 && expense_time >= start_time && expense_time <= end_time) {
                    filtered_expenses.push_back(tmp);
                }
            }

            // 2. Сортування: Лямбда-функція сортує від більшої суми до меншої
            sort(filtered_expenses.begin(), filtered_expenses.end(), [](const Expenses& a, const Expenses& b) {
                return a.sym > b.sym;
                });

            // 3. Збір виводу
            output_buffer << "Рейтинг: ТОП-3 Витрати за період (" << year1 << "/" << mon1 << "/" << day1 << " - " << year2 << "/" << mon2 << "/" << day2 << ")." << endl;
            output_buffer << "--- ТОП-3 Витрати за Сумою ---" << endl;

            // Виводимо до 3-х елементів (або менше, якщо менше доступно)
            for (size_t i = 0; i < min((size_t)3, filtered_expenses.size()); ++i) {
                // Форматуємо дату для виводу
                char date_str[11];
                strftime(date_str, sizeof(date_str), "%Y/%m/%d", &filtered_expenses[i].date);

                output_buffer << i + 1 << ". " << filtered_expenses[i].type << " ("
                    << fixed << setprecision(2) << filtered_expenses[i].sym
                    << ") від " << date_str << endl;
            }
            if (filtered_expenses.empty()) output_buffer << "Немає даних за цей період." << endl;

            // Виведення на екран і збереження у файл
            cout << output_buffer.str();
            saveStatsToFile("top_stats_report.txt", output_buffer.str());

            break;
        }

        case 2: { // ТОП-3 Категорії (сумарні витрати за типом)

            vector<Type> types; // Список для агрегації (категорія + сума)

            // 1. Агрегація витрат за періодом
            for (const Expenses& tmp : ue) {
                tm expense_tm = tmp.date;
                time_t expense_time = mktime(&expense_tm);
                if (expense_time != (time_t)-1 && expense_time >= start_time && expense_time <= end_time) {

                    // Шукаємо, чи категорія вже є у списку types
                    auto it = find(types.begin(), types.end(), tmp.type);

                    if (it == types.end()) {
                        // Якщо не знайдено, додаємо нову категорію
                        Type tmp1;
                        tmp1.type = tmp.type;
                        tmp1.sym += tmp.sym;
                        types.push_back(tmp1);
                    }
                    else {
                        // Якщо знайдено, додаємо суму до існуючої категорії
                        it->sym += tmp.sym;
                    }
                }
            }

            // 2. Сортування: Сортуємо категорії від більшої суми до меншої
            sort(types.begin(), types.end(), [](const Type& a, const Type& b) {
                return a.sym > b.sym;
                });

            // 3. Збір виводу
            output_buffer << "Рейтинг: ТОП-3 Категорії за період (" << year1 << "/" << mon1 << "/" << day1 << " - " << year2 << "/" << mon2 << "/" << day2 << ")." << endl;
            output_buffer << "--- ТОП-3 Категорії за Сумою ---" << endl;

            for (size_t i = 0; i < std::min((size_t)3, types.size()); ++i) {
                output_buffer << i + 1 << ". " << types[i].type << ": "
                    << fixed << setprecision(2) << types[i].sym << endl;
            }
            if (types.empty()) output_buffer << "Немає даних за цей період." << endl;

            // Виведення на екран і збереження у файл
            cout << output_buffer.str();
            saveStatsToFile("top_stats_report.txt", output_buffer.str());

            break;
        }
        default:
            cout << "Невірний вибір. Введіть 1 або 2." << endl;
            break;
        }
    }

    // Метод для збереження звітів у текстовий файл
    void saveStatsToFile(const std::string& filename, const std::string& content) {
        // Відкриваємо файл у режимі додавання (ios_base::app)
        ofstream outfile(filename, ios_base::app);
        if (outfile.is_open()) {
            // Отримуємо поточний час для мітки у звіті
            time_t now = time(0);
            // std::ctime повертає покажчик на внутрішній статичний буфер
            char* dt_ptr = ctime(&now); // ✅ Використовуємо стандартну функцію
            if (dt_ptr) { // Перевірка на успішність
                // Якщо dt_ptr не nullptr, використовуємо його
                outfile << "\n--- СТАТИСТИКА ТОП-3 (" << dt_ptr << ") ---" << endl;
            }
            outfile << content; // Записуємо вміст зі stringstream
            outfile.close();
            cout << "\n✅ Дані ТОП-3 успішно збережено у файл: " << filename << endl;
        }
        else {
            cerr << "\n❌ Помилка: Не вдалося відкрити файл " << filename << endl;
        }
    }
};

// --- Основна функція для запуску та тестування ---
int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // 2. Встановлення локалі C (для сумісності)
    // У Windows може знадобитися встановити нейтральну UTF-8 локаль для коректної роботи std::cout
    setlocale(LC_ALL, "C.UTF-8");

    
    cout.sync_with_stdio(false);
    System app;

    cout << "============================================" << endl;
    cout << "          ТЕСТУВАННЯ СИСТЕМИ              " << endl;
    cout << "============================================" << endl;

    // 1. Тест addUser
    cout << "\n--- 1. Додавання Користувача ---" << endl;
    cout << "Введіть ім'я (Тестовий Користувач):" << endl;
    app.addUser();

    // 2. Тест addWal
    cout << "\n--- 2. Додавання Гаманця ---" << endl;
    cout << "Будь ласка, введіть дані для першого гаманця (приклад: 123456789, 321, 2025, 12):" << endl;
    app.addWal();

    // 3. Тест addExpenses (Введення тестових даних)
    cout << "\n--- 3. Додавання Витрат (ТЕСТОВІ ДАНІ) ---" << endl;

    // Ввід 1: 350.00 Їжа 2025 11 5 (Для перевірки case 1)
    cout << "Введіть дані для витрати 1 (100.00 Їжа 2025 11 5):" << endl; app.addExpenses();
    // Ввід 2: 50.50 Транспорт 2025 11 4 (Для перевірки case 2)
    cout << "Введіть дані для витрати 2 (50.50 Транспорт 2025 11 4):" << endl; app.addExpenses();
    // Ввід 3: 250.00 Їжа 2025 11 5 (Для агрегації Їжа: 100 + 250 = 350)
    cout << "Введіть дані для витрати 3 (250.00 Їжа 2025 11 5):" << endl; app.addExpenses();
    // Ввід 4: 500.00 Розваги 2025 10 25 (Для перевірки періоду)
    cout << "Введіть дані для витрати 4 (500.00 Розваги 2025 10 25):" << endl; app.addExpenses();
    // Ввід 5: 120.00 Транспорт 2025 11 6 (Для агрегації Транспорт: 50.50 + 120 = 170.50)
    cout << "Введіть дані для витрати 5 (120.00 Транспорт 2025 11 6):" << endl; app.addExpenses();

    // 4. Тест showStats
    cout << "\n============================================" << endl;
    cout << "         4. ТЕСТУВАННЯ showStats            " << endl;
    cout << "============================================" << endl;

    // Тест 1: За день 2025/11/05. Очікується 350.00 (100 + 250)
    cout << "Введіть 1, а потім 2025 11 5:" << endl;
    app.showStats();

    // Тест 2: За період 2025/10/20 - 2025/11/06. Очікується 100+50.5+250+500+120 = 1020.50
    cout << "Введіть 2, а потім 2025 10 20 та 2025 11 6:" << endl;
    app.showStats();

    // 5. Тест showRating
    cout << "\n============================================" << endl;
    cout << "         5. ТЕСТУВАННЯ showRating           " << endl;
    cout << "============================================" << endl;

    // Тест 3: ТОП-3 Суми (період 2025/11/01 - 2025/11/30).
    // Очікується: 250.00, 120.00, 100.00 (Розваги 500.00 виключені, бо жовтень)
    cout << "Введіть 1, а потім 2025 11 1 та 2025 11 30:" << endl;
    app.showRating();

    // Тест 4: ТОП-3 Категорії (період 2025/11/01 - 2025/11/30).
    // Очікується: Їжа (350.00), Транспорт (170.50)
    cout << "Введіть 2, а потім 2025 11 1 та 2025 11 30:" << endl;
    app.showRating();

    cout << "\nПрограма завершила тестування." << endl;

    return 0; // При завершенні main() автоматично викликається деструктор app (неявний),
    // який коректно звільняє вектори.
}