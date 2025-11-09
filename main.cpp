#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <random>
#include <chrono>
#include <cctype>
#include <limits>
#include <algorithm>

using namespace std;
using namespace std::chrono;

// ---------- ДОПОМІЖНІ ФУНКЦІЇ ----------

// Шифр Цезаря: зсув вправо (шифрування)
string encryptWord(const string& word, int key) {
    string encrypted;
    for (char ch : word) {
        if (isalpha(ch)) {
            char base = isupper(ch) ? 'A' : 'a';
            encrypted += (base + (ch - base + key) % 26);
        } else {
            encrypted += ch;
        }
    }
    return encrypted;
}

// Розшифрування (зсув вліво)
string decryptWord(const string& word, int key) {
    string decrypted;
    for (char ch : word) {
        if (isalpha(ch)) {
            char base = isupper(ch) ? 'A' : 'a';
            decrypted += (base + (ch - base - key + 26) % 26);
        } else {
            decrypted += ch;
        }
    }
    return decrypted;
}

// Видалення пробілів і \r, \n з країв
string trim(const string& str) {
    const string whitespace = " \t\n\r";
    size_t start = str.find_first_not_of(whitespace);
    if (start == string::npos) return "";
    size_t end = str.find_last_not_of(whitespace);
    return str.substr(start, end - start + 1);
}

// ---------- КЛАС ГРИ ----------
class System {
private:
    vector<string> words;
    vector<char> guessedLetters;
    vector<char> mustLetters;
    vector<char> letters;
    int num = 7;
    string word;

public:
    void clearInputBuffer() {
        if (cin.fail()) cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    void generateLetters(const string& w) {
        letters.clear();
        mustLetters.clear();
        guessedLetters.clear();
        for (size_t i = 0; i < w.length(); i++) {
            if (i % 2 == 1 || i == 0)
                letters.push_back(w[i]);
            else
                mustLetters.push_back(w[i]);
        }
    }

    bool isItInMustLetters(const char& j) {
        return find(mustLetters.begin(), mustLetters.end(), j) != mustLetters.end();
    }

    void read(const string& PATH) {
        ifstream inFile(PATH);
        if (!inFile.is_open()) {
            cerr << "❌ Не вдалося відкрити файл " << PATH << endl;
            return;
        }

        string line;
        while (getline(inFile, line)) {
            string trimmed_line = trim(line);
            if (!trimmed_line.empty()) {
                words.push_back(decryptWord(trimmed_line, 4)); // Ключ 4
            }
        }

        if (words.empty()) {
            cerr << "❌ Помилка: файл порожній або невірний формат." << endl;
        } else {
            cout << "✅ Завантажено " << words.size() << " слів.\n";
        }
    }

    void game() {
        if (words.empty()) {
            cout << "Немає слів для гри.\n";
            return;
        }

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dist(0, words.size() - 1);
        int n = dist(gen);
        word = words[n];

        generateLetters(word);
        cout << "\n🎮 Почнемо гру! Вгадайте слово!\n";

        size_t mustLettersCount = mustLetters.size();
        while (num > 0 && guessedLetters.size() < mustLettersCount) {
            cout << "\nСлово: ";
            for (size_t i = 0; i < word.length(); ++i) {
                bool isGuessed = find(guessedLetters.begin(), guessedLetters.end(), word[i]) != guessedLetters.end();
                if ((i % 2 == 1 || i == 0) || isGuessed)
                    cout << word[i];
                else
                    cout << "_";
            }
            cout << "\nВгадано: " << guessedLetters.size() << "/" << mustLettersCount;
            cout << " | Спроб залишилось: " << num << "\nВведіть літеру: ";

            char tmp;
            cin >> tmp;
            clearInputBuffer();
            tmp = tolower(tmp);

            if (find(guessedLetters.begin(), guessedLetters.end(), tmp) != guessedLetters.end()) {
                cout << "⚠️ Ви вже називали '" << tmp << "'.\n";
                continue;
            }

            if (isItInMustLetters(tmp)) {
                cout << "✅ Вгадали!\n";
                guessedLetters.push_back(tmp);
            } else {
                cout << "❌ Такої літери немає.\n";
                num--;
            }
        }

        if (guessedLetters.size() == mustLettersCount)
            cout << "\n🎉 ВИ ПЕРЕМОГЛИ! Слово: " << word << "\n";
        else
            cout << "\n💀 Ви програли. Слово було: " << word << "\n";
    }

    void runGame(const string& path) {
        read(path);
        if (words.empty()) return;
        num = 7;

        auto start_time = high_resolution_clock::now();
        game();
        auto end_time = high_resolution_clock::now();

        auto seconds = duration_cast<std::chrono::seconds>(end_time - start_time).count();
        cout << "\n⏱️ Час гри: " << seconds << " секунд\n";
    }
};

// ---------- MAIN ----------
int main() {
    // Створюємо тестовий файл зі зашифрованими словами
    vector<string> words = {"apple", "banana", "cherry", "orange", "grape"};
    ofstream out("encrypted_words.txt");
    for (auto& w : words) {
        out << encryptWord(w, 4) << "\n";
    }
    out.close();
    cout << "📁 Створено файл encrypted_words.txt з 5 зашифрованими словами.\n";

    // Запускаємо гру
    System game;
    game.runGame("encrypted_words.txt");
    return 0;
}
