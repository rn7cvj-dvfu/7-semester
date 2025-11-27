import pandas as pd
import numpy as np
import plotly.express as px
import plotly.graph_objects as go

import kmeans as kms

CLUSTERS_COUNT = 3

df = pd.read_csv('./dataset.csv')
df_raw = df.copy()


temp_x = df[['BALANCE', 'PURCHASES']]



elbow_trainer: kms.ElbowTrainer = kms.ElbowTrainerImpl(
    X=temp_x.to_numpy(),
    ks=range(1, 11),
    km_stopper= kms.KMeansIterationStoperItersCount(10),
)

elbow_results = elbow_trainer.train()


fig = go.Figure()

fig.add_trace(
    go.Scatter(
        x=[res['k'] for res in elbow_results],
        y=[res['inertia'] for res in elbow_results],
        mode='lines+markers',
        name='Inertia vs K',
    )
)

fig.show()


# kmeans : kms.KMeans = kms.KMeansImpl(
#     X=temp_x.to_numpy(),
#     clusters_count=CLUSTERS_COUNT,
#     stopper= kms.KMeansIterationStoperItersCount(10),
# )

# kmeans.fit()

# clusters = [kmeans.predict(x) for x in temp_x.to_numpy()]

# # print(clusters)

# temp_x['СLUSTER'] = clusters

# fig = px.scatter(
#     temp_x,
#     x='BALANCE',
#     y='PURCHASES',
#     color='СLUSTER',
#     title='Распределение данных по BALANCE и PURCHASES',
#     labels={'BALANCE': 'Баланс', 'PURCHASES': 'Покупки'},
#     opacity=0.6
# )

# fig.show()