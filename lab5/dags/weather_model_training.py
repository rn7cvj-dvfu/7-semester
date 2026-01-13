import pandas as pd
import numpy as np
import pickle
import os
import logging
import glob
from datetime import datetime, timedelta
from sklearn.linear_model import LinearRegression
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import mean_absolute_error, mean_squared_error, r2_score
from airflow.decorators import dag, task
from airflow.exceptions import AirflowSkipException, AirflowException


logger = logging.getLogger(__name__)

default_args = {
    'owner': 'airflow',
    'retries': 1,
    'retry_delay': timedelta(minutes=1),
}

DATA_DIR = "/tmp/weather_data/data"  # Директория с CSV файлами погоды (должна совпадать с weather_fetch.py)
MODEL_DIR = "/tmp/weather_data/models"  # Директория для сохранения моделей
TRAINING_DATA_DIR = "/tmp/weather_data/training_data"  # Директория для обучающих данных
PREDICTIONS_DIR = "/tmp/weather_data/predictions"  # Директория для результатов предсказаний


@dag(
    dag_id="weather_model_training_dag",
    description='Дообучение модели предсказания температуры на основе данных о погоде',
    start_date=datetime.now(),
    schedule=None, 
    catchup=False,
    default_args=default_args,
    tags=["weather", "machine-learning", "training"],
)
def weather_model_training_pipeline():

    @task
    def load_latest_weather_data() -> str:
        """
        Загрузка последних данных о погоде из CSV файла
        
        Returns:
            str: Путь к pickle файлу с DataFrame
        """
        logger.info("Поиск последних данных о погоде...")
        
        csv_files = glob.glob(f"{DATA_DIR}/weather_data_*.csv")
        
        if not csv_files:
            logger.error("Не найдено CSV файлов с данными о погоде")
            raise AirflowSkipException("Нет данных для обучения")
        
        latest_csv = max(csv_files, key=os.path.getmtime)
        logger.info(f"Загрузка данных из {latest_csv}")
        
        try:
            df = pd.read_csv(latest_csv)
            logger.info(f"Загружено {len(df)} записей")
            logger.info(f"Столбцы: {df.columns.tolist()}")
            
            os.makedirs(TRAINING_DATA_DIR, exist_ok=True)
            df_path = f"{TRAINING_DATA_DIR}/current_data.pkl"
            with open(df_path, 'wb') as f:
                pickle.dump(df, f)
            
            return df_path
        except Exception as e:
            logger.error(f"Ошибка загрузки данных: {e}")
            raise AirflowException(f"Ошибка загрузки данных: {e}")

    @task
    def load_historical_data(current_data_path: str) -> str:
        """
        Объединение текущих данных с историческими
        
        Args:
            current_data_path: Путь к текущим данным
        
        Returns:
            str: Путь к объединенным данным
        """
        logger.info("Загрузка исторических данных...")
        
        with open(current_data_path, 'rb') as f:
            current_df = pickle.load(f)
        
        os.makedirs(TRAINING_DATA_DIR, exist_ok=True)
        history_path = f"{TRAINING_DATA_DIR}/historical_data.pkl"
        
        if os.path.exists(history_path):
            try:
                with open(history_path, 'rb') as f:
                    historical_df = pickle.load(f)

                logger.info(f"Загружено {len(historical_df)} исторических записей")
                
                combined_df = pd.concat([historical_df, current_df], ignore_index=True)
                
                combined_df = combined_df.drop_duplicates(subset=['timestamp'], keep='last')
                
            
    
                logger.info(f"Объединено {len(combined_df)} записей")
            except Exception as e:
                logger.warning(f"Ошибка загрузки истории, используем только текущие данные: {e}")
                combined_df = current_df
        else:
            logger.info("Исторические данные не найдены, используем только текущие")
            combined_df = current_df
        
        combined_path = f"{TRAINING_DATA_DIR}/combined_data.pkl"
        with open(combined_path, 'wb') as f:
            pickle.dump(combined_df, f)
        
        with open(history_path, 'wb') as f:
            pickle.dump(combined_df, f)
        
        return combined_path

    @task
    def prepare_features(data_path: str) -> str:
        """
        Подготовка признаков для обучения модели
        
        Args:
            data_path: Путь к данным
        
        Returns:
            str: Путь к подготовленным данным
        """
        logger.info("Подготовка признаков...")
        
        with open(data_path, 'rb') as f:
            df = pickle.load(f)
        

        required_columns = ['temperature', 'pressure', 'humidity', 'wind_speed']
        missing_columns = [col for col in required_columns if col not in df.columns]
        
        if missing_columns:
            logger.error(f"Отсутствуют столбцы: {missing_columns}")
            raise AirflowException(f"Отсутствуют необходимые столбцы: {missing_columns}")
        

        df_clean = df[required_columns].dropna()
        
        if len(df_clean) == 0:
            logger.error("Нет данных после очистки")
            raise AirflowSkipException("Недостаточно данных для обучения")
        
        logger.info(f"Подготовлено {len(df_clean)} записей для обучения")
        logger.info(f"Диапазон температур: {df_clean['temperature'].min():.2f} - {df_clean['temperature'].max():.2f}°C")
        
       
        prepared_path = f"{TRAINING_DATA_DIR}/prepared_features.pkl"
        with open(prepared_path, 'wb') as f:
            pickle.dump(df_clean, f)
        
        return prepared_path

    @task
    def train_model(features_path: str) -> str:
        """
        Обучение модели LinearRegression для предсказания температуры
        
        Args:
            features_path: Путь к подготовленным признакам
        
        Returns:
            str: Путь к обученной модели
        """
        logger.info("Обучение модели линейной регрессии...")
        
        with open(features_path, 'rb') as f:
            df = pickle.load(f)
        
        X = df[['pressure', 'humidity', 'wind_speed']].values
        y = df['temperature'].values
        
        logger.info(f"Обучающая выборка: {X.shape[0]} примеров, {X.shape[1]} признаков")
        
        os.makedirs(MODEL_DIR, exist_ok=True)
        model_path = f"{MODEL_DIR}/temperature_model.pkl"
        scaler_path = f"{MODEL_DIR}/scaler.pkl"
        
        if os.path.exists(scaler_path):
            with open(scaler_path, 'rb') as f:
                scaler = pickle.load(f)
            logger.info("Загружен существующий scaler")
        else:
            scaler = StandardScaler()
            logger.info("Создан новый scaler")
        

        X_scaled = scaler.fit_transform(X)
        

        model = LinearRegression()
        model.fit(X_scaled, y)
        
        if os.path.exists(model_path):
            logger.info("Модель переобучена на обновленных данных")
        else:
            logger.info("Создана и обучена новая модель")
        

        feature_names = ['pressure', 'humidity', 'wind_speed']
        logger.info("Коэффициенты модели:")
        for name, coef in zip(feature_names, model.coef_):
            logger.info(f"  {name}: {coef:.4f}")
        logger.info(f"  intercept (свободный член): {model.intercept_:.4f}")
        

        y_pred = model.predict(X_scaled)
        mae = mean_absolute_error(y, y_pred)
        rmse = np.sqrt(mean_squared_error(y, y_pred))
        r2 = r2_score(y, y_pred)
        
        logger.info(f"Метрики модели: MAE={mae:.2f}°C, RMSE={rmse:.2f}°C, R²={r2:.4f}")
        
   
        with open(model_path, 'wb') as f:
            pickle.dump(model, f)
        
        with open(scaler_path, 'wb') as f:
            pickle.dump(scaler, f)
        
        logger.info(f"Модель сохранена в {model_path}")
        logger.info(f"Scaler сохранен в {scaler_path}")
        
        return model_path

    @task
    def make_prediction(model_path: str, features_path: str) -> str:
        """
        Создание предсказаний с помощью обученной модели
        
        Args:
            model_path: Путь к модели
            features_path: Путь к признакам
        
        Returns:
            str: Путь к файлу с предсказаниями
        """
        logger.info("Создание предсказаний...")
        
        with open(model_path, 'rb') as f:
            model = pickle.load(f)
        
        scaler_path = f"{MODEL_DIR}/scaler.pkl"
        with open(scaler_path, 'rb') as f:
            scaler = pickle.load(f)
        
        with open(features_path, 'rb') as f:
            df = pickle.load(f)
        

        X = df[['pressure', 'humidity', 'wind_speed']].values
        X_scaled = scaler.transform(X)
        
        y_pred = model.predict(X_scaled)
        y_true = df['temperature'].values
        
        results = pd.DataFrame({
            'actual_temperature': y_true,
            'predicted_temperature': y_pred,
            'error': y_true - y_pred,
            'absolute_error': np.abs(y_true - y_pred),
            'pressure': df['pressure'].values,
            'humidity': df['humidity'].values,
            'wind_speed': df['wind_speed'].values
        })
        
        logger.info("Примеры предсказаний:")
        logger.info(results.head().to_string())
        

        os.makedirs(PREDICTIONS_DIR, exist_ok=True)
        predictions_path = f"{PREDICTIONS_DIR}/predictions_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
        results.to_csv(predictions_path, index=False)
        
        logger.info(f"Предсказания сохранены в {predictions_path}")
        
        return predictions_path

    data_path = load_latest_weather_data()
    combined_path = load_historical_data(data_path)
    features_path = prepare_features(combined_path)
    model_path = train_model(features_path)
    predictions_path = make_prediction(model_path, features_path)


instance = weather_model_training_pipeline()
