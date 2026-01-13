import numpy as np
import abc


class LinkageMethod(abc.ABC):
    """
    Абстрактный класс для метода связи в иерархической кластеризации
    """
    
    @abc.abstractmethod
    def compute(
        self,
        cluster1_indices: np.ndarray,
        cluster2_indices: np.ndarray,
        distance_matrix: np.ndarray
    ) -> float:
        """
        Вычисляет расстояние между двумя кластерами

        Args:
            cluster1_indices (np.ndarray): индексы точек в первом кластере
            cluster2_indices (np.ndarray): индексы точек во втором кластере
            distance_matrix (np.ndarray): матрица расстояний между точками

        Returns:
            distance (float): расстояние между кластерами
        """
        pass


class SingleLinkage(LinkageMethod):
    """
    Single linkage - минимальное расстояние между точками кластеров
    """
    
    def compute(
        self,
        cluster1_indices: np.ndarray,
        cluster2_indices: np.ndarray,
        distance_matrix: np.ndarray
    ) -> float:
        """
        Вычисляет минимальное расстояние между кластерами

        Args:
            cluster1_indices (np.ndarray): индексы точек в первом кластере
            cluster2_indices (np.ndarray): индексы точек во втором кластере
            distance_matrix (np.ndarray): матрица расстояний между точками

        Returns:
            distance (float): минимальное расстояние
        """
        return float(np.min(distance_matrix[np.ix_(cluster1_indices, cluster2_indices)]))


class CompleteLinkage(LinkageMethod):
    """
    Complete linkage - максимальное расстояние между точками кластеров
    """
    
    def compute(
        self,
        cluster1_indices: np.ndarray,
        cluster2_indices: np.ndarray,
        distance_matrix: np.ndarray
    ) -> float:
        """
        Вычисляет максимальное расстояние между кластерами

        Args:
            cluster1_indices (np.ndarray): индексы точек в первом кластере
            cluster2_indices (np.ndarray): индексы точек во втором кластере
            distance_matrix (np.ndarray): матрица расстояний между точками

        Returns:
            distance (float): максимальное расстояние
        """
        return float(np.max(distance_matrix[np.ix_(cluster1_indices, cluster2_indices)]))


class AverageLinkage(LinkageMethod):
    """
    Average linkage - среднее расстояние между точками кластеров
    """
    
    def compute(
        self,
        cluster1_indices: np.ndarray,
        cluster2_indices: np.ndarray,
        distance_matrix: np.ndarray
    ) -> float:
        """
        Вычисляет среднее расстояние между кластерами

        Args:
            cluster1_indices (np.ndarray): индексы точек в первом кластере
            cluster2_indices (np.ndarray): индексы точек во втором кластере
            distance_matrix (np.ndarray): матрица расстояний между точками

        Returns:
            distance (float): среднее расстояние
        """
        distances = distance_matrix[np.ix_(cluster1_indices, cluster2_indices)]
        return float(np.mean(distances))

