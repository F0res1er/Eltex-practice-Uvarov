#include "header.h"
#include <string.h>

//В комментариях оставляю пометочки для себя

int parse_rights(const char *s, mode_t *mode) 
{
    mode_t value = 0;
    size_t n = strlen(s);

    if (n == 3) 
    {
        for (int i = 0; i < 3; i++) 
        {
            if (s[i] < '0' || s[i] > '7') return 0;
            
            value = (value << 3) | (s[i] - '0');
            /*value = 000 000 000 -> побитовый сдвиг даст 0 -> 
            закинули 7 преобразовав в число -> побитовое ИЛИ докинет результат в value -> 
            закинули 5 (000 000 101) -> сдвинули value на 3 позиции (000 111 000) -> 
            -> побитовым ИЛИ докинули 5 -> value = 000 111 101 и т.д.*/
        }
    } 
    
    else if (n == 9) 
    {
        for (int i = 0; i < 9; i++) 
        {
            if (s[i] == '-') continue;
            
            if (s[i] != "rwx"[i % 3]) return 0;
            
            value |= 0400 >> i;
            /*Побитовый сдвиг вправо от 0400 (чтение для владельца) на позицию i даст соответствующую позицию 
            и право по группам: i = 1 (0200 - запись); i = 2 (0100 - исполнение); 
            i = 3 (побитовый сдвиг на 3 позиции -> чтение для группы) и т.д.
            
            Побитовое ИЛИ докидывает права в value*/
        }
    } 
    
    else 
    {
        return 0;
    }

    *mode = value;
    return 1;
}

int file_rights(const char *name, mode_t *mode)
{
    struct stat st;

    if (stat(name, &st) != 0) return 0;

    *mode = st.st_mode & 0777; //через побитовое И выделяем только 9 младших бит прав доступа из st_mode
    return 1;
}

int change_rights(mode_t *mode, const char *cmd)
{
    mode_t result = *mode, numeric;

    if (parse_rights(cmd, &numeric))
    {
        *mode = numeric;
        return 1;
    }

    do {
        mode_t who = 0, bits = 0;

        while (*cmd == ' ') cmd++;
        
        if (!*cmd) return 0;

        while (*cmd && strchr("ugoa", *cmd))
        {
            if (*cmd == 'u') who |= 0700; // 111 000 000
            if (*cmd == 'g') who |= 0070; // 000 111 000
            if (*cmd == 'o') who |= 0007; // 000 000 111
            if (*cmd == 'a') who |= 0777; // 111 111 111
            
            cmd++;
        }

        if (!who) who = 0777; // если категория не указана, то считается, что выполняется для всех (+x например)

        char op = *cmd++;
        
        if (op != '+' && op != '-' && op != '=') return 0;

        while (*cmd && strchr("rwx", *cmd))
        {
            if (*cmd == 'r') bits |= 0444; // 100 100 100
            if (*cmd == 'w') bits |= 0222; // 010 010 010
            if (*cmd == 'x') bits |= 0111; // 001 001 001
            
            cmd++;
        }

        if (!bits && op != '=') return 0; //если не указаны права и при этом оператор не '=', то бан
        
        bits &= who; // через побитовое И оставляем только те биты, которые относятся к указанным в who категориям

        if (op == '+') result |= bits; // через побитовое ИЛИ докидываем права в result
        if (op == '-') result &= ~bits; //Через побитовое И исключаем инвертированные биты права  
        if (op == '=') result = (result & ~who) | bits; //через побитовое И очищаем права выбранных категорий и побитовым ИЛИ докидываем обновлённые права

        while (*cmd == ' ') cmd++;

        if (*cmd) 
        {
            if (*cmd != ',') return 0; //если находим запятую, продолжаем кудесничать
            
            cmd++;
            
            if (!*cmd) return 0;
        }

    } while (*cmd);

    *mode = result; 
    return 1;
}