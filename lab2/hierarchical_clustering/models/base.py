import numpy as np
import abc

from .utils.dendrogram import DendrogramNode


class HierarchicalClustering(abc.ABC):
    """
    Абстрактный класс для иерархической кластеризации
    """
    
    @abc.abstractmethod
    def fit(self) -> None:
        """
        Обучает модель иерархической кластеризации

        Args:
            None

        Returns:
            None
        """
        pass
    
    @abc.abstractmethod
    def predict(self, n_clusters: int) -> np.ndarray:
        """
        Предсказывает кластеры путем разрезания дендрограммы на нужное количество кластеров

        Args:
            n_clusters (int): желаемое количество кластеров

        Returns:
            clusters (np.ndarray): массив кластеров для каждой точки
        """
        pass
    
    @abc.abstractmethod
    def get_dendrogram(self) -> DendrogramNode:
        """
        Возвращает дендрограмму

        Args:
            None

        Returns:
            dendrogram (DendrogramNode): корневой узел дендрограммы
        """
        pass
