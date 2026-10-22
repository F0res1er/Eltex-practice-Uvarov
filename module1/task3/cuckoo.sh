#!/bin/bash

# Абсолютный путь к именованному каналу (будет получен из --pishitesuda=)
PIPE=""

# Разбираем параметры командной строки.
for arg in "$@"; do
    case "$arg" in
        --pishitesuda=*)
            # Удаляем часть "--pishitesuda=" и оставляем только путь.
            PIPE="${arg#*=}"
            ;;
    esac
done

# Проверяем, что путь указан и является абсолютным
if [[ -z "$PIPE" || "$PIPE" != /* ]]; then
    echo "Usage: $0 --pishitesuda=/absolute/path/to/pipe" >&2
    exit 1
fi

# Файл cuckoo.log в том же каталоге где находится именованный канал
LOG="$(dirname "$PIPE")/cuckoo.log"

# Обработчик завершения скрипта (вызывается при получении SIGTERM)
shutdown() {
    # Записываем дату, время и сообщение о завершении.
    printf '%s Shutdown!\n' "$(date '+%F %T')" >> "$LOG"

    # Удаляем именованный канал перед завершением работы.
    rm -f "$PIPE"

    exit 0
}

# Перехватываем сигнал SIGTERM и вызываем функцию shutdown
trap shutdown SIGTERM

# Если старый канал остался после предыдущего запуска, удаляем его
rm -f "$PIPE"

# Создаём именованный канал FIFO
mkfifo "$PIPE"

# Записываем в журнал сообщение о запуске
printf '%s Startup!\n' "$(date '+%F %T')" >> "$LOG"

# Основной цикл работы сервера.
while true; do

    # Ожидаем сообщение из именованного канала.
    if IFS= read -r request < "$PIPE"; then

        # Проверяем формат сообщения:
        # NAME[PID]: how much time do I have?
        if [[ "$request" =~ ^(.+)\[([0-9]+)\]:\ how\ much\ time\ do\ I\ have\?$ ]]; then

            # Получаем имя обратившегося процесса.
            NAME="${BASH_REMATCH[1]}"

            # Получаем PID обратившегося процесса.
            PID="${BASH_REMATCH[2]}"

            # Генерируем случайное число от 2 до 10 включительно.
            N=$((RANDOM % 9 + 2))

            # Записываем обращение и выданное число в cuckoo.log.
            printf '%s %s[%s] %s\n' \
                "$(date '+%F %T')" \
                "$NAME" \
                "$PID" \
                "$N" >> "$LOG"

            # Возвращаем клиентскому процессу число N через FIFO.
            printf '%s\n' "$N" > "$PIPE"
        fi
    fi
done