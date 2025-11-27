import numpy as np
import abc as abc

class KMeansCenterDestributer(abc.ABC):
    """ 
    Класс для начального распредлеения центров для алгоритма K-Means

    Agrs:
        X (np.ndarray): Входные данные.

    Returns:
        None

    Raises:
        AssertionError: Если centers_count <= 0 или centers_count > n_samples
        
    """
    def __init__(
            self,
            X : np.ndarray,
            centers_count : int = 3,
    ) -> None:
        
        assert centers_count > 0, "centers_count должен быть больше 0"
        assert X.shape[0] >= centers_count, "Количество центров не может быть больше количества образцов"

        self.X : np.ndarray = X
        self.centers_count : int = centers_count
 
 
    @abc.abstractmethod
    def destribute(self) -> np.ndarray:
        """
        Метод для распределения центров

        Args:
            None

        Returns:
            (np.ndarray): Массив размерностью (centers_count, n_features) с начальными центрами.
        """ 
        pass 

class KMeansCenterDestributerRandom(KMeansCenterDestributer):
    """
    Класс для случайного распределения центров
    """

   
    def destribute(self) -> np.ndarray:
        """
        Метод для случайного распределения центров

        Args:
            None

        Returns:
            сenters (np.ndarray): Массив размерностью (centers_count, n_features) с начальными центрами.
        """ 

        random_indexes : np.ndarray = np.random.choice(
            self.X.shape[0],
            size=self.centers_count,
            replace=False
        )

        random_indices : np.ndarray = self.X[random_indexes]
        return random_indices
    
class KMeansCenterDestributerEven(KMeansCenterDestributer):
    """
    Класс для равномерного распределения центров
    """


    def destribute(self) -> np.ndarray:
        """
        Метод для равномерного распределения центров
        
        Args:
            None

        Returns:
            сenters (np.ndarray): Массив размерностью (centers_count, n_features) с начальными центрами.
        """ 
        n_samples = self.X.shape[0]
        indices = np.linspace(0, n_samples - 1, self.centers_count, dtype=int)
        return self.X[indices]