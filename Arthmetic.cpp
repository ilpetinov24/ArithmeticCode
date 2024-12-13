#include <bitset>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

#define BREAK_SYMBOL  '\0'
#define EOF_SYMBOL    '-'

// Константы для нормализации
#define HIGH          65536
#define HALF          32768
#define QUARTER       16384
#define THIRD_QUARTER 49152

using namespace std;


void PrintTable(vector<char> al, vector<uint32_t> freq) {
    for (int i = 0; i < al.size(); i++) {
        cout << al[i] << " - " << freq[i] << endl;
    }
    cout << endl;
}


/* Функция для формирования алфавита. 
   Функция возвращает массив символов, которые встречались в тексте. */
vector<char> GetAlphabet(const string& sourceText) {
    vector<char> alphabet; // Алфавит

    alphabet.push_back(EOF_SYMBOL);

    // Формирования алфавита
    for (auto c: sourceText) {
        // Флаг для проверки символа в алфавите
        bool flag = false;
        for (int i = 0; i < alphabet.size(); i++) {
            /* Если символ уже был есть в алфавите, то останавливаем цикл
               и берем следующий символ */
            if (alphabet[i] == c) {
                flag = true;
                break;
            }
        }
        
        // Если символа нету в алфавите, то добавляем его
        if (!flag)
            alphabet.push_back(c);
    }

    return alphabet;
}


/* Функция для подсчета частот символов.
   Возвращает массив частот.
   Массивы frequency и alphabet взаимно-однозначно связаны между собой. */
vector<uint32_t> GetFrequency(const string& sourceText) {
    vector<char> alphabet = GetAlphabet(sourceText);    // Формирую алфавит
    vector<uint32_t> frequency(alphabet.size()); // Таблица частот

    for (int i = 0; i < alphabet.size(); i++) {
        for (auto c: sourceText)
            if (alphabet[i] == c)
                frequency[i]++;

        if (i > 0) frequency[i] += frequency[i - 1];
    }

    return frequency;
}


/* Функция для нахождения символа в алфавите.
   Возвращает индекс под которым находится элемент в алфавите. */
int GetSymbolInAlpha(const string& text, const int& i, const vector<char>& alphabet) {
    // Вывожу ошибку, если индекс некорректен
    if (i >= text.size()) {
        cout << "Index out of range!\n";
        exit(1);
    }

    // Поиск элемента в алфавите
    char symbol = text[i];

    for (int i = 0; i < alphabet.size(); i++) {
        if (symbol == alphabet[i])
            return i;
    }

    return -1;
}


/* Ф-ия для переноса найденных битов в output. */
void BitsToAdd(string& output, bool bit, int bitsToAdd) {
    output += to_string(bit);
    
    while (bitsToAdd > 0) {
        output += to_string(!bit);
        bitsToAdd--;
    }
}


/* Функция арифметического кодирования.
   На вход принимает исходный текст, алфавит и частоты символов.
   Возвращает закодированную строку. */
string ArithmeticEncoding(const string& sourceText, const vector<char>& alphabet, const vector<uint32_t>& freq) {
    string encode = "";                // Закодированная строка
    int textLen = sourceText.length(); // Длина исходного текста
    
    // Интервалы
    uint32_t high  = HIGH - 1;
    uint32_t low   = 0;
    uint32_t range = 0;

    uint32_t del = freq[alphabet.size() - 1];
    uint32_t bitsToAdd = 0;

    int current = 1;
    int i = 0;

    while (i < textLen) {
        current = GetSymbolInAlpha(sourceText, i, alphabet);
        i++;
        
        range = high - low + 1;
        high = low + (range * freq[current]) / del - 1;
        low = low + (range * freq[current - 1]) / del;

        // cout << low << "; " << high << endl;
        
        // Нормализация интервала
        for(;;) {
            if (high < HALF) {
                BitsToAdd(encode, 0, bitsToAdd);
                bitsToAdd = 0;
            }
            else if (low >= HALF) {
                BitsToAdd(encode, 1, bitsToAdd);
                bitsToAdd = 0;
                low -= HALF;
                high -= HALF;
            } else if (low >= QUARTER && high < THIRD_QUARTER) {
                bitsToAdd++;
                low -= QUARTER;
                high -= QUARTER;
            } else break;

            low  = 2 * low;
            high = 2 * high + 1;
        }
    }

    return encode;
}


uint32_t Read16Bit(const string& encode, int& bitsCount) {
    uint32_t value = 0;
    uint16_t mask = 1;
    int bitsInEncode = encode.length();

    for (int i = 15; i >= 0; i--) {
        // Если в закодированном файле меньше 16 битов
        if (i < bitsInEncode) {
            if (encode[i] == '1')
                value |= (mask << bitsCount);
            bitsCount++;
            mask = 1;
        }
    }
    
    return value;
}


int AddBit(const string& encode, int value, int currentBit, bool& flag) {
    bitset<16> a(value);

    if (flag == 1) {
        a.reset(0);
    } else if (currentBit >= encode.length()) {
        a.set(0);
        flag = 1;
    } else if (encode[currentBit] == '1') {
        a.set(0);
    } else if (encode[currentBit] == '0') {
        a.reset(0);
    }
    value = (uint32_t)(a.to_ulong());
    return value;

}


/* Функция арифметического декодирования */
string ArithmeticDecoding(const string& encode, const vector<char>& alphabet, const vector<uint32_t> frequency) {
    string decode = "";
    int bitsInEncode = encode.length();

    uint32_t low = 0;
    uint32_t high = HIGH - 1;
    uint32_t value = 0;
    uint32_t range = 0;
    
    uint32_t frequence = 0;
    uint32_t del = frequency[alphabet.size() - 1];
    
    int currentBit = 0;
    value = Read16Bit(encode, currentBit);
    int notReadBits = 16 - currentBit;

    for (int i = 0; i < notReadBits; i++) {
        value *= 2;
    }

    bool flag = false;
    
    for (int i = 1; i < del; i++) {
        range = high - low + 1;
        frequence = (((value - low) + 1) * del - 1) / range;
        int symbolIndex = 0;
        for (symbolIndex = 1; frequency[symbolIndex] <= frequence; symbolIndex++);

        high = low + (range * frequency[symbolIndex]) / del - 1;
        low = low + (range * frequency[symbolIndex - 1]) / del;

        decode += alphabet[symbolIndex];

        cout << endl;
        cout << decode << endl;
        // cout << value << endl;
        cout << alphabet[symbolIndex] << endl;
        // cout << currentBit << endl;

        // Проверка на последний символ
        if (alphabet[symbolIndex] == BREAK_SYMBOL)
            return decode;

        for (;;) {
            if (high < HALF) {
                // Ничего не делаем
            } else if (low >= HALF) {
                low -= HALF;
                high -= HALF;
                value -= HALF;
            } else if (low >= QUARTER && high < THIRD_QUARTER) {
                low -= QUARTER;
                high -= QUARTER;
                value -= QUARTER;
            } else { 
                break;
            }

            low = low * 2;
            high = high * 2 + 1;
            value = AddBit(encode, 2 * value, currentBit, flag);
            currentBit++;
        }
    }

    return decode;
}


int main() {
    string text = "a";
    text.push_back(BREAK_SYMBOL); 

    vector<char> al = GetAlphabet(text);
    vector<uint32_t> freq = GetFrequency(text);
    
    PrintTable(al, freq);
    
    string encode = ArithmeticEncoding(text, al, freq);

    cout << encode << endl;

    string decode = ArithmeticDecoding(encode, al, freq);

    cout << decode << endl;
    
    return 0;
}