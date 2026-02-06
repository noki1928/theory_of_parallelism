# Float vs Double sine array

## Описание
Программа создаёт массив из 10^7 элементов типа `float` или `double`,
заполняет его значениями синуса (один период на всю длину массива),
считает сумму элементов и выводит результат в терминал.

Тип массива выбирается во время сборки.

---

## Сборка

### Float (по умолчанию)
```bash
mkdir build
cd build
cmake -G Ninja ..
ninja
./main
```
### Double
```bash
mkdir build
cd build
cmake -G Ninja -DDOUBLE=ON ..
ninja
./main
```
