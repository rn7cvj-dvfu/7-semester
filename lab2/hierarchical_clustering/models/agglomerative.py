import numpy as np
from typing import Optional

from .base import HierarchicalClustering
from .utils.dendrogram import DendrogramNode
from .utils.linkage import LinkageMethod, AverageLinkage
from .utils.distance import DistanceMetric, EuclideanDistance, compute_distance_matrix


class AgglomerativeClustering(HierarchicalClustering):
    """
    Агломеративная иерархическая кластеризация (bottom-up подход)
    
    Args:
        X (np.ndarray): входные данные размера (n_samples, n_features)
        linkage_method (LinkageMethod): метод связи (экземпляр LinkageMethod)
        distance_metric (DistanceMetric): метрика расстояния
    """
    
    def __init__(
        self,
        X: np.ndarray,
        linkage_method: LinkageMethod = AverageLinkage(),
        distance_metric: DistanceMetric = EuclideanDistance()
    ):
        self.X = X
        self.linkage_method = linkage_method 
        self.distance_metric = distance_metric
        self.n_samples = X.shape[0]
        self.dendrogram: Optional[DendrogramNode] = None

        self.nodes: list[DendrogramNode] = [
            DendrogramNode(index=i) for i in range(self.n_samples)
        ]

       
    
    def fit(self) -> None:
        """
        Построение дендрограммы с использованием агломеративной кластеризации
        """
        distance_matrix = compute_distance_matrix(self.X, self.distance_metric)

        current_clusters = list(range(self.n_samples))

        while len(current_clusters) > 1:
            # Находим два ближайших кластера
            min_distance = np.inf
            to_merge = (0, 0)

            for i in range(len(current_clusters)):
                for j in range(i + 1, len(current_clusters)):
                    cluster_i = current_clusters[i]
                    cluster_j = current_clusters[j]
                    dist = distance_matrix[cluster_i, cluster_j]
                    if dist < min_distance:
                        min_distance = dist
                        to_merge = (cluster_i, cluster_j)

            cluster_a, cluster_b = to_merge

            # Создаем новый узел дендрограммы
            new_index = len(self.nodes)
            new_node = DendrogramNode(
                index=new_index,
                left_child=self.nodes[cluster_a],
                right_child=self.nodes[cluster_b],
                distance=min_distance,
                point_indices=np.concatenate([
                    self.nodes[cluster_a].get_all_indices(),
                    self.nodes[cluster_b].get_all_indices()
                ])
            )
            self.nodes.append(new_node)

            # Обновляем матрицу расстояний
            new_distances = []
            for k in current_clusters:
                if k != cluster_a and k != cluster_b:
                    cluster_a_indices = self.nodes[cluster_a].get_all_indices()
                    cluster_b_indices = self.nodes[cluster_b].get_all_indices()
                    cluster_k_indices = self.nodes[k].get_all_indices()
                    
                    # Вычисляем расстояние между объединенным кластером (a+b) и кластером k
                    dist_a_k = self.linkage_method.compute(
                        cluster_a_indices,
                        cluster_k_indices,
                        distance_matrix
                    )
                    dist_b_k = self.linkage_method.compute(
                        cluster_b_indices,
                        cluster_k_indices,
                        distance_matrix
                    )
                    
                    # Для average linkage берем среднее
                    dist = (dist_a_k + dist_b_k) / 2.0
                    new_distances.append((k, dist))

            new_cluster_index = new_index
            current_clusters = [
                c for c in current_clusters if c != cluster_a and c != cluster_b
            ]
            current_clusters.append(new_cluster_index)

            new_size = len(distance_matrix) + 1
            new_distance_matrix = np.zeros((new_size, new_size))
            new_distance_matrix[:-1, :-1] = distance_matrix

            for k, dist in new_distances:
                new_distance_matrix[new_cluster_index, k] = dist
                new_distance_matrix[k, new_cluster_index] = dist

            distance_matrix = new_distance_matrix
        
        self.dendrogram = self.nodes[-1]
        
       
    
    def predict(self, n_clusters: int) -> np.ndarray:
        if self.dendrogram is None:
            raise RuntimeError("Model must be fitted first")
        
        n_samples = self.X.shape[0]
        
        if n_clusters <= 1:
            return np.zeros(n_samples, dtype=int)
        
        if n_clusters >= n_samples:
            return np.arange(n_samples, dtype=int)
        
        # Собираем все расстояния слияния из дендрограммы
        merge_distances = []
        for node in self.nodes[n_samples:]:
            if node.left_child is not None and node.right_child is not None:
                merge_distances.append(node.distance)
        
        merge_distances.sort()
        # Для получения n_clusters нужно сделать n_clusters-1 разрезов
        # Разрезаем между (n_clusters-1)-м и n_clusters-м самым большим слиянием
        # Это означает, что threshold должен быть между merge_distances[-(n_clusters)] и merge_distances[-(n_clusters-1)]
        threshold_idx = len(merge_distances) - n_clusters
        if threshold_idx < 0:
            threshold = -np.inf  # Все в отдельные кластеры
        elif threshold_idx >= len(merge_distances):
            threshold = np.inf  # Все в один кластер
        else:
            # Берем среднее между двумя соседними расстояниями
            if threshold_idx + 1 < len(merge_distances):
                threshold = (merge_distances[threshold_idx] + merge_distances[threshold_idx + 1]) / 2.0
            else:
                threshold = merge_distances[threshold_idx] + 0.001
        
        cluster_assignment = np.zeros(n_samples, dtype=int)
        cluster_id = [0]
        
        def assign_cluster(node: DendrogramNode):
            """Присваивает всем точкам в узле один кластер"""
            for idx in node.get_all_indices():
                cluster_assignment[idx] = cluster_id[0]
            cluster_id[0] += 1
        
        def cut_tree(node: DendrogramNode):
            # Листовой узел - одна точка
            if node.left_child is None and node.right_child is None:
                assign_cluster(node)
                return
            
            # Если расстояние слияния больше порога, 
            # продолжаем разделять потомков
            if node.distance > threshold:
                if node.left_child is not None:
                    cut_tree(node.left_child)
                if node.right_child is not None:
                    cut_tree(node.right_child)
            else:
                # Если расстояние <= порога, все точки в этом узле 
                # образуют один кластер
                assign_cluster(node)
        
        cut_tree(self.dendrogram)
        return cluster_assignment
    
    def get_dendrogram(self) -> DendrogramNode:
        if self.dendrogram is None:
            raise RuntimeError("Model must be fitted first")
        return self.dendrogram
