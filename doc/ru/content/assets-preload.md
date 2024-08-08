# Предзагрузка ассетов (файл *preload.json*)

Так как не все ассеты загружаются автоматически (например звуки и дополнительные текстуры)
в движке есть предзагрузка ассетов.
Она описывается в файле `preload.json` который находиться в корне контент-пакаю

Ассеты в файле разделяются на категории:

- fonts - шрифты
- shaders - шейдеры
- textures - текстуры
- sounds - звуки
- models - модели

## Примеры

> [!NOTE] Пример
>
> ```json
> {
>     "shaders": [
>         "ui3d",
>         "screen",
>         "background",
>         "skybox_gen"
>     ],
>     "textures": [
>         "misc/moon",
>         "misc/sun",
>         "gui/crosshair"
>     ]
> }
> ```

> [!TIP] Подсказка
> Добавление звука `sounds/events/explosion.ogg` со всеми его вариантами:
>
> ```json
> {
>     "sounds": [
>         "events/explosion"
>     ]
> }
> ```
>
> Будет доступен под именем: "events/explosion"
>
> При загрузке звука подгружаются все его вариации, по 2 шаблонам:
>
> `explosion.ogg`, `explosion_1.ogg`, `explosion_2.ogg`, ...
>
> `explosion_0.ogg`, `explosion_1.ogg`, `explosion_2.ogg`, ...
>
> ---
>
> В случае, если нужно будет работать с PCM данными звука (сейчас не доступно из скриптинга), требуется указать параметр `keep-pcm`:
>
> ```json
> {
>     "sounds": [
>         {
>             "name": "events/explosion",
>             "keep-pcm": true
>         }
>     ]
> }
> ```
