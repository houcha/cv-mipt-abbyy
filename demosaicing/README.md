# Report

Реализован алгоритм **Patterned Pixel Grouping (PPG)** интерполяции фильтра Байера.

Для ввода/ввывода использованна сторонняя библиотека [Bitmap](https://github.com/ArashPartow/bitmap).

## Reproduction (Linux)

Compile & run:
```sh
$ g++ interpolation.cpp -o interpolate
$ ./interpolate img/RGB_CFA.bmp img/Interpolated.bmp
```

PSNR:
```sh
$ python psnr.py img/Interpolated.bmp img/Original.bmp
```

## Image comparison

RGB_CFA (input):
![RGB_CFA](img/RGB_CFA.bmp)

Interpolated (output):
![interpolated](img/Interpolated.bmp)

Original:
![original](img/Original.bmp)

Интерполированное изображение схоже с оригиналом, однако содержит ряд недостатков:

* Серые области по краям имеют зеленоватый оттенок,
* Цветовые дефекты на границах предметов: например, бирюзовая вертикальная полоса справа содержит множественные вкрапления горизонтальных красных полос, жёлтая снизу - синие вертикальные,
* Цвет мелких деталей изменён, причём неоднородно

Несмотря на цветовые искажения, мелкие детали практически не потеряли резкости, то есть можно сделать вывод, что разрешение восстановленного изображения отличается от оригинала незначительно.

## Performance

| Metric       |      Score   |
| ------------ | ------------ |
| PSNR         |   14.0       |

| Optimization | Time, s/MP   |
| ------------ | ------------ |
| -O2          |   1.39       |
| -O3          |   0.12       |
