"""
Метрики разбиения для деревьев регрессии.
"""

import numpy as np
import abc as abc


class SplitMetric(abc.ABC):
    """
    Абстрактный класс для метрик разбиения узлов в дереве решений.
    """

    @abc.abstractmethod
    def calculate(self, y: np.ndarray) -> float:
        """
        Вычисляет значение метрики для узла.

        Args:
            y (np.ndarray): Целевые значения в узле.

        Returns:
            metric (float): Значение метрики.
        """
        pass

    @abc.abstractmethod
    def calculate_split_quality(
        self, 
        y_left: np.ndarray, 
        y_right: np.ndarray
    ) -> float:
        """
        Вычисляет качество разбиения.

        Args:
            y_left (np.ndarray): Целевые значения в левом дочернем узле.
            y_right (np.ndarray): Целевые значения в правом дочернем узле.

        Returns:
            quality (float): Качество разбиения (чем меньше, тем лучше).
        """
        pass


class MSESplitMetric(SplitMetric):
    """
    Метрика разбиения на основе среднеквадратичной ошибки (MSE).
    Используется для деревьев регрессии.

    Формула:
        MSE = (1/n) * Σ(yᵢ - ȳ)²
        
        где:
        - n — количество образцов в узле
        - yᵢ — значение i-го образца
        - ȳ — среднее значение в узле

    Взвешенное качество разбиения:
        Q = (n_left/n) * MSE_left + (n_right/n) * MSE_right
    """

    def calculate(self, y: np.ndarray) -> float:
        """
        Вычисляет MSE для узла.

        Args:
            y (np.ndarray): Целевые значения в узле.

        Returns:
            mse (float): Среднеквадратичная ошибка.
        """
        if len(y) == 0:
            return 0.0
        mean = np.mean(y)
        return np.mean((y - mean) ** 2)

    def calculate_split_quality(
        self, 
        y_left: np.ndarray, 
        y_right: np.ndarray
    ) -> float:
        """
        Вычисляет взвешенную MSE после разбиения.

        Args:
            y_left (np.ndarray): Целевые значения в левом дочернем узле.
            y_right (np.ndarray): Целевые значения в правом дочернем узле.

        Returns:
            weighted_mse (float): Взвешенная MSE.
        """
        n_left = len(y_left)
        n_right = len(y_right)
        n_total = n_left + n_right
        
        if n_total == 0:
            return 0.0
        
        mse_left = self.calculate(y_left)
        mse_right = self.calculate(y_right)
        
        weighted_mse = (n_left / n_total) * mse_left + (n_right / n_total) * mse_right
        return weighted_mse


class MAESplitMetric(SplitMetric):
    """
    Метрика разбиения на основе средней абсолютной ошибки (MAE).
    Используется для деревьев регрессии.

    Формула:
        MAE = (1/n) * Σ|yᵢ - median(y)|
        
        где:
        - n — количество образцов в узле
        - yᵢ — значение i-го образца
        - median(y) — медиана значений в узле

    Взвешенное качество разбиения:
        Q = (n_left/n) * MAE_left + (n_right/n) * MAE_right
    """

    def calculate(self, y: np.ndarray) -> float:
        """
        Вычисляет MAE для узла.

        Args:
            y (np.ndarray): Целевые значения в узле.

        Returns:
            mae (float): Средняя абсолютная ошибка.
        """
        if len(y) == 0:
            return 0.0
        median = np.median(y)
        return np.mean(np.abs(y - median))

    def calculate_split_quality(
        self, 
        y_left: np.ndarray, 
        y_right: np.ndarray
    ) -> float:
        """
        Вычисляет взвешенную MAE после разбиения.

        Args:
            y_left (np.ndarray): Целевые значения в левом дочернем узле.
            y_right (np.ndarray): Целевые значения в правом дочернем узле.

        Returns:
            weighted_mae (float): Взвешенная MAE.
        """
        n_left = len(y_left)
        n_right = len(y_right)
        n_total = n_left + n_right
        
        if n_total == 0:
            return 0.0
        
        mae_left = self.calculate(y_left)
        mae_right = self.calculate(y_right)
        
        weighted_mae = (n_left / n_total) * mae_left + (n_right / n_total) * mae_right
        return weighted_mae
