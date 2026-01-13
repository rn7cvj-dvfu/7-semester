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
        X (np.ndarray): входные данные размера
        linkage_method (LinkageMethod): метод связи 
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

    def _find_closest_clusters(
        self, 
        current_clusters: list[int], 
        distance_matrix: np.ndarray
    ) -> tuple[tuple[int, int], float]:
        """
        Находит пару кластеров с минимальным расстоянием
        
        Args:
            current_clusters: список индексов текущих кластеров
            distance_matrix: матрица расстояний
            
        Returns:
            tuple: ((индекс_кластера_a, индекс_кластера_b), расстояние)
        """
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
        
        return to_merge, min_distance

    def _merge_clusters(
        self, 
        cluster_a: int, 
        cluster_b: int, 
        min_distance: float
    ) -> DendrogramNode:
        """
        Создает новый узел объединяющий два кластера
        
        Args:
            cluster_a: индекс первого кластера
            cluster_b: индекс второго кластера
            min_distance: расстояние между кластерами
            
        Returns:
            DendrogramNode: новый узел дендрограммы
        """
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
        return new_node

    def _compute_new_distances(
        self,
        cluster_a: int,
        cluster_b: int,
        current_clusters: list[int],
        distance_matrix: np.ndarray
    ) -> list[tuple[int, float]]:
        """
        Вычисляет расстояния от нового объединенного кластера до остальных
        
        Args:
            cluster_a: индекс первого объединенного кластера
            cluster_b: индекс второго объединенного кластера
            current_clusters: список текущих кластеров
            distance_matrix: матрица расстояний
            
        Returns:
            list: список пар (индекс_кластера, расстояние)
        """
        new_distances = []
        for k in current_clusters:
            if k != cluster_a and k != cluster_b:
                cluster_a_indices = self.nodes[cluster_a].get_all_indices()
                cluster_b_indices = self.nodes[cluster_b].get_all_indices()
                cluster_k_indices = self.nodes[k].get_all_indices()
                
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
                
                dist = (dist_a_k + dist_b_k) / 2.0
                new_distances.append((k, dist))
        
        return new_distances

    def _update_distance_matrix(
        self,
        distance_matrix: np.ndarray,
        new_cluster_index: int,
        new_distances: list[tuple[int, float]]
    ) -> np.ndarray:
        """
        Обновляет матрицу расстояний после объединения кластеров
        
        Args:
            distance_matrix: текущая матрица расстояний
            new_cluster_index: индекс нового кластера
            new_distances: расстояния от нового кластера до остальных
            
        Returns:
            np.ndarray: обновленная матрица расстояний
        """
        new_size = len(distance_matrix) + 1
        new_distance_matrix = np.zeros((new_size, new_size))
        new_distance_matrix[:-1, :-1] = distance_matrix

        for k, dist in new_distances:
            new_distance_matrix[new_cluster_index, k] = dist
            new_distance_matrix[k, new_cluster_index] = dist

        return new_distance_matrix
    
    def fit(self) -> None:
        """
        Построение дендрограммы с использованием агломеративной кластеризации
        """
        distance_matrix = compute_distance_matrix(self.X, self.distance_metric)
        current_clusters = list(range(self.n_samples))

        while len(current_clusters) > 1:
            # Находим ближайшие кластеры
            (cluster_a, cluster_b), min_distance = self._find_closest_clusters(
                current_clusters, 
                distance_matrix
            )

            # Объединяем кластеры
            new_node = self._merge_clusters(cluster_a, cluster_b, min_distance)

            # Вычисляем новые расстояния
            new_distances = self._compute_new_distances(
                cluster_a, 
                cluster_b, 
                current_clusters, 
                distance_matrix
            )

            # Обновляем список кластеров
            new_cluster_index = new_node.index
            current_clusters = [
                c for c in current_clusters if c != cluster_a and c != cluster_b
            ]
            current_clusters.append(new_cluster_index)

            # Обновляем матрицу расстояний
            distance_matrix = self._update_distance_matrix(
                distance_matrix, 
                new_cluster_index, 
                new_distances
            )
        
        self.dendrogram = self.nodes[-1]

    def _calculate_threshold(self, n_clusters: int, n_samples: int) -> float:
        """
        Вычисляет порог для разрезания дендрограммы
        
        Args:
            n_clusters: желаемое количество кластеров
            n_samples: количество образцов
            
        Returns:
            float: порог расстояния
        """
        merge_distances = []
        for node in self.nodes[n_samples:]:
            if node.left_child is not None and node.right_child is not None:
                merge_distances.append(node.distance)
        
        merge_distances.sort()
        threshold_idx = len(merge_distances) - n_clusters
        
        if threshold_idx < 0:
            return -np.inf
        elif threshold_idx >= len(merge_distances):
            return np.inf
        else:
            if threshold_idx + 1 < len(merge_distances):
                return (merge_distances[threshold_idx] + merge_distances[threshold_idx + 1]) / 2.0
            else:
                return merge_distances[threshold_idx] + 0.001

    def _assign_cluster(
        self, 
        node: DendrogramNode, 
        cluster_assignment: np.ndarray, 
        cluster_id: list[int]
    ) -> None:
        """
        Присваивает всем точкам в узле один кластер
        
        Args:
            node: узел дендрограммы
            cluster_assignment: массив для сохранения меток кластеров
            cluster_id: список с текущим ID кластера (мутируемый)
        """
        for idx in node.get_all_indices():
            cluster_assignment[idx] = cluster_id[0]
        cluster_id[0] += 1

    def _cut_tree(
        self,
        node: DendrogramNode,
        threshold: float,
        cluster_assignment: np.ndarray,
        cluster_id: list[int]
    ) -> None:
        """
        Рекурсивно разрезает дерево по порогу
        
        Args:
            node: текущий узел дендрограммы
            threshold: порог для разрезания
            cluster_assignment: массив для сохранения меток кластеров
            cluster_id: список с текущим ID кластера (мутируемый)
        """
        if node.left_child is None and node.right_child is None:
            self._assign_cluster(node, cluster_assignment, cluster_id)
            return
        
        if node.distance > threshold:
            if node.left_child is not None:
                self._cut_tree(node.left_child, threshold, cluster_assignment, cluster_id)
            if node.right_child is not None:
                self._cut_tree(node.right_child, threshold, cluster_assignment, cluster_id)
        else:
            self._assign_cluster(node, cluster_assignment, cluster_id)
    
    def predict(self, n_clusters: int) -> np.ndarray:
        if self.dendrogram is None:
            raise RuntimeError("Model must be fitted first")
        
        n_samples = self.X.shape[0]
        
        if n_clusters <= 1:
            return np.zeros(n_samples, dtype=int)
        
        if n_clusters >= n_samples:
            return np.arange(n_samples, dtype=int)
        
        threshold = self._calculate_threshold(n_clusters, n_samples)
        cluster_assignment = np.zeros(n_samples, dtype=int)
        cluster_id = [0]
        
        self._cut_tree(self.dendrogram, threshold, cluster_assignment, cluster_id)
        return cluster_assignment
    
    def get_dendrogram(self) -> DendrogramNode:
        if self.dendrogram is None:
            raise RuntimeError("Model must be fitted first")
        return self.dendrogram
