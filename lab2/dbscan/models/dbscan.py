import numpy as np
from typing import Optional, List, Set
import abc as abc


class DistanceMetric(abc.ABC):
    """Базовый абстрактный класс для метрик расстояния"""
    
    @abc.abstractmethod
    def compute(self, x1: np.ndarray, x2: np.ndarray) -> float:
        """
        Вычисляет расстояние между двумя точками
        
        Args:
            x1 (np.ndarray): первая точка
            x2 (np.ndarray): вторая точка
        
        Returns:
            distance (float): расстояние между точками
        """
        pass


class EuclideanDistance(DistanceMetric):
    """Евклидово расстояние"""
    def compute(self, x1: np.ndarray, x2: np.ndarray) -> float:
        return np.sqrt(np.sum((x1 - x2) ** 2))


class DBSCAN:
    """
    DBSCAN (Density-Based Spatial Clustering of Applications with Noise)
    
    Алгоритм кластеризации на основе плотности, который группирует точки,
    находящиеся в плотных областях, и помечает точки в разреженных областях как шум.
    
    Args:
        X (np.ndarray): входные данные размера (n_samples, n_features)
        eps (float): радиус окрестности (epsilon) вокруг точки. По умолчанию 0.5.
        min_samples (int): минимальное количество точек в eps-окрестности 
                          для формирования ядра кластера. По умолчанию 5.
        distance_metric (DistanceMetric): метрика расстояния. По умолчанию EuclideanDistance.
    
    Attributes:
        labels_ (np.ndarray): метки кластеров для каждой точки (-1 для шума)
        core_sample_indices_ (np.ndarray): индексы ядерных точек
        n_clusters_ (int): количество найденных кластеров (не считая шум)
    """
    
    NOISE = -1
    UNCLASSIFIED = -2
    
    def __init__(
        self,
        X: np.ndarray,
        eps: float = 0.5,
        min_samples: int = 5,
        distance_metric: Optional[DistanceMetric] = None
    ):
        self.X = X
        self.eps = eps
        self.min_samples = min_samples
        self.distance_metric = distance_metric if distance_metric is not None else EuclideanDistance()
        
        # Результаты после fit
        self.labels_: Optional[np.ndarray] = None
        self.core_sample_indices_: Optional[np.ndarray] = None
        self.n_clusters_: int = 0
    
    def _get_neighbors(self, point_idx: int, distance_matrix: np.ndarray) -> np.ndarray:
        """
        Находит все точки в eps-окрестности данной точки
        
        Args:
            point_idx (int): индекс точки
            distance_matrix (np.ndarray): матрица расстояний
        
        Returns:
            neighbors (np.ndarray): индексы соседних точек
        """
        distances = distance_matrix[point_idx]
        neighbors = np.where(distances <= self.eps)[0]
        return neighbors
    
    def _expand_cluster(
        self,
        point_idx: int,
        neighbors: np.ndarray,
        cluster_id: int,
        labels: np.ndarray,
        distance_matrix: np.ndarray
    ) -> bool:
        """
        Расширяет кластер, начиная с ядерной точки
        
        Args:
            point_idx (int): индекс начальной точки
            neighbors (np.ndarray): соседи начальной точки
            cluster_id (int): ID текущего кластера
            labels (np.ndarray): массив меток
            distance_matrix (np.ndarray): матрица расстояний
        
        Returns:
            success (bool): True если кластер был успешно расширен
        """
        labels[point_idx] = cluster_id
        
        # Используем список для обхода соседей
        i = 0
        while i < len(neighbors):
            neighbor_idx = neighbors[i]
            
            # Если точка была шумом, добавляем её в кластер
            if labels[neighbor_idx] == self.NOISE:
                labels[neighbor_idx] = cluster_id
            
            # Если точка ещё не классифицирована
            elif labels[neighbor_idx] == self.UNCLASSIFIED:
                labels[neighbor_idx] = cluster_id
                
                # Проверяем, является ли эта точка ядерной
                neighbor_neighbors = self._get_neighbors(neighbor_idx, distance_matrix)
                
                if len(neighbor_neighbors) >= self.min_samples:
                    # Добавляем новых соседей в список для обработки
                    neighbors = np.concatenate([neighbors, neighbor_neighbors])
                    neighbors = np.unique(neighbors)
            
            i += 1
        
        return True
    
    def fit(self) -> 'DBSCAN':
        """
        Выполняет кластеризацию DBSCAN
        
        Returns:
            self: возвращает экземпляр класса
        """
        n_samples = self.X.shape[0]
        
        # Вычисляем матрицу расстояний более эффективно
        try:
            from scipy.spatial.distance import cdist
            distance_matrix = cdist(self.X, self.X, metric='euclidean')
        except ImportError:
            # Если scipy недоступна, используем numpy
            distance_matrix = np.zeros((n_samples, n_samples))
            for i in range(n_samples):
                for j in range(i + 1, n_samples):
                    dist = self.distance_metric.compute(self.X[i], self.X[j])
                    distance_matrix[i, j] = dist
                    distance_matrix[j, i] = dist
        
        # Инициализируем метки как неклассифицированные
        labels = np.full(n_samples, self.UNCLASSIFIED, dtype=int)
        core_samples = []
        
        cluster_id = 0
        
        # Проходим по всем точкам
        for point_idx in range(n_samples):
            # Пропускаем уже классифицированные точки
            if labels[point_idx] != self.UNCLASSIFIED:
                continue
            
            # Находим соседей
            neighbors = self._get_neighbors(point_idx, distance_matrix)
            
            # Если недостаточно соседей, помечаем как шум
            if len(neighbors) < self.min_samples:
                labels[point_idx] = self.NOISE
            else:
                # Это ядерная точка - создаём новый кластер
                core_samples.append(point_idx)
                self._expand_cluster(point_idx, neighbors, cluster_id, labels, distance_matrix)
                cluster_id += 1
        
        self.labels_ = labels
        self.core_sample_indices_ = np.array(core_samples)
        self.n_clusters_ = cluster_id
        
        return self
    
    def fit_predict(self) -> np.ndarray:
        """
        Выполняет кластеризацию и возвращает метки
        
        Returns:
            labels (np.ndarray): метки кластеров (-1 для шума)
        """
        self.fit()
        return self.labels_
    
    def get_labels(self) -> np.ndarray:
        """
        Возвращает метки кластеров
        
        Returns:
            labels (np.ndarray): метки кластеров для каждой точки
        """
        if self.labels_ is None:
            raise RuntimeError("Model must be fitted first")
        return self.labels_
    
    def get_core_samples(self) -> np.ndarray:
        """
        Возвращает индексы ядерных точек
        
        Returns:
            core_samples (np.ndarray): индексы ядерных точек
        """
        if self.core_sample_indices_ is None:
            raise RuntimeError("Model must be fitted first")
        return self.core_sample_indices_
    
    def get_n_clusters(self) -> int:
        """
        Возвращает количество найденных кластеров (без шума)
        
        Returns:
            n_clusters (int): количество кластеров
        """
        return self.n_clusters_
