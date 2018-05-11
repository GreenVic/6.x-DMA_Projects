﻿# DMA Projects

Здесь будут представлены демо-проекты работы с DMA

Заводить новый репозиторий под каждый простейший проект показалось не слишком разумным.

## DMA_ToDAC
Пример выводит сигнал в ЦАП посредством DMA по отсчетам таймера.

Помимо вывода сигнала, проект показывает, что для работы с DMA необходимо включать тактирование всех блоков SSP. Если этого не сделать, то прерывания DMA начинают генерироваться постоянно, еще до инициализации таймера и разрешения ему работать на DMA. При этом основная ветвь перестает исполняться, что демонстрируют два светодиода.

Оба светодиода зажигаются при старте инициализации и выключаются при окончании инициализации. Далее в основном цикле мигает только один светодиод, показывая что исполнение проходит штатно. При этом на выходе ЦАП осциллографом можно увидеть выводимый сигнал синуса. (Сигнал можно поменять если выбрать другую функцию рассчета сигнала в коде)

Баг активируется макроопределением BUG_SSPx_ACTIVATE. При этом загораются оба светодиода и останутся гореть, т.к. цикл инициализации прерывается непрерывными прерываниями от SSP.

Баг связан с тем, что при включении МК со стороны SSP к NVIC стоит активный сигнал запроса прерывания. При разрешении тактирования SSP этот сигнал запроса приходит в состояние, согласованное с регистром SSP в котором запрос на прерывание по умолчанию выключен. Поэтому включение тактирования SSP помогает избавиться от лишних прерываний от DMA.

## 6.3 - DMA_TimerCAP
В примере канал 1 таймера 3 генерирует сигнал ШИМ, который перемычкой на демо-плате подается на вход канала 4 таймера 1. Канал DMA передает данные из регистра захвата в массив, из которого затем высчитывается средний период и выводится на экран LCD.

Кнопки UP/DOWN на плате позволяют регулировать период ШИМ, с тем, чтобы определить минимальных период сигнала, который может быть захвачен.

Кнопки LEFT/RIGHT регулируют отступ в массиве захваченных данных, из которых вычисляется период. Это необходимо, потому что при периоде ШИМ порядка 25 тиков частоты CPU в первых отсчетах массива иногда приходят неправильные данные. Возможно это связано с пересинхронизацией между DMA и таймером, при запуске нового цикла DMA.

Минимальный захваченный период ШИМ получился в 11 тактов.
