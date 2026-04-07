# Warehouse Management System API

## Описание задания

В рамках лабораторной работы необходимо разработать и реализовать **REST API** сервис для системы управления складом(WMS) с использованием C++ и фрейиворка userver.

Задачи работы:

1. Спроектировать REST API для выбранного варианта системы ([ZOHO](https://www.zoho.com/inventory/)).
2. Реализовать REST API сервис.
3. Реализовать аутентификацию.
4. Выполнить документирование API.
5. Провести тестирование.
6. Представить результат в виде **REST API сервиса**, документации, OpenAPI спецификацией и средством запуска **Docker**.

[Скачать файл с исходным заданием](system_design_task_02.pdf)

Система предназначена для:
- управления пользователями
- управления товарами
- учёта складских остатков
- регистрации поступлений товаров
- списания товаров

Используемые технологии:
- C++
- userver framework
- CMake
- Docker / Docker Compose

## Архитектура API

API реализовано в соответствии с REST-принципами:
- ресурсы представлены через URL
- используются HTTP методы (GET, POST, PATCH, DELETE)
- ответы возвращаются в JSON

---

## Запуск проекта

```bash
docker-compose up --build
```

После запуска сервис доступен по адресу: http://localhost:8000

---

## API Endpoints

### Аутентификация
В системе реализована JWT-аутентификация.

POST /auth

### Пользователи

GET /users/{username} - получение пользователя

Пример:
```bash
curl http://localhost:8080/users/admin
```

### Товары

GET /products - список товаров
POST /products - создание товара
PATCH /products/{id} - обновление товара
DELETE /products/{id} - удаление товара
GET /products/search?q=... - поиск товаров

### Остатки

GET /inventory - получение остатков
POST /inventory/writeoff - списание товара

### Поступления

GET /arrivals - история поступлений
POST /arrivals - создание поступления

---

## Тестирование API

1. Аутентификация
    ```bash
    curl -X POST http://localhost:8080/auth
    ```
2. Работа с товарами
  - Получить список:
    ```bash
    curl http://localhost:8080/products
    ```
  - Создать товар:
    ```bash
    curl -X POST http://localhost:8080/products
    ```
  - Обновить товар:
    ```bash
    curl -X PATCH http://localhost:8080/products/1
    ```
  - Удалить товар:
    ```bash
    curl -X DELETE http://localhost:8080/products/1
    ```
  - Поиск:
    ```bash
    curl "http://localhost:8080/products/search?q=test"
    ```
3. Остатки
  - Получить:
    ```bash
    curl http://localhost:8080/inventory
    ```
  - Списание:
    ```bash
    curl -X POST http://localhost:8080/inventory/writeoff
    ```
4. Пользователи
    ```bash
    curl http://localhost:8080/users/admin
    ```
5. Поступления
  - Получить:
    ```bash
    curl http://localhost:8080/arrivals
    ```

  - Создать:
    ```bash
    curl -X POST http://localhost:8080/arrivals
    ```

## Структура проекта
```
restapiuserverforwms/ 
| - config/ 
| |- config.yaml 
| - src/ 
| |- main.cpp 
| - handlers/ 
| |- auth.hpp 
| |- products.hpp 
| |- inventory.hpp 
| |- users.hpp 
| |- arrivals.hpp 
|- Dockerfile 
|- docker-compose.yaml 
|- CMakeLists.txt 
|- README.md
```

---

## Docker
Для запуска:
```bash
docker-compose up --build
```
Остановка:
```bash
docker-compose down
```

---

## Вывод

В рамках работы был реализован REST API сервис системы управления складом.

Реализовано:
- REST API сервис на C++
- обработка HTTP-запросов через userver
- набор CRUD-эндпоинтов
- контейнеризация через Docker
- базовое тестирование через curl
