﻿# 6.3 - DMA_TimerCAP
В примере канал 1 таймера 3 генерирует сигнал ШИМ, который перемычкой на демо-плате подается на вход канала 4 таймера 1. Канал DMA передает данные из регистра захвата в массив, из которого затем высчитывается средний период и выводится на экран LCD.

Кнопки UP/DOWN на плате позволяют регулировать период ШИМ, с тем, чтобы определить минимальных период сигнала, который может быть захвачен.

Кнопки LEFT/RIGHT регулируют отступ в массиве захваченных данных, из которых вычисляется период. Это необходимо, потому что при периоде ШИМ порядка 25 тиков частоты CPU в первых отсчетах массива иногда приходят неправильные данные. Возможно это связано с пересинхронизацией между DMA и таймером, при запуске нового цикла DMA.

Минимальный захваченный период ШИМ получился в 11 тактов.

Описание проекта статье - https://startmilandr.ru/doku.php/prog:dma:dma_timercap