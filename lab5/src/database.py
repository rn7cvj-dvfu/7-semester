import psycopg2
from psycopg2 import pool
from psycopg2.extras import RealDictCursor
from contextlib import contextmanager
from config import settings
import logging

logger = logging.getLogger(__name__)


class Database:
    def __init__(self):
        self.connection_pool = None
        
    def initialize(self):
        """Инициализация пула соединений с базой данных"""
        try:
            self.connection_pool = psycopg2.pool.SimpleConnectionPool(
                1, 20,
                host=settings.postgres_host,
                port=settings.postgres_port,
                database=settings.postgres_db,
                user=settings.postgres_user,
                password=settings.postgres_password
            )
            logger.info("Database connection pool created successfully")
            self.create_tables()
        except Exception as e:
            logger.error(f"Error creating database connection pool: {e}")
            raise
    
    def create_tables(self):
        """Создание таблиц в базе данных"""
        create_table_query = """
        CREATE TABLE IF NOT EXISTS sensor_readings (
            id SERIAL PRIMARY KEY,
            sensor_value INTEGER NOT NULL,
            timestamp BIGINT NOT NULL,
            received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
        
        CREATE INDEX IF NOT EXISTS idx_timestamp ON sensor_readings(timestamp);
        CREATE INDEX IF NOT EXISTS idx_received_at ON sensor_readings(received_at);
        """
        
        with self.get_connection() as conn:
            with conn.cursor() as cursor:
                cursor.execute(create_table_query)
                conn.commit()
                logger.info("Tables created successfully")
    
    @contextmanager
    def get_connection(self):
        """Получение соединения из пула"""
        conn = self.connection_pool.getconn()
        try:
            yield conn
        finally:
            self.connection_pool.putconn(conn)
    
    def insert_reading(self, sensor_value: int, timestamp: int):
        """Вставка записи о показании датчика"""
        query = """
        INSERT INTO sensor_readings (sensor_value, timestamp)
        VALUES (%s, %s)
        RETURNING id;
        """
        
        with self.get_connection() as conn:
            with conn.cursor() as cursor:
                cursor.execute(query, (sensor_value, timestamp))
                conn.commit()
                reading_id = cursor.fetchone()[0]
                logger.debug(f"Inserted reading with ID: {reading_id}")
                return reading_id
    
    def get_readings_by_timestamp_range(self, start_timestamp: int, end_timestamp: int, limit: int = 1000):
        """Получение записей в диапазоне временных меток"""
        query = """
        SELECT id, sensor_value, timestamp, received_at, created_at
        FROM sensor_readings
        WHERE timestamp BETWEEN %s AND %s
        ORDER BY timestamp ASC
        LIMIT %s;
        """
        
        with self.get_connection() as conn:
            with conn.cursor(cursor_factory=RealDictCursor) as cursor:
                cursor.execute(query, (start_timestamp, end_timestamp, limit))
                return cursor.fetchall()
    
    def get_readings_by_datetime_range(self, start_datetime: str, end_datetime: str, limit: int = 1000):
        """Получение записей в диапазоне дат (по received_at)"""
        query = """
        SELECT id, sensor_value, timestamp, received_at, created_at
        FROM sensor_readings
        WHERE received_at BETWEEN %s AND %s
        ORDER BY received_at ASC
        LIMIT %s;
        """
        
        with self.get_connection() as conn:
            with conn.cursor(cursor_factory=RealDictCursor) as cursor:
                cursor.execute(query, (start_datetime, end_datetime, limit))
                return cursor.fetchall()
    
    def get_latest_readings(self, limit: int = 100):
        """Получение последних записей"""
        query = """
        SELECT id, sensor_value, timestamp, received_at, created_at
        FROM sensor_readings
        ORDER BY received_at DESC
        LIMIT %s;
        """
        
        with self.get_connection() as conn:
            with conn.cursor(cursor_factory=RealDictCursor) as cursor:
                cursor.execute(query, (limit,))
                return cursor.fetchall()
    
    def close(self):
        """Закрытие пула соединений"""
        if self.connection_pool:
            self.connection_pool.closeall()
            logger.info("Database connection pool closed")


# Глобальный экземпляр базы данных
db = Database()
