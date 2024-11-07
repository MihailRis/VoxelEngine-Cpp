# Расширения стандартных библиотек

В скрипте ядра **stdmin.lua** определены функции, расширяющие и дополняющие некоторые из стандартных библиотек **Lua**

## Расширения для table

Создаёт и возвращает копию переданной таблицы путём создания новой и копирования в неё всех элементов из переданной
```lua
function table.copy(t: table) -> table
```

Возвращает количество пар в переданной таблице
```lua
function table.count_pairs(t: table) -> integer
```

Возвращает один элемент из переданной таблицы на случайной позиции
```lua
function table.random(t: table) -> object
```

Возвращает **true**, если **x** содержится в **t**
```lua
function table.has(t: table, x: object) -> bool
```

Возвращает индекс обьекта **x** в **t**. Если переданный обьект не содержится в таблице, то функция вернёт значение **-1**
```lua
function table.index(t: table, x: object) -> integer
```

Удаляет элемент **x** из **t**
```lua
function table.remove_value(t: table, x: object)
```

Конвертирует переданную таблицу в строку
```lua
function table.tostring(t: table) -> string
```

## Расширения для string

Разбивает строку **str** на части по указанному разделителю/выражению **separator** и возвращает результат ввиде таблицы из строк. Если **withpattern** равен **true**, то параметр **separator** будет определяться как регулярное выражение
```lua
function string.explode(separator: string, str: string, withpattern: bool) -> table[string]
```

Разбивает строку **str** на части по указанному разделителю **delimiter** и возвращает результат ввиде таблицы из строк
```lua
function string.split(str: string, delimiter: string) -> table[string]
```

Экранирует специальные символы в строке, такие как `()[]+-.$%^?*` в формате `%символ`. Символ `NUL` (`\0`) будет преобразован в `%z`
```lua
function string.pattern_safe(str: string)
```

Разбивает секунды на часы, минуты и миллисекунды и форматирует в **format** с следующим порядком параметров: `минуты, секунды, миллисекунды` и после возвращает результат. Если **format** не указан, то возвращает таблицу, где: **h** - hours, **m** - minutes, **s** - seconds, **ms** - milliseconds
```lua
function string.formatted_time(seconds: number, format: string) -> string | table
```

Заменяет все подстроки в **str**, равные **tofind** на **toreplace** и возвращает строку со всеми измененными подстроками
```lua
function string.replace(str: string, tofind: string, toreplace: string) -> string
```

Удаляет все символы, равные **char** из строки **str** с левого и правого конца и возвращает результат. Если параметр **char** не определен, то будут выбраны все пустые символы.
```lua
function string.trim(str: string, char: string) -> string
```

Удаляет все символы, равные **char** из строки **str** с левого конца и возвращает результат. Если параметр **char** не определен, то будут выбраны все пустые символы.
```lua
function string.trim_left(str: string, char: string) -> string
```

Удаляет все символы, равные **char** из строки **str** с правого конца и возвращает результат. Если параметр **char** не определен, то будут выбраны все пустые символы.
```lua
function string.trim_right(str: string, char: string) -> string
```

Возвращает **true**, если строка **str** начинается на подстроку **start**
```lua
function string.starts_with(str: string, start: string) -> bool
```

Возвращает **true**, если строка **str** заканчивается на подстроку **endStr**
```lua
function string.ends_with(str: string, endStr: string) -> bool
```

Также важно подметить, что все выше перечисленные функции, расширяющие **string** можно использовать как мета-методы на экземплярах строк, т.е.: 

```lua
local str = "ABA str BAB"

if str:starts_with("ABA") and str:ends_with("BAB") then
	print(str:replace("BA", "DC"))
end
```

Также функции `string.lower` и `string.upper` переопределены на `utf8.lower` и `utf8.upper`

## Расширения для math

Ограничивает число **_in** по лимитам **low** и **high**. Т.е.: Если **_in** больше чем **high** - вернётся **high**, если **_in** меньше чем **low** - вернётся **low**. В противном случае вернётся само число
```lua
function math.clamp(_in, low, high)
```

Возвращает случайное дробное число в диапазоне от **low** до **high**
```lua
function math.rand(low, high)
```

## Дополнительные глобальные функции

В этом же скрипте также определены и другие глобальные функции которые доступны для использования. Ниже их список


Возвращает **true**, если переданная таблица является массивом, тоесть если каждый ключ это целое число больше или равное единице и если каждый ключ следует за прошлым
```lua
function is_array(x: table) -> bool
```

Разбивает путь на две части и возвращает их: входную точку и путь к файлу
```lua
function parse_path(path: string) -> string, string
```

Вызывает функцию **func** **iters** раз, передавая ей аргументы `...`, а после выводит в консоль время в микросекундах, которое прошло с момента вызова **timeit**
```lua
function timeit(iters: integer, func: func, ...)
```

Вызывает остановку корутины до тех пор, пока не пройдёт количество секунд, указанное в **timesec**. Функция может быть использована только внутри корутины
```lua
function sleep(timesec: number)
```