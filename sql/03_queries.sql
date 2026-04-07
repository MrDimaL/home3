EXPLAIN (ANALYZE, BUFFERS)
SELECT p.id, p.name, p.sku, p.price, COALESCE(i.quantity, 0) AS stock
FROM products p
LEFT JOIN inventory i ON p.id = i.product_id
ORDER BY p.id;


CREATE INDEX CONCURRENTLY IF NOT EXISTS idx_products_name ON products(name);
CREATE INDEX CONCURRENTLY IF NOT EXISTS idx_products_sku ON products(sku);

EXPLAIN (ANALYZE, BUFFERS)
SELECT * FROM products
WHERE name ILIKE '%ноут%' OR sku ILIKE '%NB%';


EXPLAIN (ANALYZE, BUFFERS)
SELECT a.id, p.name, a.quantity, a.arrival_date, u.full_name
FROM arrivals a
JOIN products p ON a.product_id = p.id
LEFT JOIN users u ON a.created_by = u.id
WHERE a.arrival_date > CURRENT_DATE - INTERVAL '30 days'
ORDER BY a.arrival_date DESC;


BEGIN;
UPDATE inventory SET quantity = quantity - 5, last_updated = NOW()
WHERE product_id = 1 AND quantity >= 5;
INSERT INTO writeoffs (product_id, quantity, reason, created_by)
VALUES (1, 5, 'Продажа', 1);
COMMIT;


EXPLAIN (ANALYZE)
SELECT p.id, p.name, SUM(a.quantity) AS total_arrived
FROM arrivals a
JOIN products p ON a.product_id = p.id
GROUP BY p.id
ORDER BY total_arrived DESC
LIMIT 5;