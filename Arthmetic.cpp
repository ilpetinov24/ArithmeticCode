#include <bitset>
#include <cstddef>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>

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

        // cout << endl;
        // cout << decode << endl;
        // cout << value << endl;
        // cout << alphabet[symbolIndex] << endl;
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


// Функция для записи закодированных данных в файл
void WriteToFile(const string& encode, ofstream& out, const vector<char>& alphabet, const vector<uint32_t>& freq) {
    unsigned char byte = 0;
    unsigned char mask = 1;
    size_t counter = 0;

    ofstream wtmpFile("tmp.txt");
    
    counter = 0;
    for (int i = 0; i < encode.size(); i++) {
        byte <<= mask;
        
        if (encode[i] == '1')
            byte |= mask;

        counter++;

        if (counter == 8) {
            wtmpFile.put(byte);
            byte = 0;
            counter = 0;
        }
    }

    // Обработка не записанных битов
    if (counter > 0) {
        byte <<= (8 - counter); // Дополнение до байта
        wtmpFile.put(byte);
    }

    wtmpFile.close();
    ifstream rtmpFile("tmp.txt");

    out << counter << "|";

    for (int i = 0; i < alphabet.size() - 1; i++) {
        if (alphabet[i] == '\n') {
            out << "--" << ':' << freq[i] << '|';
            continue;
        }
        out << alphabet[i] << ":" << freq[i] << '|';
    }
    out << alphabet[alphabet.size() - 1] << ":" << freq[alphabet.size() - 1];

    out << "\n";

    char cur;
    while (rtmpFile.get(cur))
        out << cur;

    rtmpFile.close();
    remove("tmp.txt");
}


string ReadInFile(ifstream& file, vector<char>& alphabet, vector<uint32_t>& freq) {
    string encode = "";
    unsigned char byte;
    unsigned char mask = 1;
    size_t counter;
    string table = "";
    vector<string> tableInVector;

    getline(file, table);
    string s = "";
    string tmp = "";
    tmp += table[0];
    counter = stoi(tmp);
    for (int i = 2; i < table.size(); i++) {
        if (table[i] == '|') {
            tableInVector.push_back(s);
            s = "";
            continue;
        }
        if ((i < table.size() - 1) && (table[i] == '-' && table[i + 1] == '-')) {
            s += '\n';
            i += 2;
        }
        s += table[i];
    }
    
    tableInVector.push_back(s);

    for (auto p: tableInVector) {
        alphabet.push_back(p[0]);
        freq.push_back(stoi(p.substr(2)));
    }


    while (file.get((char &)byte)) {
        for (int i = 7; i >= 0; i--) {
            unsigned char tmp = (byte >> i);
            if (tmp & mask)
                encode += '1';
            else
                encode += '0';
        }
    }
    encode = encode.substr(0, encode.size() - (8 - counter));
    return encode;
}


void Coding(ifstream& input, ofstream& out) {
    if (!input.is_open() || !out.is_open()) {
        cout << "Files is not opened!!!\n";
        exit(1);
    }

    string sourceText = "";
    string encode = "";
    vector<char> alphabet;
    vector<uint32_t> freq;

    char current;
    while (input.get(current))
        sourceText += current;
    
    sourceText.push_back(BREAK_SYMBOL);

    alphabet = GetAlphabet(sourceText);
    freq = GetFrequency(sourceText);

    cout << "Table:\n";
    PrintTable(alphabet, freq);

    encode = ArithmeticEncoding(sourceText, alphabet, freq);

    cout << "Encode: " << encode << endl;

    WriteToFile(encode, out, alphabet, freq);
}


void Decoding(ifstream& input, ofstream& out) {
    if (!input.is_open() || !out.is_open()) {
        cout << "Files is not opened!!!\n";
        exit(1);
    }
    string encode = "";
    string decode = "";
    vector<char> alphabet;
    vector<uint32_t> freq;

    encode = ReadInFile(input, alphabet, freq);
    cout << "Table: \n";
    PrintTable(alphabet, freq);
    decode = ArithmeticDecoding(encode, alphabet, freq);

    cout << "Table: \n";
    PrintTable(alphabet, freq);

    cout << "Decode: " << decode << endl;
    out << decode;
}


int main() {
    int choice = 0;
    cout << "Arithmetic coding. Choose:" << endl;
    cout << "1: Coder" << endl;
    cout << "2: Decoder" << endl;
    
    cin >> choice;

    if (choice == 1) {
        ifstream in("text.txt");
        ofstream out("encode.txt");

        Coding(in, out);
    } else if (choice == 2) {
        ifstream input("encode.txt");
        ofstream out("decode.txt");

        Decoding(input, out);
    } else 
        cout << "Choose from list!!!" << endl;

    return 0;
}