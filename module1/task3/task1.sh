#!/bin/bash

PIPE="/root/Practice_3/cuckoo.pipe"

# Получаем имя запущенного скрипта из procfs
SCRIPT_PATH=$(tr '\0' '\n' < "/proc/$$/cmdline" | sed -n '2p')
NAME=$(basename "$SCRIPT_PATH")

# Получаем PID текущего процесса из procfs
PID=$(awk '{print $1}' "/proc/$$/stat")

# Сам шаблон работать не должен
if [[ "$NAME" == "template_task.sh" ]]; then
    echo "Я бригадир, сам не работаю"
    exit 0
fi

# Файл отчёта рабочей задачи
LOG="$PWD/report_${NAME}.log"

# Записываем начало работы
printf '%s [%s] Скрипт запущен\n' \
    "$(date '+%F %T')" \
    "$PID" >> "$LOG"

# Отправляем cuckoo.sh запрос требуемого формата
printf '%s[%s]: how much time do I have?\n' \
    "$NAME" \
    "$PID" > "$PIPE"

# Получаем от cuckoo.sh число секунд
IFS= read -r N < "$PIPE"

# Ждём указанное время
sleep "$N"

# Записываем результат работы
printf '%s [%s] Скрипт завершился, работал %s секунд.\n' \
    "$(date '+%F %T')" \
    "$PID" \
    "$N" >> "$LOG"
