"""
Linear Regression module
"""

from .metrics.error import (
    Error,
    MSE,
    MAE,
)

from .models.linreg import (
    CustomLinReg,
    CustomAnalLinReg,
    CustomComputeLinReg,
)

__all__ = [
    'Error',
    'MSE',
    'MAE',
    'CustomLinReg',
    'CustomAnalLinReg',
    'CustomComputeLinReg',
]
