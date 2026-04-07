INSERT INTO users (username, full_name, role) VALUES
('alice', 'Алиса Иванова', 'admin'),
('bob', 'Боб Смирнов', 'manager'),
('charlie', 'Чарли Козлов', 'worker'),
('diana', 'Диана Петрова', 'worker'),
('eve', 'Ева Сидорова', 'manager'),
('frank', 'Франк Немцов', 'worker'),
('grace', 'Грейс Ли', 'admin'),
('henry', 'Генри Браун', 'worker'),
('ivy', 'Айви Грин', 'manager'),
('jack', 'Джек Уайт', 'worker');

INSERT INTO products (name, sku, price) VALUES
('Смартфон X', 'PH-001', 19999.99),
('Ноутбук Pro', 'NB-002', 59999.99),
('Мышь беспроводная', 'MS-003', 1299.50),
('Клавиатура мех.', 'KB-004', 3499.00),
('Монитор 24"', 'MN-005', 12499.00),
('Наушники Bluetooth', 'HP-006', 2499.00),
('Зарядное устройство', 'CH-007', 899.00),
('Чехол для телефона', 'CS-008', 499.00),
('Внешний SSD 1TB', 'SS-009', 8999.00),
('Веб-камера 4K', 'WC-010', 5499.00);

INSERT INTO inventory (product_id, quantity)
SELECT id, floor(random() * 200 + 10)::int FROM products;

INSERT INTO arrivals (product_id, quantity, arrival_date, created_by) VALUES
(1, 100, '2026-03-01', 1),
(2, 50,  '2026-03-05', 2),
(3, 200, '2026-03-10', 3),
(4, 75,  '2026-03-12', 1),
(5, 30,  '2026-03-15', 2),
(6, 120, '2026-03-18', 4),
(7, 500, '2026-03-20', 5),
(8, 300, '2026-03-22', 3),
(9, 40,  '2026-03-25', 1),
(10, 60, '2026-03-28', 2);

INSERT INTO writeoffs (product_id, quantity, reason, created_by) VALUES
(1, 5,  'Брак при приемке', 3),
(3, 12, 'Потеря при хранении', 4),
(5, 2,  'Повреждение упаковки', 2);