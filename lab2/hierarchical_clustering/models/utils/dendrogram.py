import numpy as np
from typing import Optional


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
