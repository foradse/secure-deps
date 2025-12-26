# secure-deps

Инструмент для обеспечения безопасности зависимостей в проектах на C/C++.

## Авторы

- [Zed](https://github.com/foradse)
- [Timmtimm123](https://github.com/Timmtimm123)

## Описание

`secure-deps` предоставляет инструменты для проверки и управления зависимостями в проектах:
- Проверка библиотек в Makefile на соответствие разрешенному списку (allowlist)
- Переписывание URL репозиториев в CMake FetchContent согласно allowlist

## Требования

- CMake 3.16 или выше
- Компилятор C++17 или выше

## Сборка

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Использование

### Проверка библиотек в Makefile

Проверяет использование библиотек в Makefile и сравнивает их с разрешенным списком:

```bash
secure_deps makefile-check --makefile <путь> [--allowlist <путь>] [--json]
```

Параметры:
- `--makefile` - путь к Makefile (по умолчанию: `Makefile`)
- `--allowlist` - путь к файлу со списком разрешенных библиотек (опционально)
- `--json` - вывести результаты в формате JSON (опционально)

### Переписывание репозиториев в CMake FetchContent

Переписывает URL репозиториев в CMakeLists.txt согласно карте разрешенных репозиториев:

```bash
secure_deps cmake-rewrite --input <путь> --allowlist <путь> [--output <путь>] [--json]
```

Параметры:
- `--input` - путь к входному CMakeLists.txt (по умолчанию: `CMakeLists.txt`)
- `--allowlist` - путь к файлу с картой репозиториев (обязательно)
- `--output` - путь к выходному файлу (по умолчанию: перезаписывает входной файл)
- `--json` - вывести результаты в формате JSON (опционально)

### Запуск SEC2 и SEC4 с сохранением JSON

Примеры запуска с записью JSON-вывода в файл:

```bash
# SEC2: проверка Makefile
secure_deps makefile-check --makefile Makefile --allowlist allowlist_libs.txt --json > sec2_output.json

# SEC4: переписывание FetchContent
secure_deps cmake-rewrite --input CMakeLists.txt --allowlist allowlist_repos.txt --json > sec4_output.json
```

Флаги в примерах:
- `--makefile` — путь к Makefile для SEC2.
- `--allowlist` — путь к allowlist (для библиотек в SEC2, для репозиториев в SEC4).
- `--json` — печатает результат в JSON (перенаправляем в файл).
- `--input` — входной `CMakeLists.txt` для SEC4.

## Формат файла allowlist

### Для библиотек (makefile-check)

Каждая строка содержит имя библиотеки (префикс `-l` опционален). Комментарии начинаются с `#`:

```
ssl
crypto
-lz
```

### Для репозиториев (cmake-rewrite)

Каждая строка содержит пару исходный_репозиторий => целевой_репозиторий. Кавычки опциональны:

```
https://github.com/orig/repo.git => https://mirror.local/repo.git
"https://example.com/lib.git" => "https://mirror.local/lib.git"
```

## Примеры JSON вывода

### Проверка Makefile:

```json
{
  "libraries": ["ssl", "crypto"],
  "disallowed": ["foo"],
  "status": "fail"
}
```

### Переписывание CMake:

```json
{
  "replaced": ["https://github.com/orig/repo.git => https://mirror.local/repo.git"],
  "missing": [],
  "status": "ok"
}
```

## Как это работает

- **Парсер Makefile**: сканирует текст и извлекает токены, соответствующие паттерну `-l...`, таким образом `-lssl` становится `ssl`.
- **Allowlist для библиотек**: если предоставлен, любая библиотека, не находящаяся в списке, помечается как неразрешенная.
- **Парсер CMake**: сканирует токены, находит `FetchContent_Declare(...)`, затем ищет значения `GIT_REPOSITORY`.
- **Карта репозиториев**: если URL репозитория находится в карте, он заменяется в выходном файле; в противном случае помечается как отсутствующий.
