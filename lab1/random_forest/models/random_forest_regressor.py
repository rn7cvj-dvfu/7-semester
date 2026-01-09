import numpy as np
import abc as abc
from typing import Union

from decision_tree import DecisionTreeRegressor, MSESplitMetric, SplitMetric


class RandomForest(abc.ABC):
    """
    Абстрактный класс для случайного леса.
    """

    @abc.abstractmethod
    def fit(self, X: np.ndarray, y: np.ndarray) -> None:
        """
        Обучает случайный лес.

        Args:
            X (np.ndarray): Матрица признаков.
            y (np.ndarray): Целевая переменная.

        Returns:
            None
        """
        pass

    @abc.abstractmethod
    def predict(self, X: np.ndarray) -> np.ndarray:
        """
        Предсказывает значения для входных данных.

        Args:
            X (np.ndarray): Матрица признаков.

        Returns:
            predictions (np.ndarray): Предсказанные значения.
        """
        pass


class RandomForestRegressor(RandomForest):
    """
    Случайный лес для задачи регрессии.

    Args:
        n_estimators (int): Количество деревьев в лесу. По умолчанию 100.
        max_depth (int): Максимальная глубина каждого дерева. По умолчанию 5.
        min_samples_split (int): Минимальное количество образцов для разбиения узла. По умолчанию 2.
        min_samples_leaf (int): Минимальное количество образцов в листовом узле. По умолчанию 1.
        max_features (Union[int, str, None]): Максимальное количество признаков для рассмотрения при разбиении.
                                               'sqrt' - корень из числа признаков.
                                               'log2' - логарифм по основанию 2 от числа признаков.
                                               int - конкретное число признаков.
                                               None - использовать все признаки.
                                               По умолчанию 'sqrt'.
        bootstrap (bool): Использовать ли бутстрэп для обучения каждого дерева. По умолчанию True.
        split_metric (SplitMetric): Метрика для оценки качества разбиения. По умолчанию MSESplitMetric.
        random_state (int | None): Зерно генератора случайных чисел. По умолчанию None.
    """

    def __init__(
        self,
        n_estimators: int = 100,
        max_depth: int = 5,
        min_samples_split: int = 2,
        min_samples_leaf: int = 1,
        max_features: Union[int, str, None] = 'sqrt',
        bootstrap: bool = True,
        split_metric: SplitMetric = None,
        random_state: int = None
    ):
        self.n_estimators = n_estimators
        self.max_depth = max_depth
        self.min_samples_split = min_samples_split
        self.min_samples_leaf = min_samples_leaf
        self.max_features = max_features
        self.bootstrap = bootstrap
        self.split_metric = split_metric if split_metric is not None else MSESplitMetric()
        self.random_state = random_state
        self.trees = []
        self.n_features = None

    def _get_max_features(self) -> Union[int, None]:
        """
        Возвращает максимальное количество признаков для рассмотрения.

        Returns:
            max_features (int | None): Максимальное количество признаков.
        """
        if self.max_features == 'sqrt':
            return int(np.sqrt(self.n_features))
        elif self.max_features == 'log2':
            return int(np.log2(self.n_features))
        elif isinstance(self.max_features, int):
            return min(self.max_features, self.n_features)
        else:
            return None

    def fit(self, X: np.ndarray, y: np.ndarray) -> None:
        """
        Обучает случайный лес.

        Args:
            X (np.ndarray): Матрица признаков.
            y (np.ndarray): Целевая переменная.

        Returns:
            None
        """
        if self.random_state is not None:
            np.random.seed(self.random_state)

        self.n_features = X.shape[1]
        self.trees = []
        n_samples = X.shape[0]

        max_features = self._get_max_features()

        for _ in range(self.n_estimators):
            if self.bootstrap:
                indices = np.random.choice(n_samples, size=n_samples, replace=True)
                X_bootstrap = X[indices]
                y_bootstrap = y[indices]
            else:
                X_bootstrap = X
                y_bootstrap = y

            tree = DecisionTreeRegressor(
                max_depth=self.max_depth,
                min_samples_split=self.min_samples_split,
                min_samples_leaf=self.min_samples_leaf,
                max_features=max_features,
                split_metric=self.split_metric
            )
            tree.fit(X_bootstrap, y_bootstrap)
            self.trees.append(tree)

    def predict(self, X: np.ndarray) -> np.ndarray:
        """
        Предсказывает значения для входных данных путем усреднения предсказаний всех деревьев.

        Args:
            X (np.ndarray): Матрица признаков.

        Returns:
            predictions (np.ndarray): Предсказанные значения.

        Raises:
            ValueError: Если модель не обучена.
        """
        if not self.trees:
            raise ValueError("Model is not fitted yet.")


        predictions = np.array([tree.predict(X) for tree in self.trees])

        return np.mean(predictions, axis=0)
