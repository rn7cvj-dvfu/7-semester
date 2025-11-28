import logging
import requests
import pandas as pd
from datetime import datetime, timedelta
from airflow.decorators import dag, task
from airflow.models import Variable
import psycopg2
from psycopg2.extras import execute_values

# Логирование
logger = logging.getLogger(__name__)

# Конфигурация из Airflow Variables
CITY = Variable.get('WEATHER_CITY', default_var='Vladivostok')
COUNTRY_CODE = 'RU'
LAT = float(Variable.get('WEATHER_LATITUDE', default_var='43.1056'))
LON = float(Variable.get('WEATHER_LONGITUDE', default_var='131.8735'))

# БД
DB_HOST = 'postgres'
DB_USER = 'airflow'
DB_PASSWORD = 'airflow'
DB_NAME = 'airflow'
DB_PORT = 5432

# ========== ФУНКЦИИ PIPELINE ==========

def connect_to_db():
    """Подключение к PostgreSQL"""
    logger.info("Подключение к БД...")
    try:
        conn = psycopg2.connect(
            host=DB_HOST,
            user=DB_USER,
            password=DB_PASSWORD,
            database=DB_NAME,
            port=DB_PORT
        )
        logger.info("✓ Подключение успешно")
        return conn
    except Exception as e:
        logger.error(f"✗ Ошибка подключения: {e}")
        raise

@task
def create_table_task():
    """Создание таблицы для данных о погоде"""
    logger.info("Проверка/создание таблицы weather_data...")
    conn = connect_to_db()
    cursor = conn.cursor()
    
    try:
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS weather_data (
                id SERIAL PRIMARY KEY,
                city VARCHAR(100),
                country VARCHAR(100),
                temperature FLOAT,
                feels_like FLOAT,
                temp_min FLOAT,
                temp_max FLOAT,
                pressure INT,
                humidity INT,
                weather_main VARCHAR(100),
                weather_description TEXT,
                wind_speed FLOAT,
                wind_deg INT,
                clouds INT,
                timestamp TIMESTAMP,
                collected_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
            
            CREATE INDEX IF NOT EXISTS idx_weather_timestamp 
            ON weather_data(timestamp);
        ''')
        conn.commit()
        logger.info("✓ Таблица готова")
    except Exception as e:
        logger.error(f"✗ Ошибка создания таблицы: {e}")
        conn.rollback()
        raise
    finally:
        cursor.close()
        conn.close()

@task
def fetch_weather_task():
    """Сбор данных о погоде из API"""
    api_key = Variable.get('OPENWEATHER_API_KEY', default_var=None)
    
    if not api_key:
        logger.error("✗ OPENWEATHER_API_KEY не установлен!")
        logger.error("Установите переменную через: airflow variables set OPENWEATHER_API_KEY <your_key>")
        raise ValueError("API Key не найден")
    
    logger.info(f"Сбор данных погоды для {CITY}...")
    
    url = f"https://api.openweathermap.org/data/2.5/weather?lat={LAT}&lon={LON}&appid={api_key}&units=metric"
    
    try:
        response = requests.get(url, timeout=10)
        response.raise_for_status()
        data = response.json()
        logger.info(f"✓ Данные получены: {data.get('main', {}).get('temp')}°C")
        return data
    except requests.exceptions.RequestException as e:
        logger.error(f"✗ Ошибка при запросе к API: {e}")
        raise

@task
def preprocess_data_task(raw_data):
    """Предобработка данных и приведение в DataFrame"""
    logger.info("Предобработка данных...")
    
    try:
        # Извлечение нужных полей
        processed = {
            'city': raw_data.get('name', CITY),
            'country': raw_data.get('sys', {}).get('country', COUNTRY_CODE),
            'temperature': raw_data.get('main', {}).get('temp'),
            'feels_like': raw_data.get('main', {}).get('feels_like'),
            'temp_min': raw_data.get('main', {}).get('temp_min'),
            'temp_max': raw_data.get('main', {}).get('temp_max'),
            'pressure': raw_data.get('main', {}).get('pressure'),
            'humidity': raw_data.get('main', {}).get('humidity'),
            'weather_main': raw_data.get('weather', [{}])[0].get('main'),
            'weather_description': raw_data.get('weather', [{}])[0].get('description'),
            'wind_speed': raw_data.get('wind', {}).get('speed'),
            'wind_deg': raw_data.get('wind', {}).get('deg'),
            'clouds': raw_data.get('clouds', {}).get('all'),
            'timestamp': datetime.fromtimestamp(raw_data.get('dt', 0))
        }
        
        df = pd.DataFrame([processed])
        logger.info(f"✓ Данные обработаны:\n{df.to_string()}")
        
        # Сериализовать DataFrame для передачи между тасками
        return df.to_json(orient='records')
    except Exception as e:
        logger.error(f"✗ Ошибка предобработки: {e}")
        raise

@task
def save_to_postgres_task(df_json):
    """Сохранение данных в PostgreSQL"""
    logger.info("Сохранение данных в БД...")
    
    # Десериализировать JSON обратно в DataFrame
    df = pd.read_json(df_json, orient='records')
    
    conn = connect_to_db()
    cursor = conn.cursor()
    
    try:
        # Подготовка данных для вставки
        rows = [
            (
                row['city'],
                row['country'],
                float(row['temperature']) if pd.notna(row['temperature']) else None,
                float(row['feels_like']) if pd.notna(row['feels_like']) else None,
                float(row['temp_min']) if pd.notna(row['temp_min']) else None,
                float(row['temp_max']) if pd.notna(row['temp_max']) else None,
                int(row['pressure']) if pd.notna(row['pressure']) else None,
                int(row['humidity']) if pd.notna(row['humidity']) else None,
                row['weather_main'],
                row['weather_description'],
                float(row['wind_speed']) if pd.notna(row['wind_speed']) else None,
                int(row['wind_deg']) if pd.notna(row['wind_deg']) else None,
                int(row['clouds']) if pd.notna(row['clouds']) else None,
                row['timestamp']
            )
            for _, row in df.iterrows()
        ]
        
        # Вставка данных
        query = '''
            INSERT INTO weather_data 
            (city, country, temperature, feels_like, temp_min, temp_max, 
             pressure, humidity, weather_main, weather_description, 
             wind_speed, wind_deg, clouds, timestamp)
            VALUES %s
        '''
        
        execute_values(cursor, query, rows)
        conn.commit()
        logger.info(f"✓ {len(rows)} записей сохранено в БД")
        
    except Exception as e:
        logger.error(f"✗ Ошибка сохранения: {e}")
        conn.rollback()
        raise
    finally:
        cursor.close()
        conn.close()

# ========== DAG DEFINITION ==========

@dag(
    dag_id='weather_vladivostok_dag',
    description='Сбор данных о погоде во Владивостоке',
    schedule_interval='*/30 * * * *',  # Каждые 30 минут
    start_date=datetime(2024, 1, 1),
    catchup=False,
    tags=['weather', 'openweathermap'],
    default_args={
        'owner': 'airflow',
        'retries': 2,
        'retry_delay': timedelta(minutes=5),
    }
)
def weather_pipeline():
    """DAG для сбора и сохранения данных о погоде"""
    logger.info("=" * 50)
    logger.info("ЗАПУСК WEATHER PIPELINE")
    logger.info(f"Город: {CITY}, Координаты: ({LAT}, {LON})")
    logger.info("=" * 50)
    
    # Создание таблицы
    create_table = create_table_task()
    
    # Сбор данных
    weather_data = fetch_weather_task()
    
    # Предобработка
    processed_data = preprocess_data_task(weather_data)
    
    # Сохранение
    save_data = save_to_postgres_task(processed_data)
    
    # Определение порядка выполнения
    create_table >> weather_data >> processed_data >> save_data

# Instantiate DAG
weather_dag = weather_pipeline()


if __name__ == "__main__":
    # Для локального тестирования
    weather_dag.test()
