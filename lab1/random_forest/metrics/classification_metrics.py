import numpy as np
import abc as abc


class ClassificationMetric(abc.ABC):
    """
    Абстрактный класс для метрик классификации при построении деревьев.
    """

    @abc.abstractmethod
    def calculate(self, y: np.ndarray) -> float:
        """
        Вычисляет значение метрики для узла.

        Args:
            y (np.ndarray): Метки классов в узле.

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
            y_left (np.ndarray): Метки классов в левом дочернем узле.
            y_right (np.ndarray): Метки классов в правом дочернем узле.

        Returns:
            quality (float): Качество разбиения (чем меньше, тем лучше).
        """
        pass


class GiniMetric(ClassificationMetric):
    """
    Метрика разбиения на основе индекса Джини.
    Используется для деревьев классификации.
    """

    def calculate(self, y: np.ndarray) -> float:
        """
        Вычисляет индекс Джини для узла.

        Args:
            y (np.ndarray): Метки классов в узле.

        Returns:
            gini (float): Индекс Джини.
        """
        if len(y) == 0:
            return 0.0
        
        _, counts = np.unique(y, return_counts=True)
        probabilities = counts / len(y)
        gini = 1.0 - np.sum(probabilities ** 2)
        return gini

    def calculate_split_quality(
        self, 
        y_left: np.ndarray, 
        y_right: np.ndarray
    ) -> float:
        """
        Вычисляет взвешенный индекс Джини после разбиения.

        Args:
            y_left (np.ndarray): Метки классов в левом дочернем узле.
            y_right (np.ndarray): Метки классов в правом дочернем узле.

        Returns:
            weighted_gini (float): Взвешенный индекс Джини.
        """
        n_left = len(y_left)
        n_right = len(y_right)
        n_total = n_left + n_right
        
        if n_total == 0:
            return 0.0
        
        gini_left = self.calculate(y_left)
        gini_right = self.calculate(y_right)
        
        weighted_gini = (n_left / n_total) * gini_left + (n_right / n_total) * gini_right
        return weighted_gini


class EntropyMetric(ClassificationMetric):
    """
    Метрика разбиения на основе энтропии.
    Используется для деревьев классификации.
    """

    def calculate(self, y: np.ndarray) -> float:
        """
        Вычисляет энтропию для узла.

        Args:
            y (np.ndarray): Метки классов в узле.

        Returns:
            entropy (float): Энтропия.
        """
        if len(y) == 0:
            return 0.0
        
        _, counts = np.unique(y, return_counts=True)
        probabilities = counts / len(y)
        entropy = -np.sum(probabilities * np.log2(probabilities + 1e-10))
        return entropy

    def calculate_split_quality(
        self, 
        y_left: np.ndarray, 
        y_right: np.ndarray
    ) -> float:
        """
        Вычисляет взвешенную энтропию после разбиения.

        Args:
            y_left (np.ndarray): Метки классов в левом дочернем узле.
            y_right (np.ndarray): Метки классов в правом дочернем узле.

        Returns:
            weighted_entropy (float): Взвешенная энтропия.
        """
        n_left = len(y_left)
        n_right = len(y_right)
        n_total = n_left + n_right
        
        if n_total == 0:
            return 0.0
        
        entropy_left = self.calculate(y_left)
        entropy_right = self.calculate(y_right)
        
        weighted_entropy = (n_left / n_total) * entropy_left + (n_right / n_total) * entropy_right
        return weighted_entropy
