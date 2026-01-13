import numpy as np
import abc


class DistanceMetric(abc.ABC):
    """
    Абстрактный класс для вычисления расстояния между точками
    """
    
    @abc.abstractmethod
    def compute(self, point1: np.ndarray, point2: np.ndarray) -> float:
        """
        Вычисляет расстояние между двумя точками

        Args:
            point1 (np.ndarray): первая точка
            point2 (np.ndarray): вторая точка

        Returns:
            distance (float): расстояние между точками
        """
        pass


class EuclideanDistance(DistanceMetric):
    """
    Евклидово расстояние между точками
    """
    
    def compute(self, point1: np.ndarray, point2: np.ndarray) -> float:
        """
        Вычисляет евклидово расстояние

        Args:
            point1 (np.ndarray): первая точка
            point2 (np.ndarray): вторая точка

        Returns:
            distance (float): евклидово расстояние
        """
        return float(np.linalg.norm(point1 - point2))




def compute_distance_matrix(
    X: np.ndarray,
    metric: DistanceMetric
) -> np.ndarray:
    """
    Вычисляет матрицу расстояний между всеми точками

    Args:
        X (np.ndarray): входные данные размера (n_samples, n_features)
        metric (DistanceMetric): метрика расстояния

    Returns:
        distance_matrix (np.ndarray): матрица расстояний размера (n_samples, n_samples)
    """
    n_samples = X.shape[0]
    distance_matrix = np.zeros((n_samples, n_samples))
    
    for i in range(n_samples):
        for j in range(i + 1, n_samples):
            distance = metric.compute(X[i], X[j])
            distance_matrix[i][j] = distance
            distance_matrix[j][i] = distance
    
    return distance_matrix
