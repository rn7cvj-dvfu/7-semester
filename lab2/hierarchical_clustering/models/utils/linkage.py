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


class WardLinkage(LinkageMethod):
    """
    Ward linkage - метод Варда для минимизации суммы квадратов расстояний
    """
    
    def compute(
        self,
        cluster1_indices: np.ndarray,
        cluster2_indices: np.ndarray,
        distance_matrix: np.ndarray
    ) -> float:
        """
        Вычисляет расстояние по методу Варда

        Args:
            cluster1_indices (np.ndarray): индексы точек в первом кластере
            cluster2_indices (np.ndarray): индексы точек во втором кластере
            distance_matrix (np.ndarray): матрица расстояний между точками

        Returns:
            distance (float): расстояние по методу Варда
        """
        # Для упрощенной реализации используем среднее расстояние
        # В полной реализации Ward метод требует знания о центроидах кластеров
        distances = distance_matrix[np.ix_(cluster1_indices, cluster2_indices)]
        return float(np.mean(distances))


def get_linkage_method(method_name: str) -> LinkageMethod:
    """
    Возвращает экземпляр метода связи по его имени

    Args:
        method_name (str): имя метода ('single', 'complete', 'average', 'ward')

    Returns:
        LinkageMethod: экземпляр соответствующего метода связи
    
    Raises:
        ValueError: если указан неизвестный метод
    """
    methods = {
        'single': SingleLinkage(),
        'complete': CompleteLinkage(),
        'average': AverageLinkage(),
        'ward': WardLinkage()
    }
    
    if method_name not in methods:
        raise ValueError(f"Unknown linkage method: {method_name}. "
                        f"Available methods: {list(methods.keys())}")
    
    return methods[method_name]
