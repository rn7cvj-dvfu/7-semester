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
        linkage_method: Optional[LinkageMethod] = AverageLinkage(),
        distance_metric: Optional[DistanceMetric] = EuclideanDistance()
    ):
        self.X = X
        self.linkage_method = linkage_method 
        self.distance_metric = distance_metric 
        
        # Вычисляем матрицу расстояний
        self.distance_matrix = compute_distance_matrix(X, self.distance_metric)
        
        # Инициализируем кластеры - каждая точка это отдельный кластер
        self.n_samples = X.shape[0]
        self.clusters = {i: np.array([i]) for i in range(self.n_samples)}
        
        # История объединения кластеров для построения дендрограммы
        self.merge_history = []
        
        # Дендрограмма
        self.dendrogram = None
    
    def fit(self) -> None:
        """
        Выполняет агломеративную кластеризацию
        
        Args:
            None
        
        Returns:
            None
        """
        
        # Создаем узлы для каждой точки данных
        nodes = {i: DendrogramNode(i, point_indices=np.array([i])) 
                for i in range(self.n_samples)}
        
        current_clusters = self.clusters.copy()
        node_index = self.n_samples
        
        # Объединяем кластеры до тех пор, пока не останется один
        while len(current_clusters) > 1:
            # Находим два ближайших кластера
            min_distance = float('inf')
            merge_pair = None
            
            cluster_ids = list(current_clusters.keys())
            
            for i in range(len(cluster_ids)):
                for j in range(i + 1, len(cluster_ids)):
                    cluster_id1 = cluster_ids[i]
                    cluster_id2 = cluster_ids[j]
                    
                    indices1 = current_clusters[cluster_id1]
                    indices2 = current_clusters[cluster_id2]
                    
                    # Вычисляем расстояние между кластерами
                    distance = self.linkage_method.compute(
                        indices1,
                        indices2,
                        self.distance_matrix
                    )
                    
                    if distance < min_distance:
                        min_distance = distance
                        merge_pair = (cluster_id1, cluster_id2)
            
            if merge_pair is None:
                break
            
            # Объединяем два ближайших кластера
            id1, id2 = merge_pair
            
            # Создаем новый узел дендрограммы
            new_node = DendrogramNode(
                node_index,
                left_child=nodes[id1],
                right_child=nodes[id2],
                distance=min_distance,
                point_indices=np.concatenate([
                    current_clusters[id1],
                    current_clusters[id2]
                ])
            )
            
            # Сохраняем новый узел
            nodes[node_index] = new_node
            
            # Обновляем кластеры
            current_clusters[node_index] = new_node.point_indices
            del current_clusters[id1]
            del current_clusters[id2]
            
            # Сохраняем историю объединения
            self.merge_history.append({
                'cluster1': id1,
                'cluster2': id2,
                'distance': min_distance,
                'new_cluster': node_index
            })
            
            node_index += 1
        
        # Сохраняем корневой узел дендрограммы
        self.dendrogram = nodes[node_index - 1]
    
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
        
        # Обработка граничных случаев
        if n_clusters <= 1:
            return np.zeros(self.n_samples, dtype=int)
        
        if n_clusters >= self.n_samples:
            return np.arange(self.n_samples, dtype=int)
        
        # Находим пороговое расстояние
        if len(self.merge_history) < n_clusters - 1:
            n_clusters = len(self.merge_history) + 1
        
        # Сортируем историю объединений по расстоянию в порядке возрастания
        sorted_merges = sorted(self.merge_history, key=lambda x: x['distance'])
        
        # Пороговое расстояние - это расстояние между (n_clusters-1)-м и n_clusters-м объединениями
        # Мы останавливаемся до объединения на позиции n_clusters-1
        threshold = sorted_merges[len(sorted_merges) - n_clusters + 1]['distance']
        
        # Рекурсивно разрезаем дендрограмму
        cluster_assignment = np.zeros(self.n_samples, dtype=int)
        cluster_id = [0]  # Используем список для изменения в вложенной функции
        
        def cut_tree(node: DendrogramNode):
            # Если листовой узел или расстояние объединения <= порога, это кластер
            if node.left_child is None and node.right_child is None:
                # Листовой узел - это отдельная точка
                for idx in node.get_all_indices():
                    cluster_assignment[idx] = cluster_id[0]
                cluster_id[0] += 1
            elif node.distance <= threshold:
                # Расстояние объединения меньше или равно порогу - это кластер
                for idx in node.get_all_indices():
                    cluster_assignment[idx] = cluster_id[0]
                cluster_id[0] += 1
            else:
                # Продолжаем разрезание вниз по дереву
                if node.left_child is not None:
                    cut_tree(node.left_child)
                if node.right_child is not None:
                    cut_tree(node.right_child)
        
        cut_tree(self.dendrogram)
        
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
