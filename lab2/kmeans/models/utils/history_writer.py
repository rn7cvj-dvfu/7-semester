import numpy as np
import abc as abc

class KMeansHistoryWriter(abc.ABC):
    """
    Абстрактный класс для записи истории обучения K-Means

    Args:
        None

    Returns:
        None

    """
    @abc.abstractmethod
    def write_iteration(
        self,
        centers: np.ndarray,
        clasters: np.ndarray,
        iteration: int
    ) -> None:
        """
        Метод для записи одной итерации обучения K-Means

        Args:
            centers (np.ndarray): Массив центров кластеров.
            clasters (np.ndarray): Массив объектов кластеров для каждой точки данных.
            iteration (int): Номер текущей итерации.

        Returns:
            None

        """
        pass

    @abc.abstractmethod
    def finalize(self) -> None:
        """
        Метод для финализации записи истории обучения K-Means

        Args:
            None

        Returns:
            None

        """
        pass


class KMeansHistoryWriterIgnore(KMeansHistoryWriter):
    """
    Класс-заглушка для игнорирования записи истории обучения K-Means

    Args:
        None

    Returns:
        None

    """

    def write_iteration(
        self,
        centers: np.ndarray,
        clasters: np.ndarray,
        iteration: int
    ) -> None:
        """
        Метод-заглушка для игнорирования записи одной итерации обучения K-Means

        Args:
            centers (np.ndarray): Массив центров кластеров.
            clasters (np.ndarray): Массив объектов кластеров для каждой точки данных.
            iteration (int): Номер текущей итерации.

        Returns:
            None

        """
        pass

    def finalize(self) -> None:
        """
        Метод-заглушка для игнорирования финализации записи истории обучения K-Means

        Args:
            None

        Returns:
            None

        """
        pass

class KMeansHistoryWriterFile(KMeansHistoryWriter):
    """
    Класс для записи истории обучения K-Means в json файл

    Args:
        file_path (str): Путь к json файлу для записи истории.
        save_on_iteration (int | None): Интервал итераций для записи истории. Если None, то запись происходит в конце.

    Returns:
        None

    """
    def __init__(self, 
                 file_path: str,
                 save_on_iteration: int | None = None
                ) -> None:
        
        assert file_path.endswith('.json'), "file_path должен оканчиваться на .json"

        self.history : list[dict] = []
        self.file_path : str = file_path
        self.save_on_iteration : int | None = save_on_iteration
        

    

    def __save(self) -> None:
        """
        Внутренний метод для сохранения истории в файл
        Обнуляет историю после сохранения

        Args:
            None

        Returns:
            None

        """ 
        import json

        with open(self.file_path, 'w') as f:
            json.dump(self.history, f, indent=4)

        self.history = []

    def write_iteration(
        self,
        centers: np.ndarray,
        clasters: np.ndarray,
        iteration: int
    ) -> None:
        """
        Метод для записи одной итерации обучения K-Means в файл

        Args:
            centers (np.ndarray): Массив центров кластеров.
            clasters (np.ndarray): Массив объектов кластеров для каждой точки данных.
            iteration (int): Номер текущей итерации.

        Returns:
            None

        """

        self.history.append({
            'iteration': iteration,
            'centers': centers.tolist(),
            'clasters': clasters.tolist()
        })

        if self.save_on_iteration is  None:
            return
        
        if iteration % self.save_on_iteration == 0:
            self.__save()




    
    def finalize(self) -> None:
        """
        Метод для финализации записи истории обучения K-Means в файл

        Args:
            None

        Returns:
            None

        """ 
        self.__save()
