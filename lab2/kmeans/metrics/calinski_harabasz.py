import numpy as np
import abc as abc

class CalinskiHarabaszMetric(abc.ABC):
    """
    Абстрактный класс для метрики Калински-Харабаша в K-Means кластеризации.
    """

    @abc.abstractmethod
    def compute(self, 
                X: np.ndarray, 
                centers: np.ndarray, 
                clasters: np.ndarray
        ) -> float:
        """
        Вычисляет значение метрики Калински-Харабаша.

        Args:
                    X (np.ndarray): Входные данные.
                    centers (np.ndarray): Центроиды кластеров.
                    clasters (np.ndarray): Кластера
        Returns:
            calinski_harabasz (float): Значение метрики Калински-Харабаша.
        """ 
        pass

class CalinskiHarabaszMetricImpl(CalinskiHarabaszMetric):

    def compute(self, 
                X: np.ndarray, 
                centers: np.ndarray, 
                clasters: np.ndarray
        ) -> float:
        """
        Вычисляет значение метрики Калински-Харабаша.

        Args:
                    X (np.ndarray): Входные данные.
                    centers (np.ndarray): Центроиды кластеров.
                    clasters (np.ndarray): Кластера
        Returns:
            calinski_harabasz (float): Значение метрики Калински-Харабаша.
        """
        n_samples, n_features = X.shape
        n_clusters = centers.shape[0]
        overall_mean = np.mean(X, axis=0)
        between_cluster_dispersion = 0.0
        within_cluster_dispersion = 0.0
        
        for k in range(n_clusters):
            cluster_points = X[clasters == k]
            n_k = cluster_points.shape[0]
            if n_k == 0:
                continue
            cluster_mean = centers[k]
            between_cluster_dispersion += n_k * np.sum((cluster_mean - overall_mean) ** 2)
            within_cluster_dispersion += np.sum((cluster_points - cluster_mean) ** 2)
        
        if within_cluster_dispersion == 0:
            return 0.0
        
        calinski_harabasz = (between_cluster_dispersion / within_cluster_dispersion) * ((n_samples - n_clusters) / (n_clusters - 1))
        return calinski_harabasz