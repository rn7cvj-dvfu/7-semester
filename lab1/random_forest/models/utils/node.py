import numpy as np
from typing import Optional


class TreeNode:
    """
    Узел дерева решений.
    
    Args:
        feature_index (Optional[int]): Индекс признака для разбиения (None для листового узла).
        threshold (Optional[float]): Пороговое значение для разбиения (None для листового узла).
        left (Optional[TreeNode]): Левый дочерний узел.
        right (Optional[TreeNode]): Правый дочерний узел.
        value (Optional[float]): Значение предсказания для листового узла.
    """
    
    def __init__(
        self,
        feature_index: int = None,
        threshold: float = None,
        left: 'TreeNode' = None,
        right: 'TreeNode' = None,
        value: float = None
    ):
        self.feature_index = feature_index
        self.threshold = threshold
        self.left = left
        self.right = right
        self.value = value
    
    def is_leaf(self) -> bool:
        """
        Проверяет, является ли узел листовым.
        
        Returns:
            bool: True, если узел является листом, иначе False.
        """
        return self.value is not None
