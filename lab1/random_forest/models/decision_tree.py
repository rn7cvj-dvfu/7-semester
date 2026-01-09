import numpy as np
import abc as abc
from typing import Union

from ..metrics.tree_metrics import SplitMetric, MSESplitMetric
from .utils.node import TreeNode


class DecisionTree(abc.ABC):
    """
    Абстрактный класс для дерева решений.
    """

    @abc.abstractmethod
    def fit(self, X: np.ndarray, y: np.ndarray) -> None:
        """
        Обучает дерево решений.

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


class DecisionTreeRegressor(DecisionTree):
    """
    Дерево решений для задачи регрессии.

    Args:
        max_depth (int): Максимальная глубина дерева. По умолчанию 5.
        min_samples_split (int): Минимальное количество образцов для разбиения узла. По умолчанию 2.
        min_samples_leaf (int): Минимальное количество образцов в листовом узле. По умолчанию 1.
        max_features (Union[int, None]): Максимальное количество признаков для рассмотрения при разбиении.
                                    None означает использовать все признаки. По умолчанию None.
        split_metric (SplitMetric): Метрика для оценки качества разбиения. По умолчанию MSESplitMetric.
    """

    def __init__(
        self,
        max_depth: int = 5,
        min_samples_split: int = 2,
        min_samples_leaf: int = 1,
        max_features: int = None,
        split_metric: SplitMetric = None
    ):
        self.max_depth = max_depth
        self.min_samples_split = min_samples_split
        self.min_samples_leaf = min_samples_leaf
        self.max_features = max_features
        self.split_metric = split_metric if split_metric is not None else MSESplitMetric()
        self.root = None

    def fit(self, X: np.ndarray, y: np.ndarray) -> None:
        """
        Обучает дерево решений.

        Args:
            X (np.ndarray): Матрица признаков.
            y (np.ndarray): Целевая переменная.

        Returns:
            None
        """
        self.n_features = X.shape[1]
        self.root = self._build_tree(X, y, depth=0)

    def _build_tree(self, X: np.ndarray, y: np.ndarray, depth: int) -> TreeNode:
        """
        Рекурсивно строит дерево решений.

        Args:
            X (np.ndarray): Матрица признаков.
            y (np.ndarray): Целевая переменная.
            depth (int): Текущая глубина дерева.

        Returns:
            node (TreeNode): Корень построенного поддерева.
        """
        n_samples = X.shape[0]

        # Критерии остановки
        if (depth >= self.max_depth or 
            n_samples < self.min_samples_split or 
            len(np.unique(y)) == 1):
            leaf_value = np.mean(y)
            return TreeNode(value=leaf_value)

        # Найти лучшее разбиение
        best_feature, best_threshold = self._find_best_split(X, y)

        if best_feature is None:
            leaf_value = np.mean(y)
            return TreeNode(value=leaf_value)

        # Разбить данные
        left_indices = X[:, best_feature] <= best_threshold
        right_indices = ~left_indices

        # Проверить минимальное количество образцов в листьях
        if (np.sum(left_indices) < self.min_samples_leaf or 
            np.sum(right_indices) < self.min_samples_leaf):
            leaf_value = np.mean(y)
            return TreeNode(value=leaf_value)

        # Рекурсивно построить дочерние узлы
        left_child = self._build_tree(X[left_indices], y[left_indices], depth + 1)
        right_child = self._build_tree(X[right_indices], y[right_indices], depth + 1)

        return TreeNode(
            feature_index=best_feature,
            threshold=best_threshold,
            left=left_child,
            right=right_child
        )

    def _find_best_split(self, X: np.ndarray, y: np.ndarray) -> tuple:
        """
        Находит лучшее разбиение для узла.

        Args:
            X (np.ndarray): Матрица признаков.
            y (np.ndarray): Целевая переменная.

        Returns:
            (feature_index, threshold): Индекс признака и пороговое значение для лучшего разбиения.
                                       Возвращает (None, None), если разбиение не найдено.
        """
        best_quality = float('inf')
        best_feature = None
        best_threshold = None

        # Определить признаки для рассмотрения
        if self.max_features is None:
            features_to_consider = range(self.n_features)
        else:
            features_to_consider = np.random.choice(
                self.n_features,
                size=min(self.max_features, self.n_features),
                replace=False
            )

        # Перебрать признаки и пороговые значения
        for feature_index in features_to_consider:
            feature_values = X[:, feature_index]
            thresholds = np.unique(feature_values)

            for threshold in thresholds:
                left_indices = feature_values <= threshold
                right_indices = ~left_indices

                if (np.sum(left_indices) < self.min_samples_leaf or 
                    np.sum(right_indices) < self.min_samples_leaf):
                    continue

                y_left = y[left_indices]
                y_right = y[right_indices]

                quality = self.split_metric.calculate_split_quality(y_left, y_right)

                if quality < best_quality:
                    best_quality = quality
                    best_feature = feature_index
                    best_threshold = threshold

        return best_feature, best_threshold

    def predict(self, X: np.ndarray) -> np.ndarray:
        """
        Предсказывает значения для входных данных.

        Args:
            X (np.ndarray): Матрица признаков.

        Returns:
            predictions (np.ndarray): Предсказанные значения.

        Raises:
            ValueError: Если модель не обучена.
        """
        if self.root is None:
            raise ValueError("Model is not fitted yet.")

        return np.array([self._predict_sample(sample, self.root) for sample in X])

    def _predict_sample(self, sample: np.ndarray, node: TreeNode) -> float:
        """
        Предсказывает значение для одного образца.

        Args:
            sample (np.ndarray): Вектор признаков образца.
            node (TreeNode): Текущий узел дерева.

        Returns:
            prediction (float): Предсказанное значение.
        """
        if node.is_leaf():
            return node.value

        if sample[node.feature_index] <= node.threshold:
            return self._predict_sample(sample, node.left)
        else:
            return self._predict_sample(sample, node.right)
