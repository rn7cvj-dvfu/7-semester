from fastapi import FastAPI, Query, HTTPException
from fastapi.responses import JSONResponse
from pydantic import BaseModel
from typing import List, Optional
from datetime import datetime
import logging
from contextlib import asynccontextmanager

from config import settings
from database import db
from serial_reader import serial_reader

# Настройка логирования
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


# Модели данных
class SensorReading(BaseModel):
    id: int
    sensor_value: int
    timestamp: int
    received_at: datetime
    created_at: datetime


class HealthResponse(BaseModel):
    status: str
    message: str


# Lifecycle события
@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup
    logger.info("Starting up the application...")
    
    try:
        db.initialize()
        logger.info("Database initialized")
    except Exception as e:
        logger.error(f"Failed to initialize database: {e}")
        raise
    
    try:
        serial_reader.start()
        logger.info("Serial reader started")
    except Exception as e:
        logger.warning(f"Failed to start serial reader: {e}")
        logger.warning("Application will continue without serial port connection")
    
    yield
    
    # Shutdown
    logger.info("Shutting down the application...")
    serial_reader.stop()
    db.close()


# Создание приложения
app = FastAPI(
    title="Sensor Data API",
    description="API для сбора и получения данных с датчиков",
    version="1.0.0",
    lifespan=lifespan
)


@app.get("/", response_model=HealthResponse)
async def root():
    """Корневой endpoint"""
    return HealthResponse(
        status="ok",
        message="Sensor Data API is running"
    )


@app.get("/health", response_model=HealthResponse)
async def health():
    """Проверка состояния сервера"""
    return HealthResponse(
        status="ok",
        message="Service is healthy"
    )


@app.get("/readings/latest", response_model=List[SensorReading])
async def get_latest_readings(
    limit: int = Query(default=100, ge=1, le=1000, description="Количество последних записей")
):
    """
    Получение последних записей с датчика
    
    - **limit**: Количество записей (от 1 до 1000)
    """
    try:
        readings = db.get_latest_readings(limit=limit)
        return readings
    except Exception as e:
        logger.error(f"Error fetching latest readings: {e}")
        raise HTTPException(status_code=500, detail="Internal server error")


@app.get("/readings/by-timestamp", response_model=List[SensorReading])
async def get_readings_by_timestamp(
    start: int = Query(..., description="Начальная временная метка (Unix timestamp)"),
    end: int = Query(..., description="Конечная временная метка (Unix timestamp)"),
    limit: int = Query(default=1000, ge=1, le=10000, description="Максимальное количество записей")
):
    """
    Получение записей в диапазоне временных меток (Unix timestamp)
    
    - **start**: Начальная временная метка
    - **end**: Конечная временная метка
    - **limit**: Максимальное количество записей (от 1 до 10000)
    """
    if start >= end:
        raise HTTPException(
            status_code=400,
            detail="Start timestamp must be less than end timestamp"
        )
    
    try:
        readings = db.get_readings_by_timestamp_range(start, end, limit)
        return readings
    except Exception as e:
        logger.error(f"Error fetching readings by timestamp: {e}")
        raise HTTPException(status_code=500, detail="Internal server error")


@app.get("/readings/by-datetime", response_model=List[SensorReading])
async def get_readings_by_datetime(
    start: str = Query(..., description="Начальная дата и время (ISO 8601)"),
    end: str = Query(..., description="Конечная дата и время (ISO 8601)"),
    limit: int = Query(default=1000, ge=1, le=10000, description="Максимальное количество записей")
):
    """
    Получение записей в диапазоне дат (по времени получения сервером)
    
    - **start**: Начальная дата и время (ISO 8601, например: 2026-01-26T10:00:00)
    - **end**: Конечная дата и время (ISO 8601)
    - **limit**: Максимальное количество записей (от 1 до 10000)
    """
    try:
        # Валидация формата дат
        datetime.fromisoformat(start.replace('Z', '+00:00'))
        datetime.fromisoformat(end.replace('Z', '+00:00'))
    except ValueError:
        raise HTTPException(
            status_code=400,
            detail="Invalid datetime format. Use ISO 8601 format (YYYY-MM-DDTHH:MM:SS)"
        )
    
    try:
        readings = db.get_readings_by_datetime_range(start, end, limit)
        return readings
    except Exception as e:
        logger.error(f"Error fetching readings by datetime: {e}")
        raise HTTPException(status_code=500, detail="Internal server error")


if __name__ == "__main__":
    import uvicorn
    uvicorn.run(
        "main:app",
        host=settings.server_host,
        port=settings.server_port,
        reload=True
    )
