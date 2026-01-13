import numpy as np
from typing import Optional

from .base import HierarchicalClustering
from .utils.dendrogram import DendrogramNode
from .utils.linkage import LinkageMethod, AverageLinkage
from .utils.distance import DistanceMetric, EuclideanDistance, compute_distance_matrix


class DivisiveClustering(HierarchicalClustering):
    """
    Дивизивная иерархическая кластеризация (top-down подход)
    
    Args:
        X (np.ndarray): входные данные размера (n_samples, n_features)
        linkage_method (LinkageMethod): метод связи (экземпляр LinkageMethod)
        distance_metric (DistanceMetric): метрика расстояния
    """
    
    def __init__(
        self,
        X: np.ndarray,
        linkage_method: Optional[LinkageMethod] = AverageLinkage(),
        distance_metric: Optional[DistanceMetric] = EuclideanDistance()
    ):
        self.X = X
        self.linkage_method = linkage_method
        self.distance_metric = distance_metric
        
        self.distance_matrix = compute_distance_matrix(X, self.distance_metric)
        
        self.n_samples = X.shape[0]
        

        self.split_history = []
        
        self.dendrogram = None
    
    def _find_farthest_pair(self, indices: np.ndarray) -> tuple:
        """
        Находит две точки с максимальным расстоянием в кластере
        
        Args:
            indices (np.ndarray): индексы точек в кластере
        
        Returns:
            (idx1, idx2): индексы двух наиболее удаленных точек
        """
        max_distance = -1
        farthest_pair = None
        
        for i in range(len(indices)):
            for j in range(i + 1, len(indices)):
                distance = self.distance_matrix[indices[i]][indices[j]]
                if distance > max_distance:
                    max_distance = distance
                    farthest_pair = (indices[i], indices[j])
        
        return farthest_pair if farthest_pair is not None else (indices[0], indices[1])
    
    def _split_cluster(self, indices: np.ndarray) -> tuple:
        """
        Разбивает кластер на два подкластера
        
        Args:
            indices (np.ndarray): индексы точек в кластере
        
        Returns:
            (cluster1, cluster2): два подкластера
        """
        if len(indices) <= 1:
            return indices, np.array([])
        
        # Находим две наиболее удаленные точки
        point1_idx, point2_idx = self._find_farthest_pair(indices)
        
        cluster1 = [point1_idx]
        cluster2 = [point2_idx]
        
        # Распределяем остальные точки между двумя кластерами
        for idx in indices:
            if idx != point1_idx and idx != point2_idx:
                dist_to_1 = self.distance_matrix[idx][point1_idx]
                dist_to_2 = self.distance_matrix[idx][point2_idx]
                
                if dist_to_1 < dist_to_2:
                    cluster1.append(idx)
                else:
                    cluster2.append(idx)
        
        return np.array(cluster1), np.array(cluster2)
    
    def fit(self) -> None:
        """
        Выполняет дивизивную кластеризацию
        
        Args:
            None
        
        Returns:
            None
        """
        
        # Создаем корневой узел с всеми точками
        all_indices = np.arange(self.n_samples)
        root_node = DendrogramNode(0, point_indices=all_indices)
        
        # Используем очередь для обхода в ширину
        node_queue = [root_node]
        node_index = 1
        
        while len(node_queue) > 0:
            current_node = node_queue.pop(0)
            indices = current_node.point_indices
            
            # Если в кластере только одна точка, не разбиваем
            if len(indices) <= 1:
                continue
            
            # Разбиваем кластер на два
            cluster1, cluster2 = self._split_cluster(indices)
            
            if len(cluster2) == 0:
                continue
            
            # Вычисляем расстояние между кластерами
            distance = self.linkage_method.compute(
                cluster1,
                cluster2,
                self.distance_matrix
            )
            
            # Создаем узлы для подкластеров
            left_child = DendrogramNode(
                node_index,
                point_indices=cluster1
            )
            node_index += 1
            
            right_child = DendrogramNode(
                node_index,
                point_indices=cluster2
            )
            node_index += 1
            
            # Обновляем текущий узел
            current_node.left_child = left_child
            current_node.right_child = right_child
            current_node.distance = distance
            
            # Сохраняем историю разбиений
            self.split_history.append({
                'parent': current_node.index,
                'cluster1': left_child.point_indices,
                'cluster2': right_child.point_indices,
                'distance': distance
            })
            
            # Добавляем подкластеры в очередь
            node_queue.append(left_child)
            node_queue.append(right_child)
        
        self.dendrogram = root_node
    
    def predict(self, n_clusters: int) -> np.ndarray:
        """
        Разрезает дендрограмму для получения n кластеров
        
        Args:
            n_clusters (int): желаемое количество кластеров
        
        Returns:
            clusters (np.ndarray): массив кластеров для каждой точки
        """
        if self.dendrogram is None:
            raise RuntimeError("Model must be fitted first")
        
        if n_clusters <= 1:
            return np.zeros(self.n_samples, dtype=int)
        
        if n_clusters >= self.n_samples:
            return np.arange(self.n_samples, dtype=int)
        
        # Собираем все расстояния разбиения
        split_distances = []
        for split in self.split_history:
            split_distances.append(split['distance'])
        
        # Сортируем в порядке убывания (самые большие разбиения первыми)
        split_distances.sort(reverse=True)
        
        # Выбираем порог: берем среднее между (n_clusters-1)-м и n_clusters-м разбиением
        if n_clusters - 1 < len(split_distances):
            if n_clusters < len(split_distances):
                threshold = (split_distances[n_clusters - 1] + split_distances[n_clusters]) / 2.0
            else:
                threshold = split_distances[n_clusters - 1] - 0.001
        else:
            threshold = -np.inf
        
        # Рекурсивно проходим по дереву и собираем кластеры
        cluster_assignment = np.zeros(self.n_samples, dtype=int)
        cluster_id = [0]
        
        def collect_clusters(node: DendrogramNode):
            # Если это листовой узел (нет потомков)
            if node.left_child is None or node.right_child is None:
                for idx in node.get_all_indices():
                    cluster_assignment[idx] = cluster_id[0]
                cluster_id[0] += 1
                return
            
            # Если расстояние разбиения меньше порога, останавливаемся
            # (т.е. не продолжаем разбивать этот кластер)
            if node.distance < threshold:
                for idx in node.get_all_indices():
                    cluster_assignment[idx] = cluster_id[0]
                cluster_id[0] += 1
            else:
                # Продолжаем разбиение
                collect_clusters(node.left_child)
                collect_clusters(node.right_child)
        
        collect_clusters(self.dendrogram)
        
        return cluster_assignment
    
    def get_dendrogram(self) -> DendrogramNode:
        """
        Возвращает дендрограмму
        
        Args:
            None
        
        Returns:
            dendrogram (DendrogramNode): корневой узел дендрограммы
        """
        if self.dendrogram is None:
            raise RuntimeError("Model must be fitted first")
        
        return self.dendrogram
