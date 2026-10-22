#!/bin/bash

WORKDIR="/root/Practice_3"
CONF="$WORKDIR/observer.conf"
LOG="$WORKDIR/observer.log"

cd "$WORKDIR"

# Читаем список задач из observer.conf
while IFS= read -r script; do

    FOUND=0

    # Ищем заданный скрипт в командных строках процессов через procfs
    for proc in /proc/[0-9]*; do
        if [[ -r "$proc/cmdline" ]]; then
            if tr '\0' '\n' < "$proc/cmdline" 2>/dev/null | grep -Fxq "$script"; then
                FOUND=1
                break
            fi
        fi
    done

    # Если процесса нет — запускаем его независимо от терминала
    if [[ "$FOUND" -eq 0 ]]; then
        printf '%s Restart %s\n' \
            "$(date '+%F %T')" \
            "$script" >> "$LOG"

        nohup "$script" >/dev/null 2>&1 &
    fi

done < "$CONF"