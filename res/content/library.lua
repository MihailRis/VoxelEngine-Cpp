---Загружает скрипт, если ещё не загружен
---Перезагружает его, если reload == true
---@param path string
---@param reload boolean
function load_script(path, reload) print(path, reload) end

---Возвращает строковый id блока, принимая в качестве агрумента числовой
---@param id integer
---@return string
function block_name(id)
    print(id)
    return ""
end

---Возвращает числовый id блока, принимая в качестве агрумента строковый
---@param id string
---@return integer
function block_index(id)
    print(id)
    return 0
end

---Возвращает числовой id блока на указанных координатах
---@param x integer
---@param y integer
---@param z integer
---@return integer
function get_block(x, y, z)
    print(x, y, z)
    return 0
end

---Возвращает состояние (поворот + доп. информация) в виде целого числа
---@param x integer
---@param y integer
---@param z integer
---@return integer
function get_block_states(x, y, z)
    print(x, y, z)
    return 0
end

---Устанавливает блок с заданным числовым id и состоянием (0 - по-умолчанию) на заданных координатах. НЕ вызывает событие on_placed
---@param x integer
---@param y integer
---@param z integer
---@param id integer
---@param states integer
function set_block(x, y, z, id, states) print(x, y, z, id, states) end

---Проверяет, является ли блок на указанных координатах полным
---@param x integer
---@param y integer
---@param z integer
---@return boolean
function is_solid_at(x, y, z)
    print(x, y, z)
    return false
end

---Проверяет, можно ли на заданных координатах поставить блок (примеры: воздух, трава, цветы, вода)
---@param x integer
---@param y integer
---@param z integer
---@return boolean
function is_replaceable_at(x, y, z)
    print(x, y, z)
    return false
end

---Возвращает количество id доступных в движке блоков
---@return integer
function blocks_count() return 0 end

---Возвращает целочисленный единичный вектор X блока на указанных координатах с учётом его вращения (три целых числа). Если поворот отсутствует, возвращает 1, 0, 0
---@param x integer
---@param y integer
---@param z integer
---@return integer, integer, integer
function get_block_X(x, y, z)
    print(x, y, z)
    return 1, 0, 0
end

---Возвращает целочисленный единичный вектор Y блока на указанных координатах с учётом его вращения (три целых числа). Если поворот отсутствует, возвращает 0, 1, 0
---@param x integer
---@param y integer
---@param z integer
---@return integer, integer, integer
function get_block_Y(x, y, z)
    print(x, y, z)
    return 0, 1, 0
end

---Возвращает целочисленный единичный вектор Z блока на указанных координатах с учётом его вращения (три целых числа). Если поворот отсутствует, возвращает 0, 0, 1
---@param x integer
---@param y integer
---@param z integer
---@return integer, integer, integer
function get_block_Z(x, y, z)
    print(x, y, z)
    return 0, 0, 1
end

---Возвращает поворот блока на указанных координатах
---@param x integer
---@param y integer
---@param z integer
---@return integer
function get_block_rotation(x, y, z)
    print(x, y, z)
    return 0
end

---Устанавливает поворот блока на указанных координатах
---@param x integer
---@param y integer
---@param z integer
---@param rotation integer
function set_block_rotation(x, y, z, rotation) print(x, y, z, rotation) end

---Возвращает выбранное число бит с указанного смещения в виде целого беззнакового числа
---@param x integer
---@param y integer
---@param z integer
---@param offset integer
---@param bits integer
---@return integer
function get_block_user_bits(x, y, z, offset, bits)
    print(x, y, z, offset, bits)
    return 0
end

---Записывает указанное число бит значения value в user bits по выбранному смещению
---@param x integer
---@param y integer
---@param z integer
---@param offset integer
---@param bits integer
---@param value integer
function set_block_user_bits(x, y, z, offset, bits, value) print(x, y, z, offset, bits, value) end

player = {}

---Возвращает x, y, z координаты игрока
---@param id integer
---@return integer, integer, integer
function player.get_pos(id)
    print(id)
    return 0, 0, 0
end

---Устанавливает x, y, z координаты игрока
---@param id integer
---@param x integer
---@param y integer
---@param z integer
function player.set_pos(id, x, y, z) print(id, x, y, z) end

---Возвращает x, y вращения камеры (в радианах), где значение X не ограничено
---@param id integer
---@return number, number
function player.get_rot(id)
    print(id)
    return 0, 0
end

---Устанавливает x, y вращения камеры (в радианах)
---@param id integer
---@param x number
---@param y number
function player.set_rot(id, x, y) print(id, x, y) end

---Возвращает id инвентаря игрока и индекс выбранного слота (от 0 до 9)
---@param playerid integer
---@return integer, integer
function player.get_inventory(playerid)
    print(playerid)
    return 0, 0
end

world = {}

---Возвращает текущее игровое время от 0.0 до 1.0, где 0.0 и 1.0 - полночь, 0.5 - полдень.
---@return number
function world.get_day_time() return 0 end

---Устанавливает указанное игровое время.
---@param time number
function world.set_day_time(time) print(time) end

---Возвращает общее суммарное время, прошедшее в мире
---@return number
function world.get_total_time() return 0 end

---Возвращает зерно мира.
---@return integer
function world.get_seed() return 0 end

inventory = {}

---Принимает id инвентаря и индекс слота. Возвращает id предмета и его количество. id = 0 (core:empty) обозначает, что слот пуст.
---@param invid integer
---@param slot integer
---@return integer, integer
function inventory.get(invid, slot)
    print(invid, slot)
    return 0, 0
end

---Устанавливает содержимое слота.
---@param invid integer
---@param slot integer
---@param itemid integer
---@param count integer
function inventory.set(invid, slot, itemid, count) print(invid, slot, itemid, count) end

---Возращает размер инвентаря (число слотов). Если указанного инвентаря не существует, бросает исключение.
---@param invid integer
---@return integer
function inventory.size(invid)
    print(invid)
    return 0
end

---Добавляет предмет в инвентарь. Если не удалось вместить все количество, возвращает остаток.
---@param invid integer
---@param itemid integer
---@param count integer
---@return integer
function inventory.add(invid, itemid, count)
    print(invid, itemid, count)
    return 0
end

---Функция возвращает id инвентаря указанного блока. Если блок не может иметь инвентарь - возвращает 0.
---@param x integer
---@param y integer
---@param z integer
---@return integer
function inventory.get_block(x, y, z)
    print(x, y, z)
    return 0
end

---Привязывает указанный инвентарь к блоку.
---@param invid integer
---@param x integer
---@param y integer
---@param z integer
function inventory.bind_block(invid, x, y, z) print(invid, x, y, z) end

---Отвязывает инвентарь от блока.
---@param x integer
---@param y integer
---@param z integer
function inventory.unbind_block(x, y, z) print(x, y, z) end

---Создает копию инвентаря и возвращает id копии. Если копируемого инвентаря не существует, возвращает 0.
---@param invid integer
---@return integer
function inventory.clone(invid)
    print(invid)
    return 0
end

item = {}

---Возвращает строковый id предмета по его числовому id (как block_name)
---@param itemid integer
---@return string
function item.name(itemid)
    print(itemid)
    return ""
end

---Возвращает числовой id предмета по строковому id (как block_index)
---@param name string
---@return integer
function item.index(name)
    print(name)
    return 0
end

---Возвращает максимальный размер стопки для предмета.
---@param itemid integer
---@return integer
function item.stack_size(itemid)
    print(itemid)
    return 0
end

---Возвращает общее число доступных предметов (включая сгенерированные)
---@return integer
function item.defs_count() return 0 end

hud = {}

---Открывает инвентарь
function hud.open_inventory() end

---Закрывает инвентарь
function hud.close_inventory() end

---Возвращает id инвентаря блока (при inventory-size=0 создаётся виртуальный инвентарь, который удаляется после закрытия), и id макета UI.
---@param x integer
---@param y integer
---@param z integer
---@return integer, string
function hud.open_block(x, y, z)
    print(x, y, z)
    return 0, ""
end

---Добавляет постоянный элемент на экран. Элемент не удаляется при закрытии инвентаря. Чтобы не перекрывать затенение в режиме инвентаря нужно установить z-index элемента меньшим чем -1. В случае тега inventory, произойдет привязка слотов к инвентарю игрока.
---@param layoutid string
function hud.open_permanent(layoutid) print(layoutid) end

---Удаляет элемент с экрана
---@param layoutid string
function hud.close(layoutid) print(layoutid) end

local file

---Приводит запись точка_входа:путь (например user:worlds/house1) к обычному пути. (например C://Users/user/.voxeng/worlds/house1)
---Функцию не нужно использовать в сочетании с другими функциями из библиотеки, так как они делают это автоматически
---Возвращаемый путь не является каноническим и может быть как абсолютным, так и относительным.
---@param path string
---@return string
function file.resolve(path)
    print(path)
    return ""
end

---Читает весь текстовый файл и возвращает в виде строки
---@param path string
---@return string
function file.read(path)
    print(path)
    return ""
end

---Записывает текст в файл (с перезаписью)
---@param path string
---@param text string
function file.write(path, text) print(path, text) end

---Возвращает размер файла в байтах, либо -1, если файл не найден
---@param path string
---@return integer
function file.length(path)
    print(path)
    return 0
end

---Проверяет, существует ли по данному пути файл или директория
---@param path string
---@return boolean
function file.exists(path)
    print(path)
    return false
end

---Проверяет, существует ли по данному пути файл
---@param path string
---@return boolean
function file.isfile(path)
    print(path)
    return false
end

---Проверяет, существует ли по данному пути директория
---@param path string
---@return boolean
function file.isdir(path)
    print(path)
    return false
end

---Создаёт директорию. Возвращает true если была создана новая директория
---@param path string
---@return boolean
function file.mkdir(path)
    print(path)
    return false
end

time = {}

---Возвращает время с момента запуска движка в секундах
---@return number
function time.uptime() return 0 end

---Вызывается после установки блока игроком
---@param x integer
---@param y integer
---@param z integer
---@param playerid integer
function on_placed(x, y, z, playerid) print(x, y, z, playerid) end

--
---Вызывается после разрушения блока игроком
---@param x integer
---@param y integer
---@param z integer
---@param playerid integer
function on_broken(x, y, z, playerid) print(x, y, z, playerid) end

---Вызывается при нажатии на блок ПКМ. Разрешает установку блоков, если возвращает true
---@param x integer
---@param y integer
---@param z integer
---@param playerid integer
function on_interact(x, y, z, playerid) print(x, y, z, playerid) end

---Вызывается при обновлении блока (если изменился соседний блок)
---@param x integer
---@param y integer
---@param z integer
function on_update(x, y, z) print(x, y, z) end

---Вызывается в случайные моменты времени (рост травы на блоках земли)
---@param x integer
---@param y integer
---@param z integer
function on_random_update(x, y, z) print(x, y, z) end

---Вызывается tps (20) раз в секунду
---@param tps integer
function on_blocks_tick(tps) print(tps) end

---Вызывается при нажатии ПКМ на блок. Предотвращает установку блока, прописанного в placing-block если возвращает true
---@param x integer
---@param y integer
---@param z integer
---@param playerid integer
function on_use_on_block(x, y, z, playerid) print(x, y, z, playerid) end

---Вызывается при нажатии ЛКМ на блок (в т.ч неразрушимый). Предотвращает разрушение блока, если возвращает true
---@param x integer
---@param y integer
---@param z integer
---@param playerid integer
function on_block_break_by(x, y, z, playerid) print(x, y, z, playerid) end

---Вызывается при загрузке мира
function on_world_open() print("World opened") end

---Вызывается перед сохранением мира
function on_world_save() print("World saved") end

---Вызывается при выходе из мира (после сохранения)
function on_world_quit() print("World quit") end

---Вызывается при добавлении элемента на экран. При отсутствии привязки к инвентарю invid будет равен 0. При отсутствии привязки к блоку x, y, z так же будут равны 0.
---@param invid integer
---@param x integer
---@param y integer
---@param z integer
function on_open(invid, x, y, z) print(invid, x, y, z) end

---Вызывается при удалении элемента с экрана.
---@param invid integer
function on_close(invid) print(invid) end

---Вызывается после входа в мир, когда становится доступна библиотека hud. Здесь на экран добавляются постоянные элементы.
---@param playerid integer
function on_hud_open(playerid) print(playerid) end

---Вызывается при выходе из мира, перед его сохранением.
---@param playerid integer
function on_hud_close(playerid) print(playerid) end
