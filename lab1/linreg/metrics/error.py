import numpy as np
import abc as abc


class Error(abc.ABC):
    """
    Абстрактный класс для метрик ошибок.
    """

    @abc.abstractmethod
    def calculate(self, y_true: np.ndarray, y_pred: np.ndarray) -> float:
        """
        Вычисляет значение метрики ошибки.

        Args:
            y_true (np.ndarray): Истинные значения.
            y_pred (np.ndarray): Предсказанные значения.

        Returns:
            error (float): Значение метрики ошибки.
        """
        pass


class MSE(Error):
    """
    Средняя квадратичная ошибка (Mean Squared Error).
    """

    def calculate(self, y_true: np.ndarray, y_pred: np.ndarray) -> float:
        """
        Вычисляет значение MSE.

        Args:
            y_true (np.ndarray): Истинные значения.
            y_pred (np.ndarray): Предсказанные значения.

        Returns:
            mse (float): Значение MSE.
        """
        return np.mean((y_true - y_pred) ** 2)


class MAE(Error):
    """
    Средняя абсолютная ошибка (Mean Absolute Error).
    """

    def calculate(self, y_true: np.ndarray, y_pred: np.ndarray) -> float:
        """
        Вычисляет значение MAE.

        Args:
            y_true (np.ndarray): Истинные значения.
            y_pred (np.ndarray): Предсказанные значения.

        Returns:
            mae (float): Значение MAE.
        """
        return np.mean(np.abs(y_true - y_pred))
