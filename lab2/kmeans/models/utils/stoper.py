
import numpy as np
import abc as abc


class KMeansIterationStoper(abc.ABC):
    """
    Абстрактный класс для остановки алгоритма K-Means кластеризации.

    Args:
        None

    Returns:
        None

    """


    @abc.abstractmethod
    def should_stop(self, 
                    iteration: int, 
                    new_centers: np.ndarray,
                    old_centers: np.ndarray,
                    new_clusters: np.ndarray,
                    old_clusters: np.ndarray
                ) -> bool:
        """
        Определяет, следует ли остановить обучение K-Means

        Args:
            iteration (int): Номер текущей итерации.
            centers (np.ndarray): Текущие центры кластеров.
            old_centers (np.ndarray): Центры кластеров с предыдущей итерации.

        Returns:
            stop (bool): True, если обучение следует остановить, иначе False.

        """
        pass


class KMeansIterationStoperItersCount(KMeansIterationStoper):
    """
    Класс для остановки обучения K-Means по количеству итераций.

    Args:
        max_iters (int): Максимальное количество итераций.

    Returns:
        None
    """

    def __init__(self, max_iters: int = 1_000) -> None:
        self.max_iters : int = max_iters


    def should_stop(self,
                    iteration: int, 
                    new_centers: np.ndarray,
                    old_centers: np.ndarray,
                    new_clusters: np.ndarray,
                    old_clusters: np.ndarray
                ) -> bool:
        return iteration >= self.max_iters
    
class KMeansIterationStoperCentersNoChange(KMeansIterationStoper):
    """
    Класс для остановки обучения K-Means при отсутствии изменений в центрах кластеров.

    Args:
        None
    Returns:
        None
    """

    def should_stop(self, 
                    iteration: int, 
                    new_centers: np.ndarray,
                    old_centers: np.ndarray,
                    new_clusters: np.ndarray,
                    old_clusters: np.ndarray
                ) -> bool:
        return np.array_equal(new_centers, old_centers)
