# SB2022_C
Проверяла работоспособность на Mac OS, на Linux.\
Думаю, что на Windows это решение не будет корректно работать.\
Через valgrind прогнала. Unit-тестов нет.
Требования к входным данным: подавать первым аргументом название или паттерн, вторым (опционально) - директорию.\
Паттерн нужно передавать в кавычках.\
Относительный путь передавать без первого слэша, но с последним: safeboard/task/.\
Абсолютный ~/user/safeboard/ (Linux), /user/safeboard (MacOS).