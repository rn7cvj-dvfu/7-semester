import serial
import threading
import logging
from config import settings
from database import db

logger = logging.getLogger(__name__)


class SerialReader:
    def __init__(self):
        self.serial_port = None
        self.running = False
        self.thread = None
        
    def start(self):
        """Запуск чтения с последовательного порта"""
        if self.running:
            logger.warning("Serial reader is already running")
            return
        
        try:
            self.serial_port = serial.Serial(
                port=settings.serial_port,
                baudrate=settings.serial_baudrate,
                timeout=settings.serial_timeout
            )
            logger.info(f"Opened serial port: {settings.serial_port}")
            
            self.running = True
            self.thread = threading.Thread(target=self._read_loop, daemon=True)
            self.thread.start()
            logger.info("Serial reader thread started")
            
        except serial.SerialException as e:
            logger.error(f"Failed to open serial port {settings.serial_port}: {e}")
            raise
    
    def _read_loop(self):
        """Цикл чтения данных из последовательного порта"""
        logger.info("Starting serial read loop")
        
        while self.running:
            try:
                if self.serial_port and self.serial_port.in_waiting > 0:
                    line = self.serial_port.readline().decode('utf-8').strip()
                    
                    if line:
                        self._process_line(line)
                        
            except Exception as e:
                logger.error(f"Error reading from serial port: {e}")
    
    def _process_line(self, line: str):
        """Обработка строки данных из порта"""
        try:
            # Формат: sensor_value|timestamp
            parts = line.split('|')
            
            if len(parts) != 2:
                logger.warning(f"Invalid data format: {line}")
                return
            
            sensor_value = int(parts[0])
            timestamp = int(parts[1])
            
            # Сохранение в базу данных
            db.insert_reading(sensor_value, timestamp)
            logger.info(f"Saved reading: value={sensor_value}, timestamp={timestamp}")
            
        except ValueError as e:
            logger.error(f"Error parsing data '{line}': {e}")
        except Exception as e:
            logger.error(f"Error processing line '{line}': {e}")
    
    def stop(self):
        """Остановка чтения с последовательного порта"""
        if not self.running:
            return
        
        logger.info("Stopping serial reader")
        self.running = False
        
        if self.thread:
            self.thread.join(timeout=5)
        
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            logger.info("Serial port closed")


# Глобальный экземпляр читателя
serial_reader = SerialReader()
