# Warehouse Management System API

## Описание задания

В рамках лабораторной работы необходимо разработать и реализовать **REST API** сервис для системы управления складом(WMS) с использованием C++ и фрейиворка userver.

Задачи работы:

1. Спроектировать схемы базы данных для выбранной системы ([ZOHO](https://www.zoho.com/inventory/)).
2. Создать базу данных.
3. Создать индексы.
4. Оптимизировать запросы.
5. Партиционирование (опционально).
6. Подключить API к базе данных
7. Представить результат в виде schema.sql, data.sql, queries.sql, optimization.md, README.md и средством запуска **Docker**.

[Скачать файл с исходным заданием](system_design_task_03.pdf)

Система предназначена для:
- управления пользователями и их ролями;
- управления товарами (продуктами);
- учёта складских остатков;
- регистрации поступлений товаров на склад;
- списания товаров (брак, продажи, потери).


## Схема базы данных

### Таблицы

#### `users`
Хранит информацию о пользователях системы.

| Колонка     | Тип                     | Ограничения                                      |
|-------------|-------------------------|--------------------------------------------------|
| `id`        | SERIAL                  | PRIMARY KEY                                      |
| `username`  | VARCHAR(50)             | UNIQUE, NOT NULL                                 |
| `full_name` | VARCHAR(100)            | NOT NULL                                         |
| `role`      | VARCHAR(20)             | CHECK (`role` IN ('admin','manager','worker'))   |
| `created_at`| TIMESTAMP               | DEFAULT CURRENT_TIMESTAMP                        |

#### `products`
Каталог товаров.

| Колонка      | Тип                | Ограничения                        |
|--------------|--------------------|------------------------------------|
| `id`         | SERIAL             | PRIMARY KEY                        |
| `name`       | VARCHAR(200)       | NOT NULL                           |
| `sku`        | VARCHAR(50)        | UNIQUE, NOT NULL                   |
| `price`      | DECIMAL(12,2)      | CHECK (`price >= 0`), NOT NULL     |
| `created_at` | TIMESTAMP          | DEFAULT CURRENT_TIMESTAMP          |

#### `inventory`
Остатки товаров на складе.

| Колонка         | Тип                | Ограничения                                      |
|-----------------|--------------------|--------------------------------------------------|
| `id`            | SERIAL             | PRIMARY KEY                                      |
| `product_id`    | INTEGER            | FOREIGN KEY → `products(id)` ON DELETE CASCADE  |
| `quantity`      | INTEGER            | DEFAULT 0, CHECK (`quantity >= 0`)               |
| `last_updated`  | TIMESTAMP          | DEFAULT CURRENT_TIMESTAMP                        |

#### `arrivals`
Журнал поступлений товаров.

| Колонка         | Тип                | Ограничения                            |
|-----------------|--------------------|----------------------------------------|
| `id`            | SERIAL             | PRIMARY KEY                            |
| `product_id`    | INTEGER            | FOREIGN KEY → `products(id)`           |
| `quantity`      | INTEGER            | CHECK (`quantity > 0`)                 |
| `arrival_date`  | DATE               | NOT NULL                               |
| `created_by`    | INTEGER            | FOREIGN KEY → `users(id)`              |
| `created_at`    | TIMESTAMP          | DEFAULT CURRENT_TIMESTAMP              |

#### `writeoffs`
Журнал списаний товаров.

| Колонка         | Тип                | Ограничения                            |
|-----------------|--------------------|----------------------------------------|
| `id`            | SERIAL             | PRIMARY KEY                            |
| `product_id`    | INTEGER            | FOREIGN KEY → `products(id)`           |
| `quantity`      | INTEGER            | CHECK (`quantity > 0`)                 |
| `reason`        | TEXT               |                                        |
| `created_by`    | INTEGER            | FOREIGN KEY → `users(id)`              |
| `created_at`    | TIMESTAMP          | DEFAULT CURRENT_TIMESTAMP              |

### Индексы

Для ускорения часто выполняемых запросов созданы следующие индексы:

| Индекс                            | Цель                                                                 |
|-----------------------------------|----------------------------------------------------------------------|
| `idx_inventory_product_id`        | Быстрый поиск остатков по `product_id` (JOIN с `products`)           |
| `idx_arrivals_product_id`         | Фильтрация поступлений по товару                                     |
| `idx_arrivals_date`               | Выборка поступлений за период (например, за последние 30 дней)       |
| `idx_writeoffs_product_id`        | Анализ списаний по товару                                            |
| `idx_products_name`               | Поиск товаров по имени (ускорение `LIKE` / `ILIKE` с префиксом)      |
| `idx_products_sku`                | Поиск по артикулу (уникальный, но индекс ускоряет `WHERE sku = ...`) |

Все индексы создаются в файле `schema.sql`. Первичные ключи индексируются автоматически.

## Создание базы данных и тестовые данные

Для быстрого развёртывания используется `docker-compose.yaml`, который поднимает контейнер PostgreSQL 15 и автоматически выполняет скрипты инициализации:

- `01_schema.sql` - создание таблиц и индексов.
- `02_data.sql` - вставка тестовых данных (минимум 10 записей в каждой таблице).
- `03_queries.sql` - примеры запросов с `EXPLAIN` и оптимизациями.

Тестовые данные включают:
- 10 пользователей с ролями `admin`, `manager`, `worker`.
- 10 товаров с уникальными артикулами (SKU) и ценами.
- Для каждого товара случайное начальное количество на складе (от 10 до 210).
- 10 записей о поступлениях (даты в марте 2026).
- 3 примера списаний.

## Оптимизация запросов

Все ключевые запросы системы (получение остатков, поиск товаров, отчёт по поступлениям, массовое списание) проанализированы с помощью `EXPLAIN (ANALYZE, BUFFERS)`. Для каждого запроса:

- Показан план выполнения до оптимизации.
- Добавлены индексы или переписаны запросы.
- Приведён улучшенный план с объяснением выигрыша.

Подробное описание оптимизаций и сравнение планов находится в файле [`optimization.md`](optimization.md).

Примеры запросов, включённых в анализ:
- Получение списка товаров с остатками (`LEFT JOIN inventory`).
- Поиск товаров по подстроке в названии или артикуле.
- Поступления за последние 30 дней с данными о товаре и создателе.
- Транзакционное списание (обновление `inventory` + вставка в `writeoffs`).
- Топ-5 товаров по суммарному количеству поступлений.

## Подключение API к базе данных

REST API сервис реализован на C++ с использованием фреймворка userver. Все обработчики (`handlers`) через компонент `postgres-db` выполняют SQL-запросы к спроектированной БД.

Список доступных эндпоинтов:

| Метод   | URL                         | Описание                          |
|---------|-----------------------------|-----------------------------------|
| POST    | `/auth`                     | "Демо-аутентификация"             |
| GET     | `/users/{username}`         | Получить информацию о пользователе|
| GET     | `/products`                 | Список товаров                    |
| POST    | `/products`                 | Создать новый товар               |
| PATCH   | `/products/{id}`            | Обновить товар                    |
| DELETE  | `/products/{id}`            | Удалить товар                     |
| GET     | `/inventory`                | Остатки всех товаров              |
| POST    | `/inventory/writeoff`       | Списать товар (с проверкой остатка)|
| GET     | `/arrivals`                 | История поступлений               |
| POST    | `/arrivals`                 | Добавить поступление (обновляет остаток)|

Все операции, изменяющие данные, используют транзакции (`BEGIN` / `COMMIT`), а для списания применяется блокировка строки (`SELECT ... FOR UPDATE`).

## Запуск проекта с использованием Docker

1. Убедитесь, что установлены **Docker** и **Docker Compose**.
2. Клонируйте репозиторий и перейдите в корневую директорию проекта.
3. Выполните команду:
   ```bash
   docker-compose up --build
   ```
4. После успешного запуска API доступно по адресу: http://localhost:8080.
   ```bash
   docker-compose down
   ```

## Примеры запросов к API

```bash
# Получить всех пользователей (только один по имени, т.к. GET /users/{username})
curl http://localhost:8080/users/alice

# Список товаров
curl http://localhost:8080/products

# Создать новый товар
curl -X POST http://localhost:8080/products \
  -H "Content-Type: application/json" \
  -d '{"name":"Планшет","sku":"TB-011","price":14999.99}'

# Обновить цену товара с id=1
curl -X PATCH http://localhost:8080/products/1 \
  -H "Content-Type: application/json" \
  -d '{"name":"Смартфон X","sku":"PH-001","price":18999.99}'

# Удалить товар с id=10
curl -X DELETE http://localhost:8080/products/10

# Получить остатки
curl http://localhost:8080/inventory

# Списать 2 единицы товара с id=3 (причина "Продажа")
curl -X POST http://localhost:8080/inventory/writeoff \
  -H "Content-Type: application/json" \
  -d '{"product_id":3,"quantity":2,"reason":"Продажа"}'

# История поступлений
curl http://localhost:8080/arrivals

# Добавить поступление 50 шт. товара id=2
curl -X POST http://localhost:8080/arrivals \
  -H "Content-Type: application/json" \
  -d '{"product_id":2,"quantity":50,"date":"2026-04-01"}'
```

## Структура проекта

```
postgreuserverforwms/
├── config/
│   └── config.yaml
├── sql/
│   ├── 01_schema.sql
│   ├── 02_data.sql
│   └── 03_queries.sql
├── src/
│   ├── handlers/
│   │   ├── arrivals/
│   │   │   ├── get/
│   │   │   │   ├── arrivals.cpp
│   │   │   │   └── arrivals.hpp
│   │   │   └── post/
│   │   │       ├── arrivals.cpp
│   │   │       └── arrivals.hpp
│   │   ├── auth/
│   │   │   └── post/
│   │   │       ├── auth.cpp
│   │   │       └── auth.hpp
│   │   ├── inventory/
│   │   │   ├── get/
│   │   │   │   ├── inventory.cpp
│   │   │   │   └── inventory.hpp
│   │   │   └── post/
│   │   │       ├── inventory.cpp
│   │   │       └── inventory.hpp
│   │   ├── products/
│   │   │   ├── delete/
│   │   │   │   ├── products.cpp
│   │   │   │   └── products.hpp
│   │   │   ├── get/
│   │   │   │   ├── products.cpp
│   │   │   │   └── products.hpp
│   │   │   ├── patch/
│   │   │   │   ├── products.cpp
│   │   │   │   └── products.hpp
│   │   │   └── post/
│   │   │       ├── products.cpp
│   │   │       └── products.hpp
│   │   └── users/
│   │       └── get/
│   │           ├── users.cpp
│   │           └── users.hpp
│   └── main.cpp
├── .dockerignore
├── .gitignore
├── CMakeLists.txt
├── docker-compose.yaml
├── Dockerfile
├── optimization.md
├── README.md
└── system_design_task_03.pdf
```