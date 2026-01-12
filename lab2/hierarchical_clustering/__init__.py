"""
Hierarchical Clustering module
"""

from .models.agglomerative import AgglomerativeClustering
from .models.divisive import DivisiveClustering
from .models.utils.dendrogram import DendrogramNode
from .models.utils.linkage import (
    LinkageMethod,
    SingleLinkage,
    CompleteLinkage,
    AverageLinkage,
    WardLinkage,
    get_linkage_method
)
from .models.utils.distance import (
    DistanceMetric,
    EuclideanDistance,
    ManhattanDistance,
    CosineDistance,
    compute_distance_matrix
)

__all__ = [
    'AgglomerativeClustering',
    'DivisiveClustering',
    'DendrogramNode',
    'LinkageMethod',
    'SingleLinkage',
    'CompleteLinkage',
    'AverageLinkage',
    'WardLinkage',
    'get_linkage_method',
    'DistanceMetric',
    'EuclideanDistance',
    'ManhattanDistance',
    'CosineDistance',
    'compute_distance_matrix',
]
