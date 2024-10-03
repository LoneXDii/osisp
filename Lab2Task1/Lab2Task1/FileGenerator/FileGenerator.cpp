#include <iostream>
#include <fstream>
#include <string>

int main() {
    setlocale(LC_ALL, "Russian");
    srand(time(0));
    // Имя файла
    std::string filename = "data.txt";

    // Открываем файл для записи
    std::ofstream outfile(filename);

    // Проверяем, удалось ли открыть файл
    if (!outfile) {
        std::cerr << "Ошибка при открытии файла для записи!" << std::endl;
        return 1;
    }

    for (int i = 0; i < 1e9; i++) {
        outfile << (char)(rand() % 92 + 30);
    }

    // Закрываем файл
    outfile.close();

    std::cout << "Данные успешно записаны в файл " << filename << std::endl;

    return 0;
}