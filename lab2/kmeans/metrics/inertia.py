import numpy as np
import abc as abc


class InertiaMetric(abc.ABC):
    """
    Абстрактный класс для метрики инерции в K-Means кластеризации.
    """

    @abc.abstractmethod
    def compute(self, 
                X: np.ndarray, 
                centers: np.ndarray, 
                clusters: np.ndarray
        ) -> float:
        """
        Вычисляет значение метрики инерции.

        Args:
                    X (np.ndarray): Входные данные.
                    centers (np.ndarray): Центроиды кластеров.
                    clusters (np.ndarray): Кластера

        Returns:
            inertia (float): Значение метрики инерции.

        """
        pass

class InertiaMetricImpl(InertiaMetric):

    def compute(self, 
                X: np.ndarray, 
                centers: np.ndarray, 
                clusters: np.ndarray
        ) -> float:
        """
        Вычисляет значение метрики инерции.

        Args:
                    X (np.ndarray): Входные данные.
                    centers (np.ndarray): Центроиды кластеров.
                    clusters (np.ndarray): Кластера

        Returns:
            inertia (float): Значение метрики инерции.

        """
        inertia = 0.0
        for i in range(X.shape[0]):
            center = centers[clusters[i]]
            inertia += np.sum((X[i] - center) ** 2)
        return inertia