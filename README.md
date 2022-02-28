## Сборка проекта в Visual Studio Code (под Windows)

### Настройка VS Code
* Устанавливаем msys2. Я установил его в `C:\Programs\msys64latest`. При этом сам msys2 позаботился о том, чтобы путь к его директории `C:\Programs\msys64latest\mingw64\bin` оказался в path. Проверить это можно, выполнив в консоли команду path

* Устанавливаем компилятор и дебаггер. Я делаю это с помощью пакета msys2, выполнив в окне, котрое открывает msys2, следующие команды:

```
pacman -Syuu
```

* Эта команда обновляет msys2. Она может закрыть консоль - это нормально, нужно перезапустить ее и ввести ту же команду, чтобы завершить обновление. Хорошей идеей будет время от времени обновляться, чтобы всегда иметь последнюю версию компилятора.

* Затем:

```
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-gdb
```

* Теперь в Вашей системе есть компилятор и дебаггер. Проверить это просто: открываем новое окно консоли, пишем g++ --version

* Если ответом не является версия - надо поискать, что пошло не так. Проверить path, возможно, отредактировать его вручную.

* Такую же проверку хорошо бы сделать для дебаггера: gdb --version

* Пишем hello world. Это позволит нам окончательно убедиться, что компилятор работает. в любой директории созадем файл hello.cpp с текстом

```c++
 #include <iostream>
 int main() {
     std::cout << "Hello world!" << std::endl;
     return 0;
 };
```
* потом в этой папке в командной строке компилируем командой g++ hello.cpp -o hello.exe Если появился файл hello.exe, и он запускается и выводит строчку - ок, этот шаг завершен.

* А вот теперь можно поставить VSC. Обратите внимание, что редакций есть несколько, во первых для 32 и 64 битных систем, а во вторых - то, что назывется "User Installer" и "System Installer". Выбираем 64 битный System Installer на [странице загрузки](https://code.visualstudio.com/#alt-downloads)

* В VSC ставим расширение для работы с C++, оно называется C/C++ for Visual Studio Code и написано Microsoft

* В файлах внутри папки `.vscode` необходимо прописать соответствующие пути до msys64

### Установка зависимостей
* [Curl](https://packages.msys2.org/package/mingw-w64-x86_64-curl)
* Bzip:
https://packages.msys2.org/base/bzip2

https://packages.msys2.org/package/bzip2?repo=msys&variant=x86_64

https://packages.msys2.org/package/libbz2?repo=msys&variant=x86_64

https://packages.msys2.org/package/libbz2-devel?repo=msys&variant=x86_64

https://packages.msys2.org/package/mingw-w64-x86_64-bzip2


### Сборка 
Сборка проекта осуществляется по `ctrl+shift+b`. Запуск через терминал командой `.\main.exe`

## Запуск и инструкция
Сборка и запуск `.\main.exe` осуществляет полный цикл работы:

* Данные будут загружены в `vector<string>`. Для ускорения работы под каждый из векторов резервируется по 200000000 элементов. Также резервируется память под вектора, в которые будет записываться результат. 

* Загрузка файлов по ссылкам в файлы `current.csv.bz2` - текущие данные по паспортам с сайта МВД и `known.csv.bz2` - по состоянию на 6 сентября 2021 года. Функцией - downloadToFile.

* Далее в параллельном режиме проходит чтение и распаковка архивов в память с одновременной валидацией номеров паспортов функцией is_valid_passport. В 

* Далее корректные данные из первого файла записываются в `list_of_expired_passports-validated.csv`.

* После чего оба вектора сортируются в параллельном режиме.

* Далее происходит сравнение векторов через `compareFiles`, результат записывается в `firstUniq` - список паспортов, которые добавлены в список недействительных по сравнению с 6 сентября 2021 года и `secondUniq` - список паспортов, которые исключены из списка недействительных по сравнению с 6 сентября 2021 года.

* После вектора `firstUniq` и `secondUniq` записываются в соответствующие файлы .csv. 

## Загрузка с сайта

При FORCE_DEBUG = true должен был осуществляться парсинг сайта МВД и взятие нужной ссылки с него, но это почему-то не работает, хотя URL формируется корректный.
