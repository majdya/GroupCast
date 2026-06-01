# c_protocol

פרויקט צ'אט ב-C עם ניהול קבוצות, תקשורת ברשת וממשק טקסטואלי.

## מבנה התיקיות

- `src/`      — קבצי קוד מקור (c)
- `include/`  — קבצי כותרת (h)
- `tests/`    — קבצי בדיקות
- `bin/`      — קבצים בינאריים/קומפילציה

## קומפילציה

יש להשתמש ב-Makefile או בפקודות gcc, לדוגמה:

```
gcc -Iinclude -o bin/client src/client_main.c src/client_mng.c src/client_groups_mng.c src/client_net.c src/protocol.c src/gen_dlist.c
```

## הרצת בדיקות

```
gcc -Iinclude -o bin/test_protocol tests/test_protocol.c src/protocol.c src/gen_dlist.c src/client_groups_mng.c
./bin/test_protocol
```

## תלויות
- מערכת לינוקס/WSL
- gcc

## הערות
- יש לעדכן את הנתיבים ב-Makefile בהתאם למבנה החדש.
- קבצי executables נוצרים בתיקיית `bin/`.
