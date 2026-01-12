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


class ManhattanDistance(DistanceMetric):
    """
    Манхэттенское расстояние между точками
    """
    
    def compute(self, point1: np.ndarray, point2: np.ndarray) -> float:
        """
        Вычисляет манхэттенское расстояние

        Args:
            point1 (np.ndarray): первая точка
            point2 (np.ndarray): вторая точка

        Returns:
            distance (float): манхэттенское расстояние
        """
        return float(np.sum(np.abs(point1 - point2)))


class CosineDistance(DistanceMetric):
    """
    Косинусное расстояние между точками
    """
    
    def compute(self, point1: np.ndarray, point2: np.ndarray) -> float:
        """
        Вычисляет косинусное расстояние

        Args:
            point1 (np.ndarray): первая точка
            point2 (np.ndarray): вторая точка

        Returns:
            distance (float): косинусное расстояние
        """
        dot_product = np.dot(point1, point2)
        norm1 = np.linalg.norm(point1)
        norm2 = np.linalg.norm(point2)
        
        if norm1 == 0 or norm2 == 0:
            return float('inf')
        
        cosine_similarity = dot_product / (norm1 * norm2)
        # Косинусное расстояние = 1 - косинусное подобие
        return float(1 - cosine_similarity)


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
