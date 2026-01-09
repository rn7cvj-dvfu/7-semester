import numpy as np
import abc as abc
from typing import Union

from .decision_tree import DecisionTree, DecisionTreeRegressor
from ..metrics.tree_metrics import SplitMetric, MSESplitMetric


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
                                          Может быть int, 'sqrt', 'log2' или None. По умолчанию 'sqrt'.
        bootstrap (bool): Использовать ли бутстрэп при создании выборок. По умолчанию True.
        split_metric (SplitMetric): Метрика для оценки качества разбиения. По умолчанию MSESplitMetric.
        random_state (Union[int, None]): Seed для генератора случайных чисел. По умолчанию None.
    """

    def __init__(
        self,
        n_estimators: int = 100,
        max_depth: int = 5,
        min_samples_split: int = 2,
        min_samples_leaf: int = 1,
        max_features: Union[int, str] = 'sqrt',
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

        if random_state is not None:
            np.random.seed(random_state)

    def _get_max_features(self, n_features: int) -> int:
        """
        Определяет количество признаков для рассмотрения при разбиении.

        Args:
            n_features (int): Общее количество признаков.

        Returns:
            max_features (int): Количество признаков для использования.
        """
        if isinstance(self.max_features, int):
            return min(self.max_features, n_features)
        elif self.max_features == 'sqrt':
            return int(np.sqrt(n_features))
        elif self.max_features == 'log2':
            return int(np.log2(n_features))
        else:
            return n_features

    def _bootstrap_sample(self, X: np.ndarray, y: np.ndarray) -> tuple:
        """
        Создает бутстрэп-выборку из данных.

        Args:
            X (np.ndarray): Матрица признаков.
            y (np.ndarray): Целевая переменная.

        Returns:
            (X_sample, y_sample): Бутстрэп-выборка признаков и целевой переменной.
        """
        n_samples = X.shape[0]
        indices = np.random.choice(n_samples, size=n_samples, replace=True)
        return X[indices], y[indices]

    def fit(self, X: np.ndarray, y: np.ndarray) -> None:
        """
        Обучает случайный лес.

        Args:
            X (np.ndarray): Матрица признаков.
            y (np.ndarray): Целевая переменная.

        Returns:
            None
        """
        self.trees = []
        n_features = X.shape[1]
        max_features = self._get_max_features(n_features)

        for _ in range(self.n_estimators):
            # Создать дерево
            tree = DecisionTreeRegressor(
                max_depth=self.max_depth,
                min_samples_split=self.min_samples_split,
                min_samples_leaf=self.min_samples_leaf,
                max_features=max_features,
                split_metric=self.split_metric
            )

            # Создать бутстрэп-выборку (если требуется)
            if self.bootstrap:
                X_sample, y_sample = self._bootstrap_sample(X, y)
            else:
                X_sample, y_sample = X, y

            # Обучить дерево
            tree.fit(X_sample, y_sample)
            self.trees.append(tree)

    def predict(self, X: np.ndarray) -> np.ndarray:
        """
        Предсказывает значения для входных данных.

        Args:
            X (np.ndarray): Матрица признаков.

        Returns:
            predictions (np.ndarray): Предсказанные значения (среднее по всем деревьям).

        Raises:
            ValueError: Если модель не обучена.
        """
        if not self.trees:
            raise ValueError("Model is not fitted yet.")

        # Получить предсказания от каждого дерева
        tree_predictions = np.array([tree.predict(X) for tree in self.trees])

        # Усреднить предсказания
        predictions = np.mean(tree_predictions, axis=0)
        return predictions

    def get_feature_importance(self) -> np.ndarray:
        """
        Вычисляет важность признаков (упрощенная версия).
        
        Примечание: Это базовая реализация. Для полноценного расчета важности
        требуется отслеживание уменьшения ошибки при разбиении.

        Returns:
            feature_importance (np.ndarray): Массив важности признаков.
        """
        # Заглушка - требует более сложной реализации с отслеживанием разбиений
        return np.zeros(self.trees[0].n_features) if self.trees else np.array([])
