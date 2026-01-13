import numpy as np
import abc as abc

from .utils.stoper import KMeansIterationStoper , KMeansIterationStoperItersCount
from .utils.center_destibuter import KMeansCenterDestributer, KMeansCenterDestributerEven
from .utils.history_writer import KMeansHistoryWriter, KMeansHistoryWriterIgnore

class KMeans(abc.ABC):
    """
    Абстрактный класс для алгоритма K-Means кластеризации.
    """
    
    @abc.abstractmethod
    def fit(self) -> None:
        """
        Обучает модель K-Means

        Args:
            None

        Returns:
            None

        """
        pass

    @abc.abstractmethod
    def predict(self, point: np.ndarray) -> int:
        """
        Предсказывает кластер для заданной точки
        
        Args:
            point (np.ndarray): точка для предсказания кластера

        Returns:
            cluster_index (int): индекс предсказанного кластера

        """
        pass

    @abc.abstractmethod
    def get_centers(self) -> np.ndarray:
        """
        Возвращает центры кластеров

        Args:
            None

        Returns:
            centers (np.ndarray): массив центров кластеров

        """
        pass

    @abc.abstractmethod
    def get_clusters(self) -> np.ndarray:
        """
        Возвращает массив кластеров для каждой точки

        Args:
            None

        Returns:
            clusters (np.ndarray): массив кластеров для каждой точки

        """
        pass


class KMeansImpl(KMeans):
    """
    Кастомный класс для алгоритма K-Means кластеризации.

    Args:
        X (np.ndarray): Входные данные для кластеризации.
        clusters_count (int): Количество кластеров. По умолчанию 3.
        max_iters (int): Максимальное количество итераций. По умолчанию 10^3.
        center_destributer (type[KMeansCenterDestributer]): Класс для распределения начальных центров кластеров.
        history_writer (type[KMeansHistoryWriter]): Класс для записи истории обучения K-Means. По умолчанию KMeansHistoryWriterIgnore.
    
    Returns:
        None
    
    """
    def __init__(
        self,
        X: np.ndarray,
        clusters_count: int = 3,
        stopper: KMeansIterationStoper =  KMeansIterationStoperItersCount(),
        center_destributer: type[KMeansCenterDestributer] = KMeansCenterDestributerEven(),
        history_writer : type[KMeansHistoryWriter] = KMeansHistoryWriterIgnore(),
    ) -> None:
        self.X = X
        self.clusters_count = clusters_count
        self.stopper = stopper
        self.center_destributer = center_destributer
        
        self.centers = center_destributer(
            X=self.X,
            centers_count=self.clusters_count
        ).destribute()

        self.history_writer = history_writer()
        
    
    def __equclide_distance(
            self, 
            a: np.ndarray, 
            b: np.ndarray
        ) -> float:
        """
        Вычисляет евклидово расстояние между двумя точками
        
        Args:
            a (np.ndarray): первая точка
            b (np.ndarray): вторая точка

        Returns:
            distance (float): евклидово расстояние между точками

        """
        return np.linalg.norm(a - b)
    
    def __predict_cluster(
            self, 
            point: np.ndarray,
        ) -> int:
        """
        Предсказывает кластер для заданной точки
        
        Args:
            point (np.ndarray): точка для предсказания кластера

        Returns:
            cluster_index (int): индекс предсказанного кластера

        """
        distances = [self.__equclide_distance(point, center) for center in self.centers]
        return int(np.argmin(distances))
    
    def __new_center(
            self,
            cluster : list[np.ndarray], 
        ) -> np.ndarray:
        """
        Вычисляет новый центр кластера как среднее значение точек в кластере

        Args:
            cluster (list[np.ndarray]): список точек в кластере

        Returns:
            new_center (np.ndarray): новый центр кластера
        """
        return np.mean(cluster, axis=0)

    def __single_iteration(self) -> tuple[np.ndarray, np.ndarray]:
        """
        Выполняет одну итерацию алгоритма K-Means
        
        Args:
            None

        Returns:
            сenters (np.ndarray): новый массив центров кластеров
            points_by_clusters (list[list[np.ndarray]]): список точек, распределенных по кластерам

        """

        points_by_clusters = [[] for _ in range(self.clusters_count)]

        for point in self.X:
            cluster_index = self.__predict_cluster(point)
            points_by_clusters[cluster_index].append(point)
        

        new_centers = np.array([
            self.__new_center(cluster) if len(cluster) > 0 else self.centers[i]
            for i, cluster in enumerate(points_by_clusters)
        ])

        return new_centers, points_by_clusters

    def fit(self) -> None:
        """
        Обучает модель K-Means


        Args:
            None


        Returns:X
            None

        """
        
        iter_number = 0

        while True:
            new_centers, points_by_clusters  = self.__single_iteration()

            points_by_clusters_indices = np.array([
                self.__predict_cluster(point) for point in self.X
            ])

            self.history_writer.write_iteration(
                centers=new_centers,
                clasters=points_by_clusters_indices,
                iteration=iter_number
            )



            if self.stopper.should_stop(
                iteration=iter_number,
                new_centers=new_centers,
                old_centers=self.centers,
                new_clusters=points_by_clusters_indices,
                old_clusters=None
            ):
                
                self.centers = new_centers
                break

            
            self.centers = new_centers    
            iter_number += 1

    


    def predict(self, point: np.ndarray) -> int:
        """
        Предсказывает кластер для заданной точки
        
        Args:
            point (np.ndarray): точка для предсказания кластера

        Returns:
            cluster_index (int): индекс предсказанного кластера

        """
        return self.__predict_cluster(point)
    

    def get_centers(self) -> np.ndarray:
        """
        Возвращает центры кластеров

        Args:
            None

        Returns:
            centers (np.ndarray): массив центров кластеров

        """
        return self.centers
    
    def get_clusters(self) -> np.ndarray:
        """
        Возвращает массив кластеров для каждой точки

        Args:
            None

        Returns:
            clusters (np.ndarray): массив кластеров для каждой точки

        """
        return np.array([
            self.__predict_cluster(point) for point in self.X
        ])