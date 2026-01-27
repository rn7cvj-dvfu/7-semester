from pydantic_settings import BaseSettings
from pydantic import Field


class Settings(BaseSettings):
    # Database
    postgres_host: str = Field(default="localhost", alias="POSTGRES_HOST")
    postgres_port: int = Field(default=5432, alias="POSTGRES_PORT")
    postgres_db: str = Field(default="sensor_data", alias="POSTGRES_DB")
    postgres_user: str = Field(default="postgres", alias="POSTGRES_USER")
    postgres_password: str = Field(default="postgres", alias="POSTGRES_PASSWORD")
    
    # Serial Port
    serial_port: str = Field(default="COM0", alias="SERIAL_PORT")
    serial_baudrate: int = Field(default=9600, alias="SERIAL_BAUDRATE")
    serial_timeout: int = Field(default=1, alias="SERIAL_TIMEOUT")
    
    # Server
    server_host: str = Field(default="0.0.0.0", alias="SERVER_HOST")
    server_port: int = Field(default=8000, alias="SERVER_PORT")
    
    class Config:
        env_file = ".env"
        case_sensitive = False


settings = Settings()
