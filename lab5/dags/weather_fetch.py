import pandas as pd
import json
import pickle
import os
import logging
import requests
from datetime import datetime, timedelta
from airflow.decorators import dag, task
from airflow.sdk import Variable
from airflow.exceptions import AirflowSkipException, AirflowException
from airflow.operators.trigger_dagrun import TriggerDagRunOperator


logger = logging.getLogger(__name__)

default_args = {
    'owner': 'airflow',
    'retries': 1,
    'retry_delay': timedelta(minutes=1),
}

OPENWEATHER_API_URL = "https://api.openweathermap.org/data/2.5/weather?q={city},{country_code}&lat={lat}&lon={lon}&appid={api_key}&units=metric"
TEMP_DIR = "/tmp/weather_data/data"


@dag(
    dag_id="weather_fetch_vladivostok_dag",
    description='Сбор данных о погоде во Владивостоке',
    start_date=datetime.now(),
    schedule="*/30 * * * *",
    catchup=False,
    default_args=default_args,
    tags=["weather", "vladivostok"],
)
def weather_fetch_pipeline():

    @task
    def get_city_config() -> str:
        """
        Получение конфигурации города из Airflow Variables
        Сохранение в JSON файл
        
        Returns:
            str: Путь к файлу с конфигурацией
        """
        city = Variable.get('WEATHER_CITY', default=None)
        country_code = 'RU'
        lat = Variable.get('WEATHER_LATITUDE', default=None)
        lon = Variable.get('WEATHER_LONGITUDE', default=None)

        if not city or not lat or not lon:
            logger.error("Ошибка: Не установлены необходимые переменные Airflow!")
            logger.error("Установите: WEATHER_CITY, WEATHER_LATITUDE, WEATHER_LONGITUDE")
            raise AirflowSkipException("Не установлены необходимые переменные Airflow!")

        config = {
            'city': city,
            'country_code': country_code,
            'lat': float(lat),
            'lon': float(lon)
        }
        
        os.makedirs(TEMP_DIR, exist_ok=True)
        config_path = f"{TEMP_DIR}/config.json"
        
        with open(config_path, 'w', encoding='utf-8') as f:
            json.dump(config, f, ensure_ascii=False, indent=2)
        
        logger.info(f"Конфигурация сохранена в {config_path}: {config}")
        return config_path

    @task
    def get_openweather_api_key() -> str:
        """
        Получение API ключа OpenWeather из Airflow Variables
        Сохранение в файл
        
        Returns:
            str: Путь к файлу с API ключом
        """
        api_key = Variable.get('OPENWEATHER_API_KEY', default=None)
        
        if not api_key:
            logger.error("OPENWEATHER_API_KEY не установлен!")
            logger.error("Установите переменную через: airflow variables set OPENWEATHER_API_KEY <your_key>")
            raise AirflowSkipException("API Key не найден")
        
        os.makedirs(TEMP_DIR, exist_ok=True)
        key_path = f"{TEMP_DIR}/api_key.txt"
        
        with open(key_path, 'w') as f:
            f.write(api_key)
        
        logger.info(f"API ключ сохранен в {key_path}")
        return key_path
    
    @task
    def fetch_weather_data(config_path: str, api_key_path: str) -> str:
        """
        Сбор данных о погоде из OpenWeather API
        
        Args:
            config_path: Путь к файлу с конфигурацией города
            api_key_path: Путь к файлу с API ключом
        
        Returns:
            str: Путь к JSON файлу с данными о погоде
        """
        # Чтение конфигурации
        try:
            with open(config_path, 'r', encoding='utf-8') as f:
                config = json.load(f)
            logger.info(f"Конфигурация загружена: {config}")
        except (FileNotFoundError, json.JSONDecodeError) as e:
            logger.error(f"Ошибка чтения конфигурации: {e}")
            raise AirflowException(f"Не удалось загрузить конфигурацию: {e}")
        
        # Чтение API ключа
        try:
            with open(api_key_path, 'r') as f:
                api_key = f.read().strip()
            logger.info("API ключ загружен")
        except FileNotFoundError as e:
            logger.error(f"Ошибка чтения API ключа: {e}")
            raise AirflowException(f"Не удалось загрузить API ключ: {e}")
        
        # Формирование URL
        url = OPENWEATHER_API_URL.format(
            city=config['city'],
            country_code=config['country_code'],
            lat=config['lat'],
            lon=config['lon'],
            api_key=api_key
        )
        
        logger.info(f"Запрос к API для города: {config['city']}")
        
        try:

            
            response = requests.get(url, timeout=10)
            

            if response.status_code == 401:
                logger.error("Ошибка 401: Неверный API ключ")
                raise AirflowException("Неверный API ключ OpenWeather")
            elif response.status_code == 404:
                logger.error(f"Ошибка 404: Город {config['city']} не найден")
                raise AirflowException(f"Город {config['city']} не найден в OpenWeather")
            elif response.status_code == 429:
                logger.error("Ошибка 429: Превышен лимит запросов к API")
                raise AirflowException("Превышен лимит запросов к OpenWeather API")
            elif response.status_code >= 500:
                logger.error(f"Ошибка сервера OpenWeather: {response.status_code}")
                raise AirflowException(f"Ошибка сервера OpenWeather: {response.status_code}")
            
            response.raise_for_status()
            data = response.json()
            

            if 'main' not in data or 'weather' not in data:
                logger.error(f"Неполные данные от API: {data}")
                raise AirflowException("Получены неполные данные от OpenWeather API")
            
            logger.info(f"Данные о погоде получены для {config['city']}")
            logger.info(f"Температура: {data.get('main', {}).get('temp')}°C")
            logger.info(f"Погода: {data.get('weather', [{}])[0].get('description')}")
            

            weather_path = f"{TEMP_DIR}/weather_raw.json"
            with open(weather_path, 'w', encoding='utf-8') as f:
                json.dump(data, f, ensure_ascii=False, indent=2)
            
            logger.info(f"Сырые данные сохранены в {weather_path}")
            return weather_path
            
        except requests.exceptions.Timeout:
            logger.error("Timeout: Превышено время ожидания ответа от API")
            raise AirflowException("Timeout при запросе к OpenWeather API")
        except requests.exceptions.ConnectionError:
            logger.error("Connection Error: Не удалось подключиться к API")
            raise AirflowException("Ошибка подключения к OpenWeather API")
        except requests.exceptions.RequestException as e:
            logger.error(f"Ошибка при запросе к OpenWeather API: {e}")
            raise AirflowException(f"Ошибка при запросе к OpenWeather API: {e}")
        except json.JSONDecodeError as e:
            logger.error(f"Ошибка парсинга JSON ответа: {e}")
            raise AirflowException(f"Неверный формат ответа от API: {e}")
        
    @task
    def process_weather_data(weather_path: str) -> str:
        """
        Предобработка данных о погоде
        
        Args:
            weather_path: Путь к JSON файлу с данными о погоде
        
        Returns:
            str: Путь к pickle файлу с DataFrame
        """
        logger.info("Предобработка данных о погоде...")
        

        try:
            with open(weather_path, 'r', encoding='utf-8') as f:
                weather_data = json.load(f)
        except (FileNotFoundError, json.JSONDecodeError) as e:
            logger.error(f"Ошибка чтения данных о погоде: {e}")
            raise AirflowException(f"Не удалось загрузить данные о погоде: {e}")
        
        try:
            processed = {
                'city': weather_data.get('name'),
                'country': weather_data.get('sys', {}).get('country'),
                'temperature': weather_data.get('main', {}).get('temp'),
                'feels_like': weather_data.get('main', {}).get('feels_like'),
                'temp_min': weather_data.get('main', {}).get('temp_min'),
                'temp_max': weather_data.get('main', {}).get('temp_max'),
                'pressure': weather_data.get('main', {}).get('pressure'),
                'humidity': weather_data.get('main', {}).get('humidity'),
                'weather_main': weather_data.get('weather', [{}])[0].get('main'),
                'weather_description': weather_data.get('weather', [{}])[0].get('description'),
                'wind_speed': weather_data.get('wind', {}).get('speed'),
                'wind_deg': weather_data.get('wind', {}).get('deg'),
                'clouds': weather_data.get('clouds', {}).get('all'),
                'visibility': weather_data.get('visibility'),
                'sunrise': datetime.fromtimestamp(weather_data.get('sys', {}).get('sunrise', 0)),
                'sunset': datetime.fromtimestamp(weather_data.get('sys', {}).get('sunset', 0)),
                'timestamp': datetime.fromtimestamp(weather_data.get('dt', 0)),
                'collection_time': datetime.now()
            }
            
            df = pd.DataFrame([processed])
            logger.info(f"Данные о погоде обработаны:\n{df.to_string()}")

            timestamp = datetime.now()
            df_path = f"{TEMP_DIR}/weather_df_{timestamp.strftime('%Y%m%d_%H%M%S')}.pkl"
            with open(df_path, 'wb') as f:
                pickle.dump(df, f)
            
            logger.info(f"DataFrame сохранен в {df_path}")
            return df_path
            
        except Exception as e:
            logger.error(f"Ошибка предобработки данных о погоде: {e}")
            raise AirflowException(f"Ошибка предобработки данных: {e}")
    
    @task
    def save_to_csv(df_path: str, config_path: str) -> str:
        """
        Сохранение данных о погоде в CSV файл
        
        Args:
            df_path: Путь к pickle файлу с DataFrame
            config_path: Путь к файлу с конфигурацией
        
        Returns:
            str: Путь к сохраненному CSV файлу
        """

        try:
            with open(df_path, 'rb') as f:
                df = pickle.load(f)
            logger.info("DataFrame загружен")
        except (FileNotFoundError, pickle.PickleError) as e:
            logger.error(f"Ошибка загрузки DataFrame: {e}")
            raise AirflowException(f"Не удалось загрузить DataFrame: {e}")
        

        try:
            with open(config_path, 'r', encoding='utf-8') as f:
                config = json.load(f)
        except (FileNotFoundError, json.JSONDecodeError) as e:
            logger.error(f"Ошибка загрузки конфигурации: {e}")
            raise AirflowException(f"Не удалось загрузить конфигурацию: {e}")
        

        timestamp = datetime.now()
        csv_filename = f"weather_data_{config['city']}_{config['country_code']}_{timestamp.strftime('%Y%m%d_%H%M%S')}.csv"
        csv_path = f"{TEMP_DIR}/{csv_filename}"
        
        try:
            df.to_csv(csv_path, index=False)
            logger.info(f"Данные о погоде сохранены в {csv_path}")
            logger.info(f"Размер файла: {os.path.getsize(csv_path)} байт")
            return csv_path
        except Exception as e:
            logger.error(f"Ошибка сохранения данных в CSV: {e}")
            raise AirflowException(f"Ошибка сохранения данных в CSV: {e}")


    config_path = get_city_config()
    api_key_path = get_openweather_api_key()
    weather_path = fetch_weather_data(config_path, api_key_path)
    df_path = process_weather_data(weather_path)
    csv_path = save_to_csv(df_path, config_path)    

    trigger_training = TriggerDagRunOperator(
        task_id='trigger_model_training',
        trigger_dag_id='weather_model_training_dag',
    )

    csv_path >> trigger_training
    

instance = weather_fetch_pipeline()
