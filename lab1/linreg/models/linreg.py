import numpy as np
import abc as abc

from ..metrics.error import Error


class CustomLinReg(abc.ABC):
    """
    Абстрактный класс для линейной регрессии.

    Args:
        target (np.ndarray): Целевая переменная.
        features (np.ndarray): Признаки.
    """

    def __init__(
            self, 
            target: np.ndarray, 
            features: np.ndarray
        ):
        self.target = target
        self.features = features
        self.weights = None
        self.bias = None

    @abc.abstractmethod
    def fit(self, 
            target_error_value: np.float32,  
            error: Error, 
            max_iter: np.int32 = 10_000,
            learning_rate: np.float32 = 0.01
            ) -> None:
        """
        Обучает модель линейной регрессии.

        Args:
            target_error_value (np.float32): Целевое значение ошибки для остановки обучения.
            error (Error): Метрика ошибки.
            max_iter (np.int32): Максимальное количество итераций. По умолчанию 10_000.
            learning_rate (np.float32): Скорость обучения. По умолчанию 0.01.

        Returns:
            None
        """
        pass

    @abc.abstractmethod
    def predict(self, features: np.ndarray) -> np.ndarray:
        """
        Предсказывает значения на основе признаков.

        Args:
            features (np.ndarray): Признаки для предсказания.

        Returns:
            predictions (np.ndarray): Предсказанные значения.
        """
        pass


class CustomAnalLinReg(CustomLinReg):
    """
    Линейная регрессия с аналитическим решением.

    Использует формулу нормального уравнения:
    w = (X^T * X)^(-1) * X^T * y
    b = mean(y) - mean(X) * w

    Args:
        target (np.ndarray): Целевая переменная.
        features (np.ndarray): Признаки.
    """

    def fit(self, 
            target_error_value: np.float32,   
            error: Error,
            max_iter: np.int32 = 10_000,
            learning_rate: np.float32 = 0.01
        ) -> None:
        """
        Обучает модель линейной регрессии с использованием аналитического решения.

        Args:
            target_error_value (np.float32): Целевое значение ошибки (не используется в аналитическом решении).
            error (Error): Метрика ошибки (не используется в аналитическом решении).
            max_iter (np.int32): Максимальное количество итераций (не используется в аналитическом решении).
            learning_rate (np.float32): Скорость обучения (не используется в аналитическом решении).

        Returns:
            None
        """
        self.weights = np.linalg.inv(self.features.T @ self.features) @ self.features.T @ self.target
        self.bias = np.mean(self.target) - np.mean(self.features, axis=0) @ self.weights

    def predict(self, features: np.ndarray) -> np.ndarray:
        """
        Предсказывает значения на основе признаков.

        Args:
            features (np.ndarray): Признаки для предсказания.

        Returns:
            predictions (np.ndarray): Предсказанные значения.

        Raises:
            ValueError: Если модель не обучена.
        """
        if self.weights is None or self.bias is None:
            raise ValueError("Model is not fitted yet.")
        
        return features @ self.weights + self.bias


class CustomComputeLinReg(CustomLinReg):
    """
    Линейная регрессия с градиентным спуском.

    Использует градиентный спуск для оптимизации параметров:
    w = w - lr * dL/dw
    b = b - lr * dL/db

    Args:
        target (np.ndarray): Целевая переменная.
        features (np.ndarray): Признаки.
    """

    def fit(self, 
            target_error_value: np.float32,  
            error: Error, 
            max_iter: np.int32 = 10_000,
            learning_rate: np.float32 = 0.01
        ) -> None:
        """
        Обучает модель линейной регрессии с использованием градиентного спуска.

        Args:
            target_error_value (np.float32): Целевое значение ошибки для остановки обучения.
            error (Error): Метрика ошибки.
            max_iter (np.int32): Максимальное количество итераций. По умолчанию 10_000.
            learning_rate (np.float32): Скорость обучения. По умолчанию 0.01.

        Returns:
            None
        """
        self.weights = np.zeros(self.features.shape[1])
        self.bias = 0.0

        for iteration in range(max_iter):
            
            predicitions = self.predict(self.features)
            error_value = error.calculate(self.target, predicitions)

            # Проверка на NaN
            if np.isnan(error_value) or np.any(np.isnan(self.weights)) or np.isnan(self.bias):
                raise ValueError(f"NaN detected at iteration {iteration}. Try reducing learning_rate.")

            if error_value <= target_error_value:
                break

            dw = (-2 / self.features.shape[0]) * (self.features.T @ (self.target - predicitions))
            db = (-2 / self.features.shape[0]) * np.sum(self.target - predicitions)

            self.weights -= learning_rate * dw
            self.bias -= learning_rate * db        

    def predict(self, features: np.ndarray) -> np.ndarray:
        """
        Предсказывает значения на основе признаков.

        Args:
            features (np.ndarray): Признаки для предсказания.

        Returns:
            predictions (np.ndarray): Предсказанные значения.

        Raises:
            ValueError: Если модель не обучена.
        """
        if self.weights is None or self.bias is None:
            raise ValueError("Model is not fitted yet.")

        return features @ self.weights + self.bias
