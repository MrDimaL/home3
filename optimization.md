# Оптимизация запросов

## 1. Поиск товаров

Запрос:

SELECT * FROM products WHERE name LIKE '%a%';

Без индекса выполняется полный перебор таблицы.

Добавил индекс:

CREATE INDEX idx_products_name ON products(name);

После этого поиск стал быстрее, но LIKE с % в начале всё равно не полностью использует индекс.

---

## 2. JOIN inventory и products

Запрос:

SELECT p.name, i.quantity
FROM inventory i
JOIN products p ON p.id = i.product_id;

Добавил индекс:

CREATE INDEX idx_inventory_product ON inventory(product_id);

Это уменьшило время соединения таблиц.

---

## 3. arrivals

Часто запрашивается по product_id:

CREATE INDEX idx_arrivals_product ON arrivals(product_id);

---

## Вывод

Индексы помогают ускорить JOIN и WHERE, но нужно понимать, что они занимают память и не всегда полезны для маленьких таблиц.
