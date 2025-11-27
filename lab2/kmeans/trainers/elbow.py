import numpy as np
import abc as abc

from ..metrics.inertia import InertiaMetric, InertiaMetricImpl

from ..models.kmeans import KMeans, KMeansImpl

from ..models.utils.stoper import KMeansIterationStoper , KMeansIterationStoperItersCount
from ..models.utils.center_destibuter import KMeansCenterDestributer, KMeansCenterDestributerEven
from ..models.utils.history_writer import KMeansHistoryWriter, KMeansHistoryWriterIgnore


class ElbowTrainer(abc.ABC):
    """
    Абстрактный класс для тренировки K-Means с использованием метода локтя.
    """

    @abc.abstractmethod
    def train(self) -> list[dict]:
        """
        Тренирует K-Means для различных значений k и вычисляет инерцию для каждого k.

        Args:
            k_values (list[int]): Список значений количества кластеров для тренировки.
        Returns:
            results (list[dict]): Список словарей с результатами тренировки для каждого k.
        """
        pass



class ElbowTrainerImpl(ElbowTrainer):
    """
    Кастомный класс для тренировки K-Means с использованием метода локтя.

    Args:
        X (np.ndarray): Входные данные для кластеризации.
        stopper (KMeansIterationStoper): Стопер для остановки итераций K-Means.
        center_destributer (type[KMeansCenterDestributer]): Класс для распределения начальных центров кластеров.
        history_writer (type[KMeansHistoryWriter]): Класс для записи истории обучения K-Means.

    Returns:
        None

    """
    def __init__(
            self,
            X : np.ndarray,
            ks : range = range(1 , 10),
            km_stopper: KMeansIterationStoper =  KMeansIterationStoperItersCount(),
            km_center_destributer: type[KMeansCenterDestributer] = KMeansCenterDestributerEven,
            km_history_writer : type[KMeansHistoryWriter] = KMeansHistoryWriterIgnore,  
    ) -> None:
        
        self.X = X
        self.km_stopper = km_stopper
        self.km_center_destributer = km_center_destributer
        self.km_history_writer = km_history_writer
        self.ks = ks

    def train(self) -> list[dict]:

        metric_calculator: InertiaMetric = InertiaMetricImpl()

        result = []

        for k in self.ks:
            kmeans = KMeansImpl(
                X=self.X,
                clusters_count=k,
                center_destributer=self.km_center_destributer,
                history_writer=self.km_history_writer,
                stopper=self.km_stopper
            )
            kmeans.fit()
            inertia = metric_calculator.compute(
                X=self.X,
                centers=kmeans.get_centers(),
                clusters=kmeans.get_clusters()
            )

            result.append({
                "k": k,
                "inertia": inertia,
                "kmeans": kmeans
            })
        
        return result