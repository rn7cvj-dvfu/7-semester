import numpy as np
import abc as abc

class SilhouetteMetric(abc.ABC):

    """
    Абстрактный класс для метрики силуэта в K-Means кластеризации.
    """

    @abc.abstractmethod
    def compute(self, 
                X: np.ndarray, 
                centers: np.ndarray, 
                clasters: np.ndarray
        ) -> float:
        """
        Вычисляет значение метрики силуэта.

        Args:
                    X (np.ndarray): Входные данные.
                    centers (np.ndarray): Центроиды кластеров.
                    clasters (np.ndarray): Кластера

        Returns:
            silhouette (float): Значение метрики силуэта.

        """
        pass


class SilhouetteMetricImpl(SilhouetteMetric):

    def compute(self, 
                X: np.ndarray, 
                centers: np.ndarray, 
                clasters: np.ndarray
        ) -> float:
        """
        Вычисляет значение метрики силуэта.

        Args:
                    X (np.ndarray): Входные данные.
                    centers (np.ndarray): Центроиды кластеров.
                    clasters (np.ndarray): Кластера

        Returns:
            silhouette (float): Значение метрики силуэта.

        """
        n_samples = X.shape[0]
        silhouette_values = np.zeros(n_samples)

        for i in range(n_samples):
            own_cluster = clasters[i]
            own_center = centers[own_cluster]

            a = np.mean([
                np.linalg.norm(X[i] - X[j])
                for j in range(n_samples)
                if clasters[j] == own_cluster and i != j
            ]) if np.sum(clasters == own_cluster) > 1 else 0.0

            b = np.min([
                np.mean([
                    np.linalg.norm(X[i] - X[j])
                    for j in range(n_samples)
                    if clasters[j] == other_cluster
                ])
                for other_cluster in range(centers.shape[0])
                if other_cluster != own_cluster
            ])

            silhouette_values[i] = (b - a) / max(a, b) if max(a, b) > 0 else 0.0

        return np.mean(silhouette_values)