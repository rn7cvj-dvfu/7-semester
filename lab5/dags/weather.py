import pandas as pd

import os
import logging
from datetime import datetime, timedelta
from airflow.decorators import dag, task
from airflow.sdk import Variable
from airflow.exceptions import AirflowSkipException


logger = logging.getLogger(__name__)

default_args = {
    'owner': 'airflow',
    'retries': 1,
    'retry_delay': timedelta(minutes=1),
}

OPENWEATHER_API_URL = "https://api.openweathermap.org/data/2.5/weather?q={city},{country_code}&lat={lat}&lon={lon}&appid={api_key}&units=metric"


@dag(
    dag_id="weather_vladivostok_dag",
    description='Сбор данных о погоде во Владивостоке',
    start_date=datetime.now(),
    schedule="*/30 * * * *",
    catchup=False,
    default_args=default_args,
    tags=["weather", "vladivostok"],
)
def weather_pipeline():

    @task
    def get_city_config() -> dict:
        """
        
        Получение конфигурации города из Airflow Variables
        
        Args:
            None
        
        Returns:
            dict: {'city': city, 'country_code': country_code, 'lat': lat, 'lon': lon}
        
        """
        city = Variable.get('WEATHER_CITY' , default=None)
        country_code = 'RU'
        lat = Variable.get('WEATHER_LATITUDE', default=None)
        lon = Variable.get('WEATHER_LONGITUDE', default=None)

        if not city or not lat or not lon:
            logger.error("Ошибка: Не установлены необходимые переменные Airflow!")
            raise AirflowSkipException("Не установлены необходимые переменные Airflow!")

        return {
            'city': city,
            'country_code': country_code,
            'lat': float(lat),
            'lon': float(lon)
        }

    @task
    def get_openweather_api_key() -> str:
        """
        
        Получение API ключа OpenWeather из Airflow Variables
        
        Args:
            None
        
        Returns:
            str: API ключ OpenWeather
        
        """
        api_key = Variable.get('OPENWEATHER_API_KEY', default=None)
        
        if not api_key:
            logger.error("OPENWEATHER_API_KEY не установлен!")
            logger.error("Установите переменную через: airflow variables set OPENWEATHER_API_KEY <your_key>")
            raise AirflowSkipException("API Key не найден")
        
        return api_key
    
    @task
    def fetch_weather_data(city: str , country_code: str , lat: float , lon: float , api_key: str) -> dict:

        """
        
        Сбор данных о погоде из OpenWeather API
        
        Args:
            city (str): Название города
            country_code (str): Код страны
            lat (float): Широта
            lon (float): Долгота
            api_key (str): API ключ OpenWeather
        
        Returns:
            dict: Данные о погоде в формате JSON
        
        """
        url = OPENWEATHER_API_URL.format(
            city=city,
            country_code=country_code,
            lat=lat,
            lon=lon,
            api_key=api_key
        )
        
        try:
            import requests
            response = requests.get(url, timeout=10)
            response.raise_for_status()
            data = response.json()
            logger.info(f"Данные о погоде получены для {city}: {data}")
            return data
        except requests.exceptions.RequestException as e:
            logger.error(f"Ошибка при запросе к OpenWeather API: {e}")
            raise AirflowSkipException("Ошибка при запросе к OpenWeather API")
        
    @task
    def process_weather_data(weather_data: dict) -> pd.DataFrame:
        """
        
        Предобработка данных о погоде и приведение их в DataFrame
        
        Args:
            weather_data (dict): Сырые данные о погоде в формате JSON
        
        Returns:
            pd.DataFrame: Обработанные данные в виде DataFrame
        
        """
        logger.info("Предобработка данных о погоде...")
        
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
                'timestamp': datetime.fromtimestamp(weather_data.get('dt', 0))
            }
            
            df = pd.DataFrame([processed])
            logger.info(f"Данные о погоде обработаны:\n{df.to_string()}")
            return df
        except Exception as e:
            logger.error(f"Ошибка предобработки данных о погоде: {e}")
            raise AirflowSkipException("Ошибка предобработки данных о погоде")
        
    
    @task
    def save_to_csv(df: pd.DataFrame , city : str  , country_code : str , lat : float , lon : float,timestamp : datetime ) -> str:
        """
        
        Сохранение данных о погоде в CSV файл
        
        Args:
            df (pd.DataFrame): Обработанные данные о погоде
        
        Returns:
            str: Путь к сохраненному файлу
        
        """
        
        save_dir = "/tmp/weather_data"
        os.makedirs(save_dir, exist_ok=True)
        csv_path = f"{save_dir}/weather_data_{city}_{country_code}_{lat}_{lon}_{timestamp.strftime('%Y%m%d%H%M%S')}.csv"

        

        try:
            df.to_csv(csv_path, index=False)
            logger.info(f"Данные о погоде сохранены в {csv_path}")
            return csv_path
        except Exception as e:
            logger.error(f"Ошибка сохранения данных в CSV: {e}")
            raise AirflowSkipException("Ошибка сохранения данных в CSV")


    config = get_city_config()
    api_key = get_openweather_api_key()
    weather_data = fetch_weather_data(
        config['city'], 
        config['country_code'], 
        config['lat'], 
        config['lon'],  
        api_key
    )
    df = process_weather_data(weather_data)
    save_to_csv(df, config['city'], config['country_code'], config['lat'], config['lon'], datetime.now())





instance = weather_pipeline()
