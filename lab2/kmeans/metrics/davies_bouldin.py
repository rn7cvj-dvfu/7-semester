import numpy as np
import abc as abc


class DaviesBouldinMetric(abc.ABC):

    """
    Абстрактный класс для метрики Дэвиса-Боулдина в K-Means кластеризации.
    """

    @abc.abstractmethod
    def compute(self, 
                X: np.ndarray, 
                centers: np.ndarray, 
                clasters: np.ndarray
        ) -> float:
        """
        Вычисляет значение метрики Дэвиса-Боулдина.

        Args:
                    X (np.ndarray): Входные данные.
                    centers (np.ndarray): Центроиды кластеров.
                    clasters (np.ndarray): Кластера
        Returns:
            davies_bouldin (float): Значение метрики Дэвиса-Боулдина.
        """ 
        pass


class DaviesBouldinMetricImpl(DaviesBouldinMetric):

    def compute(self, 
                X: np.ndarray, 
                centers: np.ndarray, 
                clasters: np.ndarray
        ) -> float:
        """
        Вычисляет значение метрики Дэвиса-Боулдина.

        Args:
                    X (np.ndarray): Входные данные.
                    centers (np.ndarray): Центроиды кластеров.
                    clasters (np.ndarray): Кластера
        Returns:   
            davies_bouldin (float): Значение метрики Дэвиса-Боулдина.
        """ 
        n_clusters = centers.shape[0]
        cluster_scatters = np.zeros(n_clusters)
        for k in range(n_clusters):
            cluster_points = X[clasters == k]
            if cluster_points.shape[0] == 0:
                continue
            cluster_scatters[k] = np.mean([
                np.linalg.norm(point - centers[k])
                for point in cluster_points
            ])
        db_index = 0.0
        for i in range(n_clusters):
            max_ratio = 0.0
            for j in range(n_clusters):
                if i != j:
                    if np.linalg.norm(centers[i] - centers[j]) == 0:
                        continue
                    ratio = (cluster_scatters[i] + cluster_scatters[j]) / np.linalg.norm(centers[i] - centers[j])
                    max_ratio = max(max_ratio, ratio)
            db_index += max_ratio
        db_index /= n_clusters
        return db_index