import numpy as np
from typing import Optional


class Linkage:
    """
    Абстрактный класс для метода связи в иерархической кластеризации
    """
    
    @staticmethod
    def compute_linkage_distance(
        cluster1_indices: np.ndarray,
        cluster2_indices: np.ndarray,
        distance_matrix: np.ndarray,
        linkage_method: str = 'single'
    ) -> float:
        """
        Вычисляет расстояние между двумя кластерами на основе выбранного метода связи

        Args:
            cluster1_indices (np.ndarray): индексы точек в первом кластере
            cluster2_indices (np.ndarray): индексы точек во втором кластере
            distance_matrix (np.ndarray): матрица расстояний между точками
            linkage_method (str): метод связи ('single', 'complete', 'average', 'ward')

        Returns:
            distance (float): расстояние между кластерами
        """
        
        if linkage_method == 'single':
            # Single linkage - минимальное расстояние
            return np.min(distance_matrix[np.ix_(cluster1_indices, cluster2_indices)])
        
        elif linkage_method == 'complete':
            # Complete linkage - максимальное расстояние
            return np.max(distance_matrix[np.ix_(cluster1_indices, cluster2_indices)])
        
        elif linkage_method == 'average':
            # Average linkage - среднее расстояние
            distances = distance_matrix[np.ix_(cluster1_indices, cluster2_indices)]
            return np.mean(distances)
        
        elif linkage_method == 'ward':
            # Ward linkage - минимизация суммы квадратов
            return np.mean(distance_matrix[np.ix_(cluster1_indices, cluster2_indices)])
        
        else:
            raise ValueError(f"Unknown linkage method: {linkage_method}")


class DendrogramNode:
    """
    Класс для представления узла дендрограммы

    Args:
        index (int): уникальный индекс узла
        left_child (Optional[DendrogramNode]): левый потомок
        right_child (Optional[DendrogramNode]): правый потомок
        distance (float): расстояние на котором произошло объединение
        point_indices (np.ndarray): индексы точек данных в этом поддереве
    """
    
    def __init__(
        self,
        index: int,
        left_child: Optional['DendrogramNode'] = None,
        right_child: Optional['DendrogramNode'] = None,
        distance: float = 0.0,
        point_indices: Optional[np.ndarray] = None
    ):
        self.index = index
        self.left_child = left_child
        self.right_child = right_child
        self.distance = distance
        self.point_indices = point_indices if point_indices is not None else np.array([index])
    
    def get_all_indices(self) -> np.ndarray:
        """
        Возвращает все индексы точек в поддереве
        
        Returns:
            indices (np.ndarray): индексы всех точек
        """
        if self.left_child is None and self.right_child is None:
            return self.point_indices
        
        indices = []
        if self.left_child is not None:
            indices.extend(self.left_child.get_all_indices())
        if self.right_child is not None:
            indices.extend(self.right_child.get_all_indices())
        
        return np.array(indices)
